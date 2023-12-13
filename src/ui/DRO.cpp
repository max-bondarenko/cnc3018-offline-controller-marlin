#include "DRO.h"
#include "devices/GCodeDevice.h"

constexpr int DRO::JOG_FEEDS[];
constexpr float DRO::JOG_DISTS[];
constexpr int DRO::SPINDLE_VALS[];
const char AXIS[] = {'X', 'Y', 'Z'};
const char AXIS_WCS[] = {'x', 'y', 'z'};
#include "../assets/arrows_lr.XBM"
#include "../assets/arrows_ud.XBM"
#include "../assets/arrows_zud.XBM"

#include "../assets/dist.XBM"
#include "../assets/feed.XBM"
#include "../assets/spindle.XBM"
#include "Screen.h"

void DRO::drawContents() {
    const int LEN = 20;
    char str[LEN];
    // need to know type
    // Grbl CNC has axis offsets

    U8G2 &u8g2 = Display::u8g2;

    //u8g2.setFont(u8g2_font_nokiafc22_tr);
    //u8g2.drawGlyph(64, 7, cMode==Mode::AXES ? 'M' : 'S');

    int sx = 2;
    int sy = Display::STATUS_BAR_HEIGHT + 3, sx2 = 72;
    constexpr int lh = 11;

    u8g2.setFont(u8g2_font_nokiafc22_tr);
//    const String &st = dev->getLastResponse(); todo ??
//    if (st) {
//        u8g2.drawStr(sx, 7, st.c_str());
//    }

    u8g2.setFont(u8g2_font_7x13B_tr);
    //u8g2.setFont(u8g2_font_freedoomr10_tu );

    //snprintf(str, 100, "u:%c bt:%d", digitalRead(PIN_DET)==0 ? 'n' : 'y',  buttStates);
    //u8g2.drawStr(sx, 7, str);
    //u8g2.drawGlyph(115, 0, !dev.isConnected() ? '-' : dev.isInPanic() ? '!' : '+' );


    if (dev.canJog()) {
        u8g2.setDrawColor(1);
        if (cMode == Mode::AXES) {
            u8g2.drawFrame(sx - 2, sy - 3, 70, lh * 3 + 6);
        } else {
            int t = 43;
            u8g2.drawFrame(sx2 - 2, sy - 3, 54, lh * 3 + 6);
            u8g2.drawBox(sx2 + t, sy - 2, 9, lh * 3 + 4);
            u8g2.setBitmapMode(1);
            u8g2.setDrawColor(2);
            t += 1;
            u8g2.drawXBM(sx2 + t, sy, arrows_zud_width, arrows_zud_height, (uint8_t *) arrows_zud_bits);
            u8g2.drawXBM(sx2 + t, sy + lh + 1, arrows_ud_width, arrows_ud_height, (uint8_t *) arrows_ud_bits);
            u8g2.drawXBM(sx2 + t, sy + lh * 2 + 3, arrows_lr_width, arrows_lr_height, (uint8_t *) arrows_lr_bits);
        }
    }

    sx += 5;
    drawAxisCoords(sx,sy);
    sx2 += 3;
    u8g2.drawXBM(sx2 + 1, sy, spindle_width, spindle_height, (uint8_t *) spindle_bits);
    u8g2.drawXBM(sx2, sy + lh + 3, feed_width, feed_height, (uint8_t *) feed_bits);
    u8g2.drawXBM(sx2, sy + lh * 2 + 3, dist_width, dist_height, (uint8_t *) dist_bits);
    sx2 += 10;
    snprintf(str, LEN, "%ld", dev.getSpindleVal());
    u8g2.drawStr(sx2, sy, str);
    snprintf(str, LEN, "%d", JOG_FEEDS[cFeed]);
    u8g2.drawStr(sx2, sy + lh, str);

    const float &jd = JOG_DISTS[cDist];
    if (jd < 1)
        snprintf(str, LEN, "0.%01u", unsigned(jd * 10));
    else
        snprintf(str, LEN, "%d", (int) jd);
    u8g2.drawStr(sx2, sy + lh * 2, str);

};

void DRO::onButton(int bt, Display::ButtonEvent evt) {

    LOGF("GrblDRO::onButton(%d,%d)\n", bt, (int)evt);

    if( !dev.canJog() ) return;

    if (bt == Display::BT_CENTER) {
        switch(evt) {
            case Evt::DOWN: {
                cMode = cMode==Mode::AXES ? Mode::SPINDLE : Mode::AXES;
                buttonWasPressedWithShift = false;
            }
                break;
            case Evt::UP: {
                if(buttonWasPressedWithShift) { cMode = cMode==Mode::AXES ? Mode::SPINDLE : Mode::AXES; }
            }
                break;
            default: break;
        }
        setDirty();
        return;
    }

    if(evt==Evt::DOWN) buttonWasPressedWithShift = true;

    if(cMode == Mode::AXES) {
        onButtonAxes(bt, evt);
    } else {
        onButtonShift(bt, evt);
    }

}

void DRO::onButtonAxes(int bt, Evt evt) {
    if (evt == Evt::DOWN || evt == Evt::HOLD) {
        int axis = -1;
        float d = JOG_DISTS[cDist];
        int f = JOG_FEEDS[cFeed];
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
            setDirty();
        }
    }
}

void DRO::onButtonShift(int bt, Evt evt) {
    if (!(evt == Evt::DOWN || evt == Evt::HOLD))
        return;

    switch (bt) {
        case Display::BT_R:
            if (evt == Evt::HOLD) cDist = N_JOG_DISTS - 1;
            else if (cDist < N_JOG_DISTS - 1) cDist++;
            break;
        case Display::BT_L:
            if (evt == Evt::HOLD) cDist = 0;
            else if (cDist > 0) cDist--;
            break;
        case Display::BT_ZDOWN:
        case Display::BT_ZUP: {
            if (bt == Display::BT_ZUP) {
                if (evt == Evt::HOLD)
                    cSpindleVal = N_SPINDLE_VALS - 1;
                else if (cSpindleVal < N_SPINDLE_VALS - 1)
                    cSpindleVal++;
            }
            if (bt == Display::BT_ZDOWN) {
                if (evt == Evt::HOLD) cSpindleVal = 0;
                else if (cSpindleVal > 0) cSpindleVal--;
            }
            int v = SPINDLE_VALS[cSpindleVal];
            if (v != 0) {
                char t[15];
                snprintf(t, 15, "M3 S%d", v);
                dev.scheduleCommand(t);
            } else {
                dev.scheduleCommand("M5");
            }
            break;
        }
        case Display::BT_DOWN:
            if (evt == Evt::HOLD)
                cFeed = 0;
            else if (cFeed > 0)
                cFeed--;
            break;
        case Display::BT_UP: //dev->schedulePriorityCommand("?");  break;
            if (evt == Evt::HOLD)
                cFeed = N_JOG_FEEDS - 1;
            else if (cFeed < N_JOG_FEEDS - 1)
                cFeed++;
            break;
        default:
            ; // skip

    }
    setDirty();

}