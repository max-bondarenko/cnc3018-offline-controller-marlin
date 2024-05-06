#include "constants.h"
#include "GCodeDevice.h"
#include "WString.h"
#include "math.h"

bool GCodeDevice::scheduleCommand(const char* cmd, size_t len) {
    if (lastStatus >= DeviceStatus::ALARM)
        return false;
    if (len == 0)
        len = strlen(cmd);
    if (len == 0)
        return false;
    if (curUnsentCmdLen != 0)
        return false;
    DEV_LOGF("< '%s'\n", cmd);
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
    DEV_LOGF("<p '%s'\n", cmd);
    memcpy(curUnsentPriorityCmd, cmd, len);
    curUnsentPriorityCmdLen = len;
    return true;
}

void GCodeDevice::step() {
    readLockedStatus();
    if (lastStatus >= DeviceStatus::ALARM) {
        cleanupQueue();
    } else if ((!xoffEnabled || !xoff)
               && lastStatus != DeviceStatus::LOCKED) {
        trySendCommand();
    }
    // how locked and disconnected should work together ??
    receiveResponses();
    checkTimeout();
    if (wantUpdate) {
        wantUpdate = false;
        notify_observers(DeviceRefreshEvent{});
    }
}

void GCodeDevice::begin(SetupFN* const onBegin) {
    if (onBegin != nullptr) {
        (*onBegin)(serialCNC);
    }
    // in case onBegin did not consume all data in stream
    while (serialCNC.available() > 0) {
        serialCNC.read();
    }
    readLockedStatus();
}

void GCodeDevice::readLockedStatus() {
    if (serialCNC.isLocked(true))
        lastStatus = DeviceStatus::LOCKED;
}

void GCodeDevice::cleanupQueue() {
    curUnsentCmdLen = 0;
    curUnsentPriorityCmdLen = 0;
}

void GCodeDevice::checkTimeout() {
    if (!isRxTimeoutEnabled())
        return;
    if (lastStatus == DeviceStatus::LOCKED)
        extendRxTimeout();
    if (millis() > serialRxTimeout) {
        lastStatus = DeviceStatus::DISCONNECTED;
        cleanupQueue();
    }
}

void GCodeDevice::extendRxTimeout() {
    if (isRxTimeoutEnabled())
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
    while (serialCNC.available()) {
        char ch = (char) serialCNC.read();
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
                if (responseLen < MAX_LINE_LEN)
                    responseBuffer[responseLen++] = ch;
        }
        if (ch == '\n') {
            responseBuffer[responseLen] = 0;
            IO_LOGLN(responseBuffer);
            tryParseResponse(responseBuffer, responseLen);
            responseLen = 0;
            extendRxTimeout();
        }
    }
}
