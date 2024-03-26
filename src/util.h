//
// Created by lima on 12/13/23.
//

#ifndef CNC_3018_UTIL_H
#define CNC_3018_UTIL_H

#include <Arduino.h>

inline bool startsWith(const char* str, const char* pre) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

constexpr int STRTOLL_BASE = 10;

size_t snprintfloat(char* destination, size_t sz, int32_t f, uint8_t fractures, int8_t width = 3);

size_t parseFloatInMils(char* strValue);

#endif //CNC_3018_UTIL_H
