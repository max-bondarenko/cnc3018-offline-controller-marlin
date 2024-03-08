#pragma once

#include "constants.h"
#include "Screen.h"
#include "debug.h"

template <class Detector>
class DetectorScreen : public Screen {
public:
    DetectorScreen() : nextRefresh{1} {}

    void step() override {
        if (nextRefresh != 0 && millis() > nextRefresh) {
            nextRefresh = millis() + REFRESH_INTL;
            doDirty();
        }
    };

protected:
    uint32_t nextRefresh;

    void drawContents() override {
        constexpr int lh = 7;
        const int LEN = 21;
        char str[LEN];
        U8G2& _u8g2 = display->getU8G2();
        _u8g2.setFont(u8g2_font_7x13B_tr);
        int sx = 4;
        int sy = Display::STATUS_BAR_HEIGHT;
        if (Detector::getDetectResult() == 0) {
            uint8_t a = Detector::cAttempt;
            _u8g2.drawStr(sx, sy, "Searching");
            for (int i = 0; i <= a; ++i) {
                _u8g2.drawStr(sx + (9 + i) * 7, sy , ".");
            }
            snprintf(str, LEN, "%s on %ld", Detector::deviceName, Detector::serialBaud);
            _u8g2.drawStr(sx, sy + lh * 2, str);
        }
    }
};
