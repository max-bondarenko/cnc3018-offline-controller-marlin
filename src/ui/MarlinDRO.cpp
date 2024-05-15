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

extern GCodeDevice* dev;

void MarlinDRO::begin() {
    menuItems.size = marlinDev.getCompatibilities().emergency_parser ? 2 + 8 : 2 + 6;
    menuItems.items = new MenuItem* [menuItems.size]; // not going to be deleted
    DRO::begin();
    uint8_t indx = 2U;
    menuItems.items[indx++] = MenuItem::simpleItem("to Relative", [this](MenuItem& m, int8_t) {
        marlinDev.schedulePriorityCommand(
            marlinDev.isRelative() ? G90_SET_ABS_COORDINATES : G91_SET_RELATIVE_COORDINATES, 3);
        marlinDev.toggleRelative();
        m.text = marlinDev.isRelative() ? "to Abs" : "to Relative";
    });
    if (marlinDev.getCompatibilities().emergency_parser) {
        menuItems.items[indx++] = MenuItem::simpleItem("1 min STOP", [](MenuItem&, int8_t) {
            dev->schedulePriorityCommand(M0_STOP_UNCONDITIONAL_FOR_60SEC, 7);
        });
        menuItems.items[indx++] = MenuItem::simpleItem("Continue", [this](MenuItem&, int8_t) {
            dev->schedulePriorityCommand(M108_CONTINUE, 4);
        });
    }
    // 14 char line is maximum for menu
    menuItems.items[indx++] = MenuItem::valueItem("Feed100<100%>", [](MenuItem& m, int8_t dir) {
        constexpr uint8_t LABEL_LEN = 14;
        static char buf[LABEL_LEN];
        if (dir == 0) {
            dev->feedrate = 100;
        } else if (dir > 0) {
            dev->feedrate = (dev->feedrate >= MAX_ADJUST_PERCENT - ADJUST_PERCENT_STEP) ?
                            MAX_ADJUST_PERCENT : dev->feedrate + ADJUST_PERCENT_STEP;
        } else {
            dev->feedrate = (dev->feedrate <= MIN_ADJUST_PERCENT + ADJUST_PERCENT_STEP) ?
                            MIN_ADJUST_PERCENT : dev->feedrate - ADJUST_PERCENT_STEP;
        }
        int l = snprintf(buf, 10, "%sS%d", M220_FEEDRATE_ADJUST, dev->feedrate);
        dev->schedulePriorityCommand(buf, 9);
        buf[l] = 0;
        l = snprintf(buf, LABEL_LEN, "Feed100<%d%%>", dev->feedrate);
        buf[l] = 0;
        m.text = buf;
    });
    menuItems.items[indx++] = MenuItem::valueItem("Flow100<100%>", [](MenuItem& m, int8_t dir) {
        constexpr uint8_t LABEL_LEN = 14;
        static char buf[LABEL_LEN];
        if (dir == 0) {
            dev->flowrate = 100;
        } else if (dir > 0) {
            dev->flowrate = (dev->flowrate >= MAX_ADJUST_PERCENT - ADJUST_PERCENT_STEP) ?
                            MAX_ADJUST_PERCENT : dev->flowrate + ADJUST_PERCENT_STEP;
        } else {
            dev->flowrate = (dev->flowrate <= MIN_ADJUST_PERCENT + ADJUST_PERCENT_STEP) ?
                            MIN_ADJUST_PERCENT : dev->flowrate - ADJUST_PERCENT_STEP;
        }
        int l = snprintf(buf, 10, "%sS%d", M221_FLOW_ADJUST, dev->flowrate);
        dev->schedulePriorityCommand(buf, 9);
        buf[l] = 0;
        l = snprintf(buf, LABEL_LEN, "Flow100<%d%%>", dev->flowrate);
        buf[l] = 0;
        m.text = buf;
    });
    menuItems.items[indx++] = MenuItem::simpleItem("Home", [](MenuItem&, int8_t) {
        dev->schedulePriorityCommand(G28_START_HOMING, 3);
    });
    menuItems.items[indx++] = MenuItem::simpleItem("Set XY to 0", [](MenuItem&, int8_t) {
        dev->schedulePriorityCommand("G92 X0Y0", 8);
    });
    menuItems.items[indx++] = MenuItem::simpleItem("Set Z to 0", [](MenuItem&, int8_t) {
        dev->schedulePriorityCommand("G92 Z0", 6);
    });
}

