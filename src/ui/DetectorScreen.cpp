#include "DetectorScreen.h"

void DetectorScreen::step() {
    if (nextRefresh != 0 && millis() > nextRefresh) {
        nextRefresh = millis() + REFRESH_INTL;
        doDirty();
    }
}

void DetectorScreen::drawContents() {
    constexpr int lh = 7;
    const int LEN = 21;
    char str[LEN];
    U8G2& _u8g2 = Display::u8g2;
    _u8g2.setFont(u8g2_font_7x13B_tr);
    int sx = 4;
    int sy = Display::STATUS_BAR_HEIGHT;
    if (detector->cResult == 0) {
        uint8_t a = detector->cAttempt;
        _u8g2.drawStr(sx, sy, "Searching");
        for (int i = 0; i <= a; ++i) {
            _u8g2.drawStr(sx + (9 + i) * 7, sy, ".");
        }
        snprintf(str, LEN, "%s on %ld", detector->deviceName, detector->serialBaud);
        _u8g2.drawStr(sx, sy + lh * 2, str);
    }
}