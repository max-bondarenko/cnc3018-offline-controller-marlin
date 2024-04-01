#include "SD.h"
#include "constants.h"
#include "util.h"
#include "GCodeDevice.h"
#include "WString.h"

bool GCodeDevice::scheduleCommand(const char* cmd, size_t len) {
    if (lastStatus >= DeviceStatus::ALARM)
        return false;
    if (len == 0)
        len = strlen(cmd);
    if (len == 0)
        return false;
    if (curUnsentCmdLen != 0)
        return false;
    LOGF("< '%s' \n", cmd);
    memcpy(curUnsentCmd, cmd, len);
    curUnsentCmdLen = len;
    return true;
}

bool GCodeDevice::schedulePriorityCommand(const char* cmd, size_t len) {
    if (len == 0)
        len = strlen(cmd);
    if (len == 0)
        return false;
    if (curUnsentPriorityCmdLen != 0)
        return false;

    memcpy(curUnsentPriorityCmd, cmd, len);
    curUnsentPriorityCmdLen = len;
    return true;
}

void GCodeDevice::step() {
    checkTimeout();
    readLockedStatus();
    if (lastStatus == DeviceStatus::ALARM) {
        cleanupQueue();
    } else if (
            (!xoffEnabled || !xoff)
            && !txLocked) {
        // no check for queued commands.
        // do it inside trySend
        trySendCommand();
    }
    receiveResponses();
    notify_observers(DeviceStatusEvent{lastStatus, lastResponse});
}

void GCodeDevice::begin() {
    while (printerSerial->available() > 0) {
        printerSerial->read();
    }
    readLockedStatus();

    if (SD.begin(PIN_CD)) {
        char buff[SHORT_BUFFER_LEN];
        uint8_t position = 0;
        File file = SD.open(SPINDLE_PRESET_FILE);
        while (file.available() > 0) {
            int ch = file.read();
            buff[position] = ch;
            ++position;
            if ('\n' == ch || '\r' == ch || position > SHORT_BUFFER_LEN - 1)
                break;

        }
        buff[position] = 0;
        char* token = strtok(buff, ",");
        position = 0;
        while (token != nullptr && position < 10) {
            spindleValues->push_back((uint16_t) strtol(token, nullptr, STRTOLL_BASE));
            ++position;
            token = strtok(nullptr, ",");
        }
        file.close();
    }
}

void GCodeDevice::readLockedStatus() {
    txLocked = printerSerial->isLocked(true);
    if (txLocked)
        lastStatus = DeviceStatus::LOCKED;
}

void GCodeDevice::cleanupQueue() {
    curUnsentCmdLen = 0;
    curUnsentPriorityCmdLen = 0;
}

void GCodeDevice::checkTimeout() {
    if (!isRxTimeoutEnabled()) return;
    if (millis() > serialRxTimeout) {
        LOGLN("checkTimeout fired");
        lastStatus = DeviceStatus::DISCONNECTED;
        cleanupQueue();
        armRxTimeout();
    }
}

void GCodeDevice::armRxTimeout() {
    if (!canTimeout)
        return;
    LOGLN(isRxTimeoutEnabled() ? "resetRxTout enable" : "resetRxTout disable");
    serialRxTimeout = millis() + KEEPALIVE_INTERVAL;
}

void GCodeDevice::disarmRxTimeout() {
    if (!canTimeout)
        return;
    serialRxTimeout = 0;
}

bool GCodeDevice::isRxTimeoutEnabled() const {
    return canTimeout && serialRxTimeout != 0;
}


void GCodeDevice::receiveResponses() {
    constexpr size_t MAX_LINE = 200; // M115 is far longer than 100
    static char resp[MAX_LINE + 1];
    static size_t respLen;

    while (printerSerial->available()) {
        char ch = (char) printerSerial->read();
        switch (ch) {
            case '\n':
            case '\r':
                break;
            case XOFF:
                if (xoffEnabled) {
                    xoff = true;
                    break;
                }
            case XON:
                if (xoffEnabled) {
                    xoff = false;
                    break;
                }
            default:
                if (respLen < MAX_LINE) resp[respLen++] = ch;
        }
        if (ch == '\n') {
            resp[respLen] = 0;
            tryParseResponse(resp, respLen);
            respLen = 0;
        }
    }
}
