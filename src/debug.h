#pragma once

#include <Arduino.h>

#ifdef LOG_DEBUG
    #define LOG_DETECTOR
    #define LOGF(...)   do{SerialUSB.printf(__VA_ARGS__);}while(0)
    #define LOGLN(...)  do{SerialUSB.println(__VA_ARGS__);}while(0)
#else
    #define LOGF(...)
    #define LOGLN(...)
#endif

#ifdef LOG_DETECTOR
    #define DETECTOR_LOGF(...)   do{SerialUSB.printf(__VA_ARGS__);}while(0)
    #define DETECTOR_LOGLN(...)  do{SerialUSB.println(__VA_ARGS__);}while(0)
#else
    #define DETECTOR_LOGF(...)
    #define DETECTOR_LOGLN(...)
#endif


//#define LOGF(...)
//#define LOGLN(...)
