#include <Arduino.h>
#include "Screen.h"
#include "Display.h"

constexpr uint8_t VISIBLE_MENUS = 5;

extern Job job;
extern GCodeDevice* dev;

uint16_t Display::buttStates;

Display::Display() : dirty{true}, selMenuItem{0}, menuShown{false} {}

void Display::setScreen(Screen* _screen) {
    if (screen != nullptr)
        screen->onHide();
    screen = _screen;
    screen->onShow();
    selMenuItem = 0;
    menuShown = false;
    doDirty();
}

void Display::begin() {
    doDirty();
}

void Display::step() {
    if (screen != nullptr)
        screen->step();
    if (dirty)
        draw();
}

void Display::ensureSelMenuVisible() {
    if (selMenuItem >= screen->firstDisplayedMenuItem + VISIBLE_MENUS)
        screen->firstDisplayedMenuItem = selMenuItem - VISIBLE_MENUS + 1;
    if (selMenuItem < screen->firstDisplayedMenuItem) {
        screen->firstDisplayedMenuItem = selMenuItem;
    }
}

void Display::processInput() {
    auto changed = buttStates ^ prevStates;
    if (screen == nullptr)
        return;
    ButtonEvent evt;
    for (int i = 0; i < N_BUTTONS; i++) {
        bool down = bitRead(buttStates, i);
        if (bitRead(changed, i)) {
            if (i == BT_STEP && down && screen->menuItems.size > 0) {
                menuShown = !menuShown;
                doDirty();
            } else {
                evt = down ? ButtonEvent::DOWN : ButtonEvent::UP;
                if (down)
                    menuShownWhenDown = menuShown;
                // don't propagate events to screen if the click was in the menu
                if (menuShown) {
                    processMenuButton(i, evt);
                } else if (!menuShownWhenDown) {
                    screen->onButton(i, evt);
                }
            }
            holdCounter[i] = 0;
        } else if (down) {
            holdCounter[i]++;
            if (holdCounter[i] >= HOLD_COUNT) {
                evt = ButtonEvent::HOLD;
                if (menuShown) {
                    processMenuButton(i, evt);
                } else {
                    screen->onButton(i, evt);
                }
                holdCounter[i] = 0;
            }
        }
    }
    prevStates = buttStates;
}

void Display::processMenuButton(uint8_t bt, ButtonEvent evt) {
    if (!(evt == ButtonEvent::DOWN || evt == ButtonEvent::HOLD)) return;
    size_t menuLen = screen->menuItems.size;
    if (menuLen != 0) {
        if (bt == BT_UP) {
            selMenuItem = selMenuItem > 0 ? selMenuItem - 1 : menuLen - 1;
            ensureSelMenuVisible();
        }
        if (bt == BT_DOWN) {
            selMenuItem = (selMenuItem + 1) % menuLen;
            ensureSelMenuVisible();
        }
        Screen::MenuItem& item = screen->menuItems[selMenuItem];
        if (bt == BT_CENTER) {
            item.cmd(item, 0);
            menuShown = false;
        }
        if (item.canChangeValue) {
            if (bt == BT_L) {
                item.cmd(item, -1);
            }
            if (bt == BT_R) {
                item.cmd(item, 1);
            }
        }
        doDirty();
    }
}

void Display::draw() {
    u8g2.clearBuffer();
    if (dev != nullptr)
        drawStatusBar();
    if (screen != nullptr) {
        screen->drawContents();
        if (menuShown)
            drawMenu();
    }
    u8g2.sendBuffer();
    dirty = false;
}

#include "../assets/locked.XBM"
#include "../assets/connected.XBM"

void Display::drawStatusBar() {
    constexpr int MAX_LEN = 22;
    constexpr int MAX_STATUS_LEN = 12;
    char str[MAX_LEN];
    GCodeDevice& _dev = *dev;

    u8g2.setFont(u8g2_font_nokiafc22_tr);
    u8g2.setDrawColor(1);
    // allow 2 lines in status bar
    constexpr int fH = 8; // 8x8
    int x = 2, y = -1;

    switch (_dev.lastStatus) {
        case DeviceStatus::NONE:
            break;
        case DeviceStatus::DISCONNECTED :
            u8g2.drawGlyph(x, y, 'X');
            break;
        case DeviceStatus::LOCKED:
            u8g2.drawXBM(x, 0, locked_width, locked_height, (const uint8_t*) locked_bits);
            break;
        default:
            u8g2.drawXBM(x, 0, connected_width, connected_height, (const uint8_t*) connected_bits);
    }
    memcpy(str, _dev.lastStatusStr, MAX_STATUS_LEN);
    str[MAX_STATUS_LEN] = 0;
    u8g2.drawStr(10, y, str);
    memcpy(str, _dev.lastResponse, MAX_LEN);
    str[MAX_LEN - 1] = 0;
    u8g2.drawStr(2, y + fH, str);
    //job status
    if (job.isValid()) {
        uint8_t p = job.getCompletion();
        snprintf(str, 9, "% 2d%%", p);
        if (job.isRunning())
            str[0] = 'R';
        else {
            if (job.isError())
                str[0] = 'E';
            else
                str[0] = 'P';
        }
        str[9] = 0;
        u8g2.drawStr(u8g2.getDisplayWidth() - 5 * fH + 1, y, str);
    }
}

void Display::drawMenu() {
    u8g2.setFont(u8g2_font_nokiafc22_tr);
    const size_t len = screen->menuItems.size;
    uint8_t onscreenLen = len - screen->firstDisplayedMenuItem;

    if (onscreenLen > VISIBLE_MENUS)
        onscreenLen = VISIBLE_MENUS;
    const int w = 80, x = 20, lh = 8, h = onscreenLen * lh;
    int y = 6;

    u8g2.setDrawColor(0);
    u8g2.drawBox(x, y, w, lh + h + 4);
    u8g2.setDrawColor(1);
    u8g2.drawFrame(x, y, w, lh + h + 4);

    char str[20];
    snprintf(str, 20, "Menu [%d/%d]", selMenuItem + 1, len);
    u8g2.drawStr(x + 2, y + 1, str);

    y = 16;

    for (size_t i = 0; i < onscreenLen; i++) {
        size_t idx = screen->firstDisplayedMenuItem + i;
        if (selMenuItem == idx) {
            u8g2.setDrawColor(1);
            u8g2.drawBox(x, y + i * lh, w, lh);
        }
        Screen::MenuItem& item = screen->menuItems[idx];
        u8g2.setDrawColor(2);
        u8g2.drawStr(x + 2, y + i * lh - 1, item.text);
    }

    u8g2.setDrawColor(0);
    int xx = x + w - 5;
    u8g2.drawBox(xx - 1, y, 5, h);
    u8g2.setDrawColor(1);
    u8g2.drawFrame(xx, y, 5, h);
    if (len > VISIBLE_MENUS) {
        const uint hh = (h - 2) * VISIBLE_MENUS / len;
        const uint sy = (h - hh) * screen->firstDisplayedMenuItem / (len - VISIBLE_MENUS);
        u8g2.drawBox(xx + 1, y + 1 + sy, 3, hh);
    }
}
