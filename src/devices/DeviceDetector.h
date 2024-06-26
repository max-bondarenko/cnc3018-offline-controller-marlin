#pragma once

#include "constants.h"
#include "GCodeDevice.h"
#include "MarlinDevice.h"
#include "GrblDevice.h"
#include "gcode/gcode.h"

#include "debug.h"

constexpr static uint8_t N_DEVICES = 2;
constexpr static uint8_t N_ATTEMPTS = 3;
constexpr static uint32_t serialBauds[] = {57600, 115200, 9600, 250000};
constexpr static uint32_t N_SERIAL_BAUDS = sizeof(serialBauds) / sizeof(serialBauds[0]);
const char* deviceNames[] = {"grbl", "marlin"};

template <class T, T& printerSerial, void (* createDevice)(int, T*)>
class DeviceDetector {
public:
    static void init() {
        nextProbeTime = 0;
        cResult = 0;
        cAttempt = 0;
        cSpeed = 0;
        cDev = 0;
        deviceName = deviceNames[cDev];
        serialBaud = serialBauds[cSpeed];
    }

    static void loop() {
        if (cResult != 0) return;

        if (int32_t(millis() - nextProbeTime) > 0) {
            sendNextProbe();
            nextProbeTime = millis() + PROBE_INTERVAL;
        } else {
            collectResponse();
        }
    }

    static int getDetectResult() { return cResult; }

    static uint32_t serialBaud;
    static const char* deviceName;
    static uint8_t cAttempt;
private:
    static uint8_t cSpeed;
    static uint8_t cDev;

    static int cResult;
    static uint32_t nextProbeTime;

    static void sendNextProbe() {
        serialBaud = serialBauds[cSpeed];
        deviceName = deviceNames[cDev];

        while (printerSerial.available()) {
            printerSerial.read();
        }
        printerSerial.end();
        printerSerial.begin(serialBaud);
        LOGLN("Send");
        switch (cDev) {
            case 0:
                GrblDevice::sendProbe(printerSerial);
                break;
            default :
                MarlinDevice::sendProbe(printerSerial);
        }
        cAttempt++;
        if (cAttempt == N_ATTEMPTS) {
            cAttempt = 0;
            cDev++;
            if (cDev == N_DEVICES) {
                cDev = 0;
                // ring
                cSpeed++;
                cSpeed %= N_SERIAL_BAUDS;
            }
        }
    }

    static void collectResponse() {
        constexpr size_t MAX_LINE = 200; // M115 is far longer than 100
        static char resp[MAX_LINE + 1];
        static size_t respLen;

        while (printerSerial.available()) {
            char ch = (char) printerSerial.read();
            switch (ch) {
                case '\n':
                case '\r':
                    break;
                default:
                    if (respLen < MAX_LINE)
                        resp[respLen++] = ch;
            }
            if (ch == '\n') {
                resp[respLen] = 0;
                LOGF("> [%s]\n", resp);
                String _resp(resp);
                bool ret = false;
                if (cDev == 0) {
                    ret = GrblDevice::checkProbeResponse(_resp);
                } else if (cDev == 1) {
                    ret = MarlinDevice::checkProbeResponse(_resp);
                }

                if (ret) {
                    createDevice(cDev, &printerSerial);
                    cResult = 1;
                    return;
                }
                respLen = 0;
            }
        }
    }

};

template <class T, T& printerSerial, void (* createDevice)(int, T*)>
uint32_t DeviceDetector<T, printerSerial, createDevice>::serialBaud;

template <class T, T& printerSerial, void (* createDevice)(int, T*)>
const char* DeviceDetector<T, printerSerial, createDevice>::deviceName;

template <class T, T& printerSerial, void (* createDevice)(int, T*)>
uint8_t DeviceDetector<T, printerSerial, createDevice>::cSpeed;

template <class T, T& printerSerial, void (* createDevice)(int, T*)>
uint8_t  DeviceDetector<T, printerSerial, createDevice>::cAttempt;

template <class T, T& printerSerial, void (* createDevice)(int, T*)>
int  DeviceDetector<T, printerSerial, createDevice>::cResult;

template <class T, T& printerSerial, void (* createDevice)(int, T*)>
uint8_t  DeviceDetector<T, printerSerial, createDevice>::cDev;

template <class T, T& printerSerial, void (* createDevice)(int, T*)>
uint32_t  DeviceDetector<T, printerSerial, createDevice>::nextProbeTime;
