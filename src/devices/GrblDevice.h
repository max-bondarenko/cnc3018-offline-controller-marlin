#pragma once

#include "GCodeDevice.h"
#include <etl/vector.h>

extern WatchedSerial serialCNC;

class GrblDevice : public GCodeDevice {
public:

    enum class GrblStatus {
        Idle, Run, Hold, Jog, Alarm, Door, Check, Home, Sleep
    };

    GrblDevice() : GCodeDevice() {
        canTimeout = true;
        serialRxTimeout = 1;
    };

    ~GrblDevice() override = default;

    bool jog(uint8_t axis, float dist, uint16_t feed) override;

    bool canJog() override;

    void begin() override {
        GCodeDevice::begin();
        schedulePriorityCommand("$I", 2);
        requestStatusUpdate();
    }

    void reset() override {
        lastStatus = DeviceStatus::OK;
        GCodeDevice::cleanupQueue();
        // ^x, reset
        schedulePriorityCommand("\x18", 1);
    }

    void requestStatusUpdate() override {
        if (lastStatus == DeviceStatus::ALARM)
            return; // grbl does not respond in panic anyway
        schedulePriorityCommand("?", 1);
    }

    bool schedulePriorityCommand(const char* cmd, size_t len) override {
        if (lastStatus == DeviceStatus::LOCKED)
            return false;
        if (len == 0) {
            len = strlen(cmd);
        }
        if (isCmdRealtime(cmd, len)) {
            serialCNC.write((const uint8_t*) cmd, len);
            return true;
        } else {
            return GCodeDevice::schedulePriorityCommand(cmd, len);
        }
    }

    /// WPos = MPos - WCO
    float getXOfs() const { return ofsX; }

    float getYOfs() const { return ofsY; }

    float getZOfs() const { return ofsZ; }

    const char* getStatusStr() const;

    static void sendProbe(Stream& serial);

    static bool checkProbeResponse(const char* input);

protected:
    void trySendCommand() override;

    void tryParseResponse(char* cmd, size_t len) override;

private:
    GrblStatus status = GrblStatus::Idle;

    //WPos = MPos - WCO
    float ofsX = 0.0,
        ofsY = 0.0,
        ofsZ = 0.0;

    void parseStatus(char* input);

    void setStatus(const char* s);

    static bool isCmdRealtime(const char* data, size_t len);
};
