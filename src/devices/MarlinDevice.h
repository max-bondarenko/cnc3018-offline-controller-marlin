#pragma once

#include "Job.h"
#include "GCodeDevice.h"
#include "gcode/gcode.h"
#include "etl/deque.h"
#include "WString.h"


class MarlinDevice : public GCodeDevice {
public:
    const etl::vector<u_int16_t, 5> SPINDLE_VALS{0, 1, 64, 128, 255};

    static void sendProbe(Stream& serial);

    static bool checkProbeResponse(const String& input);

    //// CONSTRUCTORS
    MarlinDevice(WatchedSerial* s, Job* job);

    virtual ~MarlinDevice() {}

    etl::ivector<u_int16_t>* getSpindleValues() const override;

    bool jog(uint8_t axis, float dist, uint16_t feed) override;

    bool canJog() override;

    void begin() override;

    void reset() override;

    void requestStatusUpdate() override;

    bool schedulePriorityCommand(const char* cmd, size_t len) override;

    bool scheduleCommand(const char* cmd, size_t len) override;

    const char* getStatusStr() const override;

    void toggleRelative();

    bool tempChange(uint8_t temp);

    bool isRelative() const { return relative; }

    float getE() const { return e; }

    float getTemp() const { return hotendTemp; } // todo check SIGN

    uint32_t getHotendPower() const { return hotendPower; }

    float getBedTemp() const { return bedTemp; }

    uint32_t getBedPower() const { return bedPower; }

    /// marlin does not give current spindle value
    /// use this to set value from DRO to see current set value.
    void adjustSpindle(uint32_t val) { spindleVal = val; }

protected:
    void trySendCommand() override;

    void tryParseResponse(char* cmd, size_t len) override;

private:
    etl::deque<String, 10> outQueue;

    float e = 0.0;
    float hotendTemp = 0.0,
            bedTemp = 0.0;
    /// set indirectly by Gcode command
    uint32_t hotendRequestedTemp = 0,
    /// set indirectly by Gcode command
    bedRequestedTemp = 0,
            hotendPower = 0,
            bedPower = 0;

    bool relative = false;

    int32_t resendLine = -1;

    void parseError(const char* input);

    void parseOk(const char* input, size_t len);
};
