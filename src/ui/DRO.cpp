#include <vector>
#include "DRO.h"
#include "devices/GCodeDevice.h"
#include "util.h"

#include "../assets/arrows_lr.XBM"
#include "../assets/arrows_ud.XBM"
#include "../assets/arrows_zud.XBM"

#include "../assets/dist.XBM"
#include "../assets/feed.XBM"
#include "../assets/spindle.XBM"
#include "Screen.h"

void DRO::step() {
    if (nextRefresh != 0 && millis() > nextRefresh) {
        nextRefresh = millis() + REFRESH_INTL + 9;
        dev.requestStatusUpdate();
    }
}

void DRO::begin() {
    menuItems.push_back(MenuItem::simpleItem(0, "Open", [this](MenuItem&) {
        if (job.isRunning()) return;
        display->setScreen(&fileChooser); // this will reset the card
    }));
    menuItems.push_back(MenuItem::simpleItem(1, "Pause job", [this](MenuItem& m) {
        if (job.isRunning()) {
            job.setPaused(true);
            m.text = "Resume job";
        } else {
            job.setPaused(false);
            m.text = "Pause job";
        }
        doDirty();
    }));
}

void DRO::drawContents() {
    U8G2& u8g2 = display->getU8G2();
    u8g2.setFont(u8g2_font_7x13B_tr);
    u8g2.setDrawColor(1);

    constexpr char LEN = 20;
    char str[LEN];
    int sx = 2;
    int sy = Display::STATUS_BAR_HEIGHT + 3, sx2 = 73;
    constexpr uint8_t lh = Display::LINE_HEIGHT;

    if (dev.canJog()) {
        /// =============== draw frame =============
        if (cMode == Mode::AXES) {
            u8g2.drawFrame(sx - 2, sy - 3, sx2, lh * 3 + 6);
        } else if (cMode == Mode::SPINDLE) {
            int t = 43;
            u8g2.drawFrame(sx2, sy - 3, 52, lh * 3 + 6);
            u8g2.drawBox(sx2 + t, sy - 2, 9, lh * 3 + 4);
            u8g2.setBitmapMode(1);
            u8g2.setDrawColor(2);
            t += 1;
            u8g2.drawXBM(sx2 + t, sy, arrows_zud_width, arrows_zud_height, (uint8_t*) arrows_zud_bits);
            u8g2.drawXBM(sx2 + t, sy + lh + 1, arrows_ud_width, arrows_ud_height, (uint8_t*) arrows_ud_bits);
            u8g2.drawXBM(sx2 + t, sy + lh * 2 + 3, arrows_lr_width, arrows_lr_height, (uint8_t*) arrows_lr_bits);
            u8g2.setDrawColor(1);
        }
    }

    sx += 1;
    drawAxisCoords(sx, sy, lh);
    sx2 += 3;
    u8g2.drawXBM(sx2 + 1, sy, spindle_width, spindle_height, (uint8_t*) spindle_bits);
    u8g2.drawXBM(sx2, sy + lh + 3, feed_width, feed_height, (uint8_t*) feed_bits);
    u8g2.drawXBM(sx2, sy + lh * 2 + 3, dist_width, dist_height, (uint8_t*) dist_bits);
    sx2 += 10;
    snprintf(str, LEN, "%ld", dev.getSpindleVal());
    u8g2.drawStr(sx2, sy, str);
    snprintf(str, LEN, "%u", dev.getConfig().feed.at(cFeed));
    u8g2.drawStr(sx2, sy + lh, str);
    float jd = dev.getConfig().dist.at(cDist);
    snprintf(str, LEN, "%.*f", jd < 1.0 ? 1 : 0, jd);
    u8g2.drawStr(sx2, sy + lh * 2, str);
}

