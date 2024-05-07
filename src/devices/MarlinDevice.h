#pragma once

#include "Job.h"
#include "GCodeDevice.h"
#include "gcode/gcode.h"
#include "etl/string_view.h"

// TODO list
// TODO done 1 marlin features check
// TODO  2 status polling according marlin state 70%
// TODO     2.1 make it except Job commands feed. same as for grbl
// TODO done kindaa 3 busy state is wierd. make it wait for "unbusy"

static const char* const OK_str = "ok";
static const char* const START_str = "start";
static const char* const ERR_str = "Err";
static const char* const ERROR_EXCLAMATION_str = "!!";
static const char* const BUSY_str = "busy";
static const char* const ECHO_str = "echo";
static const char* const RESEND_str = "Resend";
static const char* const DEBUG_str = "DEBUG";

extern Job job;

class MarlinDevice : public GCodeDevice {
public:
    struct Compatibility {
        bool auto_temp: 1;
        bool auto_position: 1;
        bool emergency_parser: 1;

        explicit Compatibility() : auto_temp{false}, auto_position{false}, emergency_parser{false} {};
    };

    const etl::vector<u_int16_t, 5> SPINDLE_VALS{0, 1, 64, 128, 255};

    static void sendProbe(Stream& serial);

    static bool checkProbeResponse(const char* input);

    //// CONSTRUCTORS
    MarlinDevice();

    virtual ~MarlinDevice() {}

    void jog(uint8_t axis, float dist, uint16_t feed) override;

    bool canJog() override;

    void begin() override;

    void reset() override;

    void requestStatusUpdate() override;

    void schedulePriorityCommand(const char* cmd, size_t len) override;

    bool canSchedule() const override;

    void scheduleCommand(const char* cmd, size_t len) override;

    void toggleRelative();

    void tempChange(uint8_t temp);

    void bedTempChange(uint8_t temp);

    bool isRelative() const { return relative; }

    float getE() const { return e; }

    float getTemp() const { return hotendTemp; }

    uint32_t getHotendPower() const { return hotendPower; }

    uint32_t getHotendRequestedTemp() const { return hotendRequestedTemp; }

    float getBedTemp() const { return bedTemp; }

    uint32_t getBedPower() const { return bedPower; }

    uint32_t getBedRequestedTemp() const { return bedRequestedTemp; }

    /// marlin does not give current spindle value
    /// use this to set value from DRO to see current set value.
    void adjustSpindle(uint32_t val) { spindleVal = val; }

    void setup();

    const Compatibility& getCompatibilities() const {
        return compatibility;
    }

    bool isExtrusionEnabled() const {
        return minExtrusionTemp <= (int16_t) floor(hotendTemp);
    }

protected:
    void trySendCommand() override;

    void tryParseResponse(char* cmd, size_t len) override;

private:
    struct Command {
        uint8_t len;
        uint8_t str[99];
    };
    Compatibility compatibility;
    float e = 0.0;
    float hotendTemp = 0.0,
        bedTemp = 0.0;
    /// set indirectly by Gcode command
    uint32_t hotendRequestedTemp = 0,
    /// set indirectly by Gcode command
    bedRequestedTemp = 0,
        hotendPower = 0,
        bedPower = 0;
    int16_t minExtrusionTemp = -1;
    bool relative = false;
    int ack = 0;

    void parseOk(const char* input, size_t len);
};
