#pragma once

#include "constants.h"
#include "GCodeDevice.h"
#include "MarlinDevice.h"
#include "GrblDevice.h"
#include "gcode/gcode.h"

#include "debug.h"

constexpr uint8_t N_ATTEMPTS = 3;
constexpr static uint32_t serialBauds[] = {57600, 115200, 9600, 250000};
constexpr static uint32_t N_SERIAL_BAUDS = sizeof(serialBauds) / sizeof(serialBauds[0]);

typedef std::function<void(const char* const)> DeviceCallback;

class DeviceDetector {
public:
    static constexpr size_t MAX_LINE = 200; // M115 is far longer than 100
private:
    WatchedSerial& printerSerial;
    DeviceCallback& callback;
    uint32_t nextProbeTime;
    uint8_t cSpeed;
    uint8_t cDev;
    char resp[MAX_LINE + 1];
    size_t respLen = 0;

    void sendNextProbe();

    void collectResponse();

public:
    DeviceDetector(WatchedSerial& _serial, DeviceCallback& _callback) :
        printerSerial{_serial},
        callback{_callback},
        nextProbeTime{0},
        cSpeed{0},
        cDev{0},
        serialBaud{serialBauds[cSpeed]},
        cAttempt{0},
        cResult{0},
        deviceName{DEVICE_NAMES[cDev]} {}

    void loop();

    uint32_t serialBaud;
    uint8_t cAttempt;
    uint8_t cResult;
    const char* deviceName;
};
