#include "MarlinDRO.h"
#include "gcode/gcode.h"
#include "DRO.h"
#include <math.h>

#include "../assets/arrows_lr.XBM"
#include "../assets/arrows_ud.XBM"
#include "../assets/arrows_zud.XBM"

#include "../assets/dist.XBM"
#include "../assets/feed.XBM"
#include "../assets/spindle.XBM"


void MarlinDRO::begin() {
    DRO::begin();
    menuItems.push_back(MenuItem::simpleItem(2, "Home", [this](MenuItem&) {
        dev.scheduleCommand(G28_START_HOMING, 3);
    }));
    menuItems.push_back(MenuItem::simpleItem(3, "Set XY to 0", [this](MenuItem&) {
        dev.scheduleCommand("G92 X0Y0", 8);
    }));
    menuItems.push_back(MenuItem::simpleItem(4, "Set Z to 0", [this](MenuItem&) {
        dev.scheduleCommand("G92 Z0", 6);
    }));
    menuItems.push_back(MenuItem::simpleItem(5, "to Relative", [this](MenuItem& m) {
        dev.scheduleCommand(dev.isRelative() ? G90_SET_ABS_COORDINATES : G91_SET_RELATIVE_COORDINATES, 3);
        dev.toggleRelative();
        m.text = dev.isRelative() ? "to Abs" : "to Relative";
    }));
    menuItems.push_back(MenuItem::simpleItem(6, "Goto XY=0", [this](MenuItem&) {
        dev.scheduleCommand("G0 X0Y0", 7);
    }));
    menuItems.push_back(MenuItem::simpleItem(7, "STOP", [this](MenuItem&) {
        dev.scheduleCommand(M0_STOP_UNCONDITIONAL, 3);
    }));
    menuItems.push_back(MenuItem::simpleItem(8, "CONTINUE", [this](MenuItem&) {
        dev.scheduleCommand(M108_CONTINUE, 4);
    }));
}

void MarlinDRO::drawContents() {
    const uint8_t LEN = 20;
    char str[LEN];
    U8G2& u8g2 = display->getU8G2();
    u8g2.setFont(u8g2_font_7x13B_tr);
    const uint8_t lineWidth = u8g2.getMaxCharWidth();
    const uint8_t lineHeight = u8g2.getMaxCharHeight();
    constexpr uint8_t startLeftPanel = 0;
    constexpr uint8_t axisPadding = 2;
    constexpr uint8_t startRightPanel = 72;
    constexpr uint8_t ICON_TOP_PADDING = 4;

    uint8_t sx = startLeftPanel,
            sx_start_right = startRightPanel;
    uint8_t sy = Display::STATUS_BAR_HEIGHT; // bar Height 0 - 15.

    if (dev.canJog() && !job.isRunning()) {
        /// =============== draw control bar =============
        u8g2.setDrawColor(1);
        constexpr uint8_t endOfLeft = 43 + 15;
        uint8_t boxY = sy + 3;
        switch (cMode) {
            case Mode::AXES : {
                u8g2.drawFrame(sx, boxY, 73, lineHeight * 3 + 4);
                drawAxisIcons(endOfLeft + 4, boxY + 2, lineHeight);
                break;
            }
            case Mode::SPINDLE: {
                u8g2.drawFrame(sx_start_right, boxY, 54, lineHeight * 3 + 4);
                drawAxisIcons(sx_start_right + 43, boxY + 2, lineHeight);
                break;
            }
            case Mode::TEMP : {
                u8g2.drawFrame(sx, boxY, 125, lineHeight * 3 + 4);
                drawAxisIcons(endOfLeft + 17, boxY + 2, lineHeight);
            }
            default :
                break;
        }
    }
    sy += 7;
    switch (cMode) {
        case Mode::TEMP : {
            char buffer[13];
            buffer[1] = 0;
            buffer[0] = PRINTER[1] /*B*/;
            u8g2.drawStr(axisPadding, sy, buffer);
            buffer[0] = PRINTER[0] /*T*/;
            /// === draw Extrude & temps panel ==
            /// BED
            u8g2.drawStr(axisPadding, sy + lineHeight, buffer);
            snprintf(buffer, 12, "%5.1f/%3d", dev.getBedTemp(), expectedBedTemp);
            u8g2.drawStr(lineWidth * 2 - 3, sy, buffer);
            drawPower(sy + lineHeight - 2, lineHeight, dev.getBedPower());
            /// Extruder Temp
            snprintf(buffer, 12, "%5.1f/%3d", dev.getTemp(), expectedTemp);
            u8g2.drawStr(lineWidth * 2 - 3, sy + lineHeight, buffer);
            drawPower(sy + lineHeight * 2 - 2, lineHeight, dev.getHotendPower());
            /// == extruder Axis
            drawAxis(AXIS[3], dev.getE(), axisPadding, sy + lineHeight * 2);
            sx_start_right += 11; //padding for spindle/feed values
        }
            break;
        default:
            drawAxis(AXIS[0], dev.getX(), axisPadding, sy);
            drawAxis(AXIS[1], dev.getY(), axisPadding, sy + lineHeight);
            drawAxis(AXIS[2], dev.getZ(), axisPadding, sy + lineHeight * 2);
    }
    ///  draw right panel ====================
    sx_start_right += 3;
    u8g2.drawXBM(sx_start_right, sy + ICON_TOP_PADDING, dist_width, dist_height, (uint8_t*) dist_bits);
    u8g2.drawXBM(sx_start_right, sy + lineHeight + ICON_TOP_PADDING, feed_width, feed_height, (uint8_t*) feed_bits);
    if (cMode != Mode::TEMP) {
        u8g2.drawXBM(sx_start_right, sy + lineHeight * 2, spindle_width, spindle_height, (uint8_t*) spindle_bits);
    }

    /// distance  ======
    sx_start_right += 10;
    const float& jd = JOG_DISTS[cDist];
    snprintf(str, LEN, "%.*f", jd < 1.0 ? 1 : 0, jd);
    u8g2.drawStr(sx_start_right, sy, str);
    /// Feed ======
    snprintf(str, LEN, "%d", JOG_FEEDS[cFeed]);
    u8g2.drawStr(sx_start_right, sy + lineHeight, str);
    if (cMode != Mode::TEMP) {
        /// spindle ======
        snprintf(str, LEN, "%ld", dev.getSpindleVal());
        u8g2.drawStr(sx_start_right, sy + lineHeight * 2, str);
    }
}