void MarlinDRO::drawContents() {
    const uint8_t LEN = 20;
    char str[LEN];
    Display::u8g2.setFont(u8g2_font_7x13B_tr);
    const uint8_t lineWidth = Display::u8g2.getMaxCharWidth();
    const uint8_t lineHeight = Display::u8g2.getMaxCharHeight();
    constexpr uint8_t startLeftPanel = 0;
    constexpr uint8_t axisPadding = 2;
    constexpr uint8_t startRightPanel = 72;
    constexpr uint8_t ICON_TOP_PADDING = 4;

    uint8_t sx = startLeftPanel,
        sx_start_right = startRightPanel;
    uint8_t sy = Display::STATUS_BAR_HEIGHT + 7; // bar Height 0 - 15.

    if (!job.isRunning()) {
        if (marlinDev.canJog()) {
            Display::u8g2.setDrawColor(1);
            constexpr uint8_t endOfLeft = 43 + 15;
            uint8_t boxY = sy - 4;
            /// =============== draw frame and icons =============
            switch (cMode) {
                case Mode::AXES : {
                    Display::u8g2.drawFrame(sx, boxY, 73, lineHeight * 3 + 4);
                    drawAxisIcons(endOfLeft + 4, boxY + 2, lineHeight);
                    break;
                }
                case Mode::SPINDLE: {
                    Display::u8g2.drawFrame(sx_start_right, boxY, 54, lineHeight * 3 + 4);
                    drawAxisIcons(sx_start_right + 43, boxY + 2, lineHeight);
                    break;
                }
                case Mode::TEMP : {
                    Display::u8g2.drawFrame(sx, boxY, 125, lineHeight * 3 + 4);
                    drawAxisIcons(endOfLeft + 17, boxY + 2, lineHeight);
                }
                default :
                    break;
            }
        }

        if (cMode == Mode::TEMP) {
            drawTemp(axisPadding, sy, lineHeight, lineWidth);
            drawPower(axisPadding + lineWidth * 10 + 4, sy + lineHeight - 2, lineHeight, marlinDev.getBedPower());
            drawPower(axisPadding + lineWidth * 10 + 4, sy + lineHeight * 2 - 2, lineHeight,
                      marlinDev.getHotendPower());
            sx_start_right += 11; //padding for spindle/feed values
        } else {
            drawAxis(AXIS[0], marlinDev.getX(), axisPadding, sy);
            drawAxis(AXIS[1], marlinDev.getY(), axisPadding, sy + lineHeight);
            drawAxis(AXIS[2], marlinDev.getZ(), axisPadding, sy + lineHeight * 2);
        }
        ///  draw right panel ====================
        sx_start_right += 3;
        Display::u8g2.drawXBM(sx_start_right, sy + ICON_TOP_PADDING, dist_width, dist_height, (uint8_t*) dist_bits);
        Display::u8g2.drawXBM(sx_start_right, sy + lineHeight + ICON_TOP_PADDING, feed_width, feed_height,
                              (uint8_t*) feed_bits);
        if (cMode != Mode::TEMP) {
            Display::u8g2.drawXBM(sx_start_right, sy + lineHeight * 2, spindle_width, spindle_height,
                                  (uint8_t*) spindle_bits);
        }
        /// distance  ======
        sx_start_right += 10;
        const float& jd = marlinDev.getConfig().dist.at(cDist);
        snprintf(str, LEN, "%.*f", jd < 1.0 ? 1 : 0, jd);
        Display::u8g2.drawStr(sx_start_right, sy, str);
        /// Feed ======
        snprintf(str, LEN, "%d", marlinDev.getConfig().feed.at(cFeed));
        Display::u8g2.drawStr(sx_start_right, sy + lineHeight, str);
        if (cMode != Mode::TEMP) {
            /// spindle ======
            snprintf(str, LEN, "%ld", marlinDev.getSpindleVal());
            Display::u8g2.drawStr(sx_start_right, sy + lineHeight * 2, str);
        }
    }

    if (job.isRunning()) {
        drawAxis(AXIS[0], marlinDev.getX(), axisPadding, sy);
        drawAxis(AXIS[1], marlinDev.getY(), axisPadding, sy + lineHeight);
        drawAxis(AXIS[2], marlinDev.getZ(), axisPadding, sy + lineHeight * 2);
        sx = sx_start_right - 13;
        drawTemp(sx, sy, lineHeight, lineWidth);
        drawPower(sx + lineWidth * 10 + 4, sy + lineHeight - 2, lineHeight, marlinDev.getBedPower());
        drawPower(sx + lineWidth * 10 + 4, sy + lineHeight * 2 - 2, lineHeight, marlinDev.getHotendPower());
    }
}

