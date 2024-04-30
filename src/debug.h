#pragma once

#include <Arduino.h>

#ifdef LOG_DEBUG
    #define LOG_DETECTOR
    #define LOG_JOB
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

#ifdef LOG_JOB
    #define JOB_LOGF(...)   do{SerialUSB.printf(__VA_ARGS__);}while(0)
    #define JOB_LOGLN(...)  do{SerialUSB.println(__VA_ARGS__);}while(0)
#else
    #define JOB_LOGF(...)
    #define JOB_LOGLN(...)
#endif

#ifdef LOG_DEVICE
    #define DEV_LOGF(...)   do{SerialUSB.printf(__VA_ARGS__);}while(0)
    #define DEV_LOGLN(...)  do{SerialUSB.println(__VA_ARGS__);}while(0)
#else
    #define DEV_LOGF(...)
    #define DEV_LOGLN(...)
#endif

#ifdef LOG_IO
    #define USB_TO_SERIAL
    #define IO_LOGF(...)   do{SerialUSB.printf(__VA_ARGS__);}while(0)
    #define IO_LOGLN(...)  do{SerialUSB.println(__VA_ARGS__);}while(0)
#else
    #define IO_LOGF(...)
    #define IO_LOGLN(...)
#endif

#ifdef LOG_TIME
    #define TIME_LOGF(...)   do{SerialUSB.printf(__VA_ARGS__);}while(0)
#else
    #define LOG_TIME(...)
#endif

