#include "devices/DeviceDetector.h"
#include "debug.h"

void DeviceDetector::loop() {
    if (int32_t(millis() - nextProbeTime) > 0) {
        sendNextProbe();
        nextProbeTime = millis() + PROBE_INTERVAL;
    } else {
        collectResponse();
    }
}

void DeviceDetector::sendNextProbe() {
    serialBaud = serialBauds[cSpeed];
    deviceName = DEVICE_NAMES[cDev];
    while (printerSerial.available()) {
        printerSerial.read();
    }
    printerSerial.end();
    printerSerial.begin(serialBaud);
    DETECTOR_LOGLN("Det >");
    switch (cDev) {
        case DeviceName::GRBL:
            GrblDevice::sendProbe(printerSerial);
            break;
        case DeviceName::MARLIN:
            MarlinDevice::sendProbe(printerSerial);
    }
    cAttempt++;
    if (cAttempt == N_ATTEMPTS) {
        cAttempt = 0;
        cDev++;
        if (cDev == DeviceName::N_DEVICES) {
            cDev = 0;
            // ring
            cSpeed++;
            cSpeed %= N_SERIAL_BAUDS;
        }
    }
}

void DeviceDetector::collectResponse() {
    while (printerSerial.available()) {
        char ch = (char) printerSerial.read();
        switch (ch) {
            case '\n':
            case '\r':
                ch = '\n';
                break;
            default:
                if (respLen < MAX_LINE)
                    resp[respLen++] = ch;
        }
        if (ch == '\n' && respLen > 1) {
            resp[respLen] = 0;
            DETECTOR_LOGF("> [%s]\n", resp);
            bool ret = false;
            if (cDev == DeviceName::GRBL) {
                ret = GrblDevice::checkProbeResponse(resp);
            } else if (cDev == DeviceName::MARLIN) {
                ret = MarlinDevice::checkProbeResponse(resp);
            }
            if (ret) {
                DETECTOR_LOGF("> Got ");
                DETECTOR_LOGLN(deviceName);
                callback(deviceName);
                return;
            }
            respLen = 0;
        }
    }
}
