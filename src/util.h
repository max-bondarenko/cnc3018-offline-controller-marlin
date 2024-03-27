#ifndef CNC_3018_UTIL_H
#define CNC_3018_UTIL_H

#include <Arduino.h>

inline bool startsWith(const char* str, const char* pre) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

constexpr int STRTOLL_BASE = 10;

#endif //CNC_3018_UTIL_H