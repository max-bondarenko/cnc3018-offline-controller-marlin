#include "SD.h"
#include "constants.h"
#include "util.h"
#include "GCodeDevice.h"
#include "ini.h"
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
    LOGF("< '%s'\n", cmd);
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
    LOGF("<p '%s'\n", cmd);
    memcpy(curUnsentPriorityCmd, cmd, len);
    curUnsentPriorityCmdLen = len;
    return true;
}

void GCodeDevice::step() {
    readLockedStatus();
    if (lastStatus == DeviceStatus::ALARM) {
        cleanupQueue();
    } else if ((!xoffEnabled || !xoff)
               && lastStatus != DeviceStatus::LOCKED) {
        trySendCommand();
    }
    // how locked and disconnected should work together ??
    receiveResponses();
    checkTimeout();
    notify_observers(DeviceStatusEvent{lastStatus, lastStatusStr, lastResponse});
}

auto spindleSetF = [](const char* token, GCodeDevice::Config& conf) {
    auto val = (uint16_t) strtoul(token, nullptr, STRTOLL_BASE);
    conf.spindle.push_back(val);
};
auto feedSetF = [](const char* token, GCodeDevice::Config& conf) {
    auto val = (uint16_t) strtoul(token, nullptr, STRTOLL_BASE);

    conf.feed.push_back(val);
};
auto distanceSetF = [](const char* token, GCodeDevice::Config& conf) {
    auto val = (float) atof(token);
    conf.dist.push_back(abs(val));
};

int GCodeDevice::configHandler(void* store, const char* section, const char* name, const char* value) {
    auto* device = static_cast<GCodeDevice*>(store);
    if (strcmp(section, device->name) == 0) {
        void (* pFunction)(const char*, GCodeDevice::Config&);
        if (strcmp(name, "s") == 0) {
            device->config.spindle.clear();
            device->config.spindle.push_back(0);
            pFunction = spindleSetF;
        } else if (strcmp(name, "f") == 0) {
            device->config.feed.clear();
            pFunction = feedSetF;
        } else if (strcmp(name, "d") == 0) {
            device->config.dist.clear();
            pFunction = distanceSetF;
        } else
            return 1;
        size_t len = strlen(value);
        char buf[len + 1];
        buf[len] = 0;
        memcpy(buf, value, len);
        char* t = strtok(buf, ",");
        while (t != nullptr) {
            pFunction(t, device->config);
            t = strtok(nullptr, ",");
        }
        return 0;
    }
    return 1;   /* unknown section/name, error */
}

void GCodeDevice::begin(SetupFN* const onBegin) {
    if (onBegin != nullptr) {
        (*onBegin)(printerSerial);
    }
    // in case onBegin did not consume all data in stream
    while (printerSerial.available() > 0) {
        printerSerial.read();
    }
    readLockedStatus();
    if (SD.begin(PIN_CD)) {
        File file = SD.open(PRESET_FILES);
        if (file.available()) {
            //ignore output
            ini_parse_stream([](char* str, int num, void* stream_) -> char* {
                auto stream = *(File*) stream_; //always present
                size_t i = stream.readBytesUntil('\n', str, num);
                if (i > 0) {
                    str[i] = 0;
                    return str;
                } else
                    return nullptr;
            }, &file, (ini_handler) (configHandler), this);
        }
        file.close();
    }
}

void GCodeDevice::readLockedStatus() {
    if (printerSerial.isLocked(true))
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
    constexpr size_t MAX_LINE = 200; // M115 is far longer than 100
    static char resp[MAX_LINE + 1];
    static size_t respLen;

    while (printerSerial.available()) {
        char ch = (char) printerSerial.read();
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
            extendRxTimeout();
        }
    }
}
