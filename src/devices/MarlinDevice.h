#pragma once

#include "Job.h"
#include "GCodeDevice.h"
#include "gcode/gcode.h"
#include "etl/deque.h"
#include "WString.h"

// TODO list
// TODO done 1 marlin features check
// TODO  2 status polling according marlin state 70%
// TODO     2.1 make it except Job commands feed. same as for grbl
// TODO done kindaa 3 busy state is wierd. make it wait for "unbusy"

static const char* const OK_str = "ok";
static const char* const ERR_str = "Err";
static const char* const ErrorExclamation_str = "!!";
static const char* const BUSY_str = "busy";
static const char* const ECHO_str = "echo";
static const char* const RESEND_str = "Resend";
static const char* const DEBUG_str = "DEBUG";

struct Compat {
    char auto_temp: 1;
    char auto_position: 1;
    char emergency_parser: 1;
};

class MarlinDevice : public GCodeDevice {
public:
    const etl::vector<u_int16_t, 5> SPINDLE_VALS{0, 1, 64, 128, 255};

    static void sendProbe(Stream& serial);

    static bool checkProbeResponse(const String& input);

    //// CONSTRUCTORS
    MarlinDevice(WatchedSerial& _printerSerial, Job& _job);

    virtual ~MarlinDevice() {}

    bool jog(uint8_t axis, float dist, uint16_t feed) override;

    bool canJog() override;

    void begin(SetupFN* const onBegin) override;

    void reset() override;

    void requestStatusUpdate() override;

    bool schedulePriorityCommand(const char* cmd, size_t len) override;

    bool scheduleCommand(const char* cmd, size_t len) override;

    void toggleRelative();

    bool tempChange(uint8_t temp);

    bool isRelative() const { return relative; }

    float getE() const { return e; }

    float getTemp() const { return hotendTemp; }

    uint32_t getHotendPower() const { return hotendPower; }

    float getBedTemp() const { return bedTemp; }

    uint32_t getBedPower() const { return bedPower; }

    /// marlin does not give current spindle value
    /// use this to set value from DRO to see current set value.
    void adjustSpindle(uint32_t val) { spindleVal = val; }

    const Compat& getCompatibilities() const{
        return compatibility;
    }

    bool isExtrusionEnabled() const {
        return minExtrusionTemp <= (int16_t) floor(hotendTemp);
    }

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
    int16_t minExtrusionTemp = -1;
    bool relative = false;
    Compat compatibility;

    void parseError(const char* input);

    void parseOk(const char* input, size_t len);
};
