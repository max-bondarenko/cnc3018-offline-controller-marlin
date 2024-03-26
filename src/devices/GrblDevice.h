#pragma once

#include "GCodeDevice.h"
#include <etl/vector.h>

class GrblDevice : public GCodeDevice {
public:
    const etl::vector<u_int16_t, 5> SPINDLE_VALS{0, 1, 10, 100, 1000};

    enum class GrblStatus {
        Idle, Run, Hold, Jog, Alarm, Door, Check, Home, Sleep
    };

    GrblDevice(WatchedSerial* s, Job* job) : GCodeDevice(s, job) {
        canTimeout = false;
    };

    virtual ~GrblDevice() {}

    bool jog(uint8_t axis, int32_t dist, uint16_t feed) override;

    bool canJog() override;

    void begin() override {
        GCodeDevice::begin();
        schedulePriorityCommand("$I");
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

    bool schedulePriorityCommand(const char* cmd, size_t len = 0) override {
        if (txLocked) return false;
        if (len == 0) {
            len = strlen(cmd);
        }
        if (isCmdRealtime(cmd, len)) {
            printerSerial->write((const uint8_t*) cmd, len);
            return true;
        } else {
            return GCodeDevice::schedulePriorityCommand(cmd, len);
        }
    }

    etl::ivector<u_int16_t>* getSpindleValues() const override;

    /// WPos = MPos - WCO
    int32_t getXOfs() const { return ofsX_mil; }

    int32_t getYOfs() const { return ofsY_mil; }

    int32_t getZOfs() const { return ofsZ_mil; }

    const char* getStatusStr() const override;

    static void sendProbe(Stream& serial);

    static bool checkProbeResponse(const String& input);

protected:
    void trySendCommand() override;

    void tryParseResponse(char* cmd, size_t len) override;

private:
    GrblStatus status;

    //WPos = MPos - WCO
    int32_t ofsX_mil = 0,
            ofsY_mil = 0,
            ofsZ_mil = 0;

    void parseStatus(char* input);

    void setStatus(const char* s);

    static bool isCmdRealtime(const char* data, size_t len);

};
