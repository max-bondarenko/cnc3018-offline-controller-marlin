#pragma once

#include "GCodeDevice.h"

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

    void jog(uint8_t axis, float dist, uint16_t feed) override;

    bool canJog() override;

    void begin() override {
        GCodeDevice::begin();
        schedulePriorityCommand(GRBL_INFO, strlen(GRBL_INFO));
        requestStatusUpdate();
    }

    void reset() override ;

    void requestStatusUpdate() override ;
    void schedulePriorityCommand(const char* cmd, size_t _len) override ;

    /// WPos = MPos - WCO
    float getXOfs() const { return ofsX; }

    float getYOfs() const { return ofsY; }

    float getZOfs() const { return ofsZ; }

    const char* getStatusStr() const;

    static void sendProbe(Stream& serial);

    static bool checkProbeResponse(const char* input);

protected:

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
