#include "constants.h"
#include "features.h"
#include "GCodeDevice.h"
#include "WString.h"

void GCodeDevice::scheduleCommand(const char* cmd, size_t len) {
    if (len == 0)
        len = strlen(cmd);
    if (len == 0)
        return;
    buffer.push(Command(len, cmd));
}

void GCodeDevice::schedulePriorityCommand(const char* cmd, size_t len) {
    if (len == 0)
        len = strlen(cmd);
    if (len == 0)
        return;
    if (priorityCmd.len != 0)
        return;
    memcpy(priorityCmd.str, cmd, len);
    priorityCmd.len = len;
}

bool GCodeDevice::canSchedule() const {
    return !buffer.full<Command>();
}

bool GCodeDevice::bufferEmpty()  const {
    return buffer.empty();
}

void GCodeDevice::trySendCommand() {
    if (priorityCmd.len > 0 && serialCNC.availableForWrite()) {
        LOGF("> pr:[%s]\n", priorityCmd.str);
        serialCNC.write((const uint8_t*) (priorityCmd.str), priorityCmd.len);
        serialCNC.write('\n');
        priorityCmd.len = 0;
    } else if (!buffer.empty()) {
        for (auto i = buffer.readEnd<Command>(), end = buffer.end<Command>();
             ack < JOB_BUFFER_SIZE && i != end && serialCNC.availableForWrite();
             ++i) {
            const Command& cmd = *i;
            DEV_LOGF(">>[%s]\n", cmd.str);
            serialCNC.write(cmd.str, cmd.len);
            serialCNC.write('\n');
            --buffer;
            ack++;
        }
        lastResponse = nullptr;
    }
}

void GCodeDevice::step() {
    readLockedStatus();
    if (lastStatus >= DeviceStatus::ALARM) {
        cleanupQueue();
    } else if ((!xoffEnabled || !xoff)
               && lastStatus < DeviceStatus::WAIT) {
        trySendCommand();
    }
    // how locked and disconnected should work together ??
    receiveResponses();
    checkTimeout();
    if (wantUpdate) {
        wantUpdate = false;
        notify_observers(DeviceEvent::REFRESH);
    }
}

void GCodeDevice::begin() {
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
    priorityCmd.len = 0;
    buffer.clear();
    ack = 0;
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
            USB_TO_PC(responseBuffer);
            tryParseResponse(responseBuffer, responseLen);
            responseLen = 0;
            extendRxTimeout();
        }
    }
}