void DRO::onButton(int bt, Display::ButtonEvent evt) {
    LOGF("onButton(%d,%d)\n", bt, (int) evt);
    if (!dev.canJog()) return;
    if (bt == Display::BT_CENTER) {
        switch (evt) {
            case Evt::DOWN:
                cMode = cMode == Mode::AXES ? Mode::SPINDLE : Mode::AXES;
                buttonWasPressedWithShift = false;
                break;
            case Evt::UP:
                if (buttonWasPressedWithShift) {
                    cMode = cMode == Mode::AXES ? Mode::SPINDLE : Mode::AXES;
                }
                break;
            default:
                break;
        }
        doDirty();
        return;
    }

    if (evt == Evt::DOWN)
        buttonWasPressedWithShift = true;

    if (cMode == Mode::AXES) {
        onButtonAxes(bt, evt);
    } else {
        onButtonShift(bt, evt);
    }
}

void DRO::onButtonAxes(int bt, Evt evt) {
    if (evt == Evt::DOWN || evt == Evt::HOLD) {
        int axis = -1;
        float d = dev.getConfig().dist.at(cDist);
        uint16_t f = dev.getConfig().feed.at(cFeed);
        switch (bt) {
            case Display::BT_L:
                axis = 0;
                d = -d;
                break;
            case Display::BT_R:
                axis = 0;
                break;
            case Display::BT_UP:
                axis = 1;
                break;
            case Display::BT_DOWN:
                axis = 1;
                d = -d;
                break;
            case Display::BT_ZUP:
                axis = 2;
                break;
            case Display::BT_ZDOWN:
                axis = 2;
                d = -d;
                break;
            default:
                break;
        }
        if (axis != -1) {
            dev.jog(axis, d, f);
            doDirty();
        }
    }
}

void DRO::onButtonShift(int bt, Evt evt) {
    if (!(evt == Evt::DOWN || evt == Evt::HOLD))
        return;

    size_t n_spindle_val = dev.getConfig().spindle.size() - 1;
    size_t n_dist_val = dev.getConfig().dist.size() - 1;
    size_t n_feed_val = dev.getConfig().feed.size() - 1;

    switch (bt) {
        case Display::BT_R:
            if (evt == Evt::HOLD) cDist = n_dist_val;
            else if (cDist < n_dist_val) cDist++;
            break;
        case Display::BT_L:
            if (evt == Evt::HOLD) cDist = 0;
            else if (cDist > 0) cDist--;
            break;
        case Display::BT_ZDOWN:
        case Display::BT_ZUP: {
            if (bt == Display::BT_ZUP) {
                if (evt == Evt::HOLD)
                    cSpindleVal = n_spindle_val;
                else if (cSpindleVal < n_spindle_val)
                    cSpindleVal++;
            }
            if (bt == Display::BT_ZDOWN) {
                if (evt == Evt::HOLD) cSpindleVal = 0;
                else if (cSpindleVal > 0) cSpindleVal--;
            }
            uint16_t speed = dev.getConfig().spindle.at(cSpindleVal);
            if (speed != 0) {
                char t[15];
                int l = snprintf(t, 15, "M3 S%d", speed);
                dev.scheduleCommand(t, l);
            } else {
                dev.scheduleCommand("M5", 2);
            }
            break;
        }
        case Display::BT_DOWN:
            if (evt == Evt::HOLD)
                cFeed = 0;
            else if (cFeed > 0)
                cFeed--;
            break;
        case Display::BT_UP:
            if (evt == Evt::HOLD)
                cFeed =  n_feed_val;
            else if (cFeed < n_feed_val)
                cFeed++;
            break;
        default:; // skip

    }
    doDirty();
}

void DRO::drawAxisCoords(int sx, int sy, uint8_t lineHeight) {
    drawAxis(AXIS[0], dev.getX(), sx, sy);
    drawAxis(AXIS[1], dev.getY(), sx, sy + lineHeight);
    drawAxis(AXIS[2], dev.getZ(), sx, sy + lineHeight * 2);
}

void DRO::drawAxis(char axis, float value, int x, int y) {
    static const int LEN = 13;
    static char buffer[LEN];
    snprintf(buffer, LEN, "%c% *.*f", axis, 5 + defaultAxisPrecision, defaultAxisPrecision, value);
    display->getU8G2().drawStr(x, y, buffer);
}