void MarlinDRO::drawAxisIcons(uint8_t sx, uint8_t sy, const uint8_t lineHeight) const {
    constexpr uint8_t ICONS_WIDTH = 9;
    // arrow lr 4 pixel less than rest icons
    constexpr uint8_t ICON_TOP_PADDING = 4;
    Display::u8g2.drawBox(sx, sy, ICONS_WIDTH, lineHeight * 3);
    // inverse mode
    Display::u8g2.setBitmapMode(1);
    Display::u8g2.setDrawColor(2);
    // left padding for icons
    sx += 1;
    Display::u8g2.drawXBM(sx, sy + ICON_TOP_PADDING, arrows_lr_width, arrows_lr_height, (uint8_t*) arrows_lr_bits);
    Display::u8g2.drawXBM(sx, sy + lineHeight, arrows_ud_width, arrows_ud_height, (uint8_t*) arrows_ud_bits);
    Display::u8g2.drawXBM(sx, sy + lineHeight * 2, arrows_zud_width, arrows_zud_height, (uint8_t*) arrows_zud_bits);
}

void MarlinDRO::onButton(int bt, Display::ButtonEvent evt) {
    if (!marlinDev.canJog()) return;

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
            // huck. marlin dont return status
            marlinDev.adjustSpindle(marlinDev.getConfig().spindle.at(cSpindleVal));
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
        float dist = marlinDev.getConfig().dist.at(cDist);
        uint16_t feed = marlinDev.getConfig().feed.at(cFeed);
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
        if (axis != 0xFF && marlinDev.isExtrusionEnabled()) {
            marlinDev.jog(axis, dist, feed);
        }
        int temp = (uint8_t) round(dist);
        auto expectedTempPrev = expectedTemp;
        auto expectedBedPrev = expectedBedTemp;
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
        if (expectedTemp ^ expectedTempPrev)
            marlinDev.tempChange(expectedTemp);
        if (expectedBedTemp ^ expectedBedPrev)
            marlinDev.bedTempChange(expectedBedTemp);
        doDirty();
    }
}


void MarlinDRO::drawPower(uint16_t sx, uint16_t sy, uint8_t lineHeight, uint16_t val) const {
    val = val >= 127 ? lineHeight - 2 :
          val >= 64 ? lineHeight - 4 :
          val > 0 ? 2 : 0;
    int8_t wide = val > 6 ? 3U : 2U;
    int16_t lx = sx + 4;
    // (2)(lx - wide,sy - val) -- (lx,sy - val)(0)
    //                 \              |
    //                   \            |
    //                     \          |
    //                       \        |
    //                         \      |
    //                           \    |
    //                             \  |
    //                        (lx,sy)(1)
    Display::u8g2.drawTriangle(lx, sy - val, lx, sy, lx - wide, sy - val);
}

void MarlinDRO::drawTemp(uint16_t sx, uint16_t sy, uint8_t lineHeight, uint8_t lineWidth) const {
    char buffer[13];
    buffer[1] = 0;
    buffer[0] = PRINTER[1] /*B*/;
    Display::u8g2.drawStr(sx, sy, buffer);
    buffer[0] = PRINTER[0] /*T*/;
    /// === draw Extrude & temps panel ==
    /// BED
    Display::u8g2.drawStr(sx, sy + lineHeight, buffer);
    snprintf(buffer, 6, "%5.1f", marlinDev.getBedTemp());
    Display::u8g2.drawStr(sx + lineWidth * 2 - 5, sy, buffer);
    // narrow '/'
    Display::u8g2.drawLine(sx + lineWidth * 7 - 1, sy + lineHeight - 3, sx + lineWidth * 7 + 1, sy);
    snprintf(buffer, 4, "%3lu", marlinDev.getBedRequestedTemp());
    Display::u8g2.drawStr(sx + lineWidth * 7 + 2, sy, buffer);
    /// Extruder Temp
    snprintf(buffer, 6, "%5.1f", marlinDev.getTemp());
    Display::u8g2.drawStr(sx + lineWidth * 2 - 5, sy + lineHeight, buffer);
    // narrow '/'
    Display::u8g2.drawLine(sx + lineWidth * 7 - 1, sy + lineHeight * 2 - 3, sx + lineWidth * 7 + 1, sy + lineHeight);
    snprintf(buffer, 4, "%3lu", marlinDev.getHotendRequestedTemp());
    Display::u8g2.drawStr(sx + lineWidth * 7 + 2, sy + lineHeight, buffer);
    /// == extruder Axis ================= v-big E-v/v-- small E ---v
    drawAxis(marlinDev.isExtrusionEnabled() ? AXIS[3] :
             (AXIS[3] + 0x20), marlinDev.getE(), sx, sy + lineHeight * 2);
}