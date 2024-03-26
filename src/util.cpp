#include "util.h"
#include <math.h>

static const int pows[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000};

static inline int pow10(uint8_t input) {
    return pows[input];
}

size_t snprintfloat(char* destination, size_t sz, int32_t f, uint8_t fractures, int8_t width) {
    constexpr uint8_t defaultFrac = 3;
    int removeFromLeft = defaultFrac - fractures;
    removeFromLeft = removeFromLeft < 0 ? 0 : removeFromLeft;
    width = abs(width);
    bool sign = f < 0;
    bool less = abs(f) < 1000;
    int32_t ff = abs(f / pow10(removeFromLeft));

    char buff[MAX(10, width + 1)];
    char* _f = buff;
    while (ff != 0 || fractures == 0) {
        if ((fractures--) == 0) {
            *(_f++) = '.';
            continue;
        }
        int a = ff % 10;
        ff /= 10;
        width--;
        *(_f++) = 0x30 + a;
    }
    if (less) {
        *(_f++) = '0';
        width--;
    }
    if (sign) {
        *(_f++) = '-';
        width--;
    }
    while (--width > 0) {
        *(_f++) = ' ';
    }
    char* d = destination;
    int len = 0;
    while (_f != buff && sz-- > 0) {
        *(d++) = *(--_f);
        len++;
    }
    *d = 0;
    return len;
}

size_t parseFloatInMils(char* strValue) {
    char* firstDig = strtok(strValue, ".");
    int32_t retVal = 0;
    if (firstDig != nullptr) {
        retVal = strtoll(firstDig, nullptr, STRTOLL_BASE);
        retVal *= 1000;
        firstDig = strtok(nullptr, ".");
        if (firstDig != nullptr) {
            int32_t number = strtoll(firstDig, nullptr, STRTOLL_BASE);
            uint16_t scale = strlen(firstDig);
            if (scale == 1)
                number *= 100;
            else if (scale == 2)
                number *= 10;
            if (retVal >= 0)
                retVal += number;
            else
                retVal -= number;
        }
    }
    return retVal;
}