void MarlinDRO::drawAxisIcons(uint8_t sx, uint8_t sy, const uint8_t lineHeight) const {
    U8G2& u8g2 = display->getU8G2();
    constexpr uint8_t ICONS_WIDTH = 9;
    // arrow lr 4 pixel less than rest icons
    constexpr uint8_t ICON_TOP_PADDING = 4;
    u8g2.drawBox(sx, sy, ICONS_WIDTH, lineHeight * 3);
    // inverse mode
    u8g2.setBitmapMode(1);
    u8g2.setDrawColor(2);
    // left padding for icons
    sx += 1;
    u8g2.drawXBM(sx, sy + ICON_TOP_PADDING, arrows_lr_width, arrows_lr_height, (uint8_t*) arrows_lr_bits);
    u8g2.drawXBM(sx, sy + lineHeight, arrows_ud_width, arrows_ud_height, (uint8_t*) arrows_ud_bits);
    u8g2.drawXBM(sx, sy + lineHeight * 2, arrows_zud_width, arrows_zud_height, (uint8_t*) arrows_zud_bits);
}

void MarlinDRO::onButton(int bt, Display::ButtonEvent evt) {
    if (!dev.canJog()) return;

    if (bt == Display::BT_CENTER) {
        if (evt == Evt::HOLD) {
            if (cMode != Mode::TEMP)
                cMode = Mode::TEMP;
            else
                cMode = Mode::AXES;
            buttonWasPressedWithShift = false;
        } else if (evt == Evt::UP) {
            if (buttonWasPressedWithShift) {
                auto _mode = static_cast<uint8_t>(cMode);
                _mode++;
                _mode = _mode % (static_cast<uint8_t>(Mode::N_VALS) - 1);
                cMode = static_cast<Mode>(_mode);
            }
            buttonWasPressedWithShift = false;
        } else if (evt == Evt::DOWN) {
            buttonWasPressedWithShift = true;
        }
        doDirty();
        return;
    }
// ===== not BT_CENTER
    switch (cMode) {
        case Mode::AXES :
            onButtonAxes(bt, evt);
            break;
        case Mode::SPINDLE:
            onButtonShift(bt, evt);
            dev.adjustSpindle(dev.getSpindleValues()->at(cSpindleVal));
            break;
        case Mode::TEMP:
            onButtonTemp(bt, evt);
        default:
            break;
    }
}

void MarlinDRO::onButtonTemp(uint8_t bt, Evt evt) {
    if (evt == Evt::DOWN || evt == Evt::HOLD) {
        uint8_t axis = 0xFF;
        float dist = JOG_DISTS[cDist];
        uint16_t feed = JOG_FEEDS[cFeed];
        switch (bt) {
            // === AXIS
            case Display::BT_ZUP:
                axis = 3; // E
                break;
            case Display::BT_ZDOWN:
                axis = 3; // E
                dist = -dist;
                break;
            default:; //noop
        }
        if (axis != 0xFF) {
            dev.jog(axis, dist, feed);
        }

        int temp = (uint8_t) round(dist);
        switch (bt) {
            // ======== temperature ========
            case Display::BT_UP:
                expectedTemp += temp;
                if (expectedTemp > MAX_TEMP) {
                    expectedTemp = MAX_TEMP;
                }
                break;
            case Display::BT_DOWN:
                expectedTemp -= temp;
                if (expectedTemp < 0) {
                    expectedTemp = 0;
                }
                break;
            case Display::BT_R:
                expectedBedTemp += temp;
                if (expectedBedTemp > MAX_TEMP) {
                    expectedBedTemp = MAX_TEMP;
                }
                break;
            case Display::BT_L:
                expectedBedTemp -= temp;
                if (expectedBedTemp < 0) {
                    expectedBedTemp = 0;
                }
                break;
            default:;
        }
        dev.tempChange(expectedTemp);
        doDirty();
    }
}


void MarlinDRO::drawPower(uint16_t sy, uint8_t lineHeight, uint16_t val) {
    U8G2& u8g2 = display->getU8G2();
    val = val >= 127 ? lineHeight - 2 :
          val >= 64 ? lineHeight / 2 :
          val > 0 ? 2 : 0;
    int8_t wide = val > 6 ? 3 :
                  val > 4 ? 2 : 1;
    int16_t lx = 70 + 3;
    // (2)( lx - wide,sy - val)   --  (lx + wide,sy - val)  (0)
    //                 \                      /
    //                       \            /
    //                        (lx, sy )  (1)
    u8g2.drawTriangle(lx + wide, sy - val, lx, sy, lx - wide, sy - val);
}