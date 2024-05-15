#pragma once

#include <etl/observer.h>
#include "WatchedSerial.h"
#include "gcode/gcode.h"

class Job;

extern WatchedSerial serialCNC;

#include "util.h"
#include "debug.h"
#include "Buffer.h"

// todo done switch to states from flags for protocol state
// todo done 5 move read preset out of begin()

/// Device abstraction statuses.
/// Real Device can have different statuses,
/// but they can be mapped to this.
/// Use it for general communication with DRO and Job.
struct DeviceStatus {
    enum {
        NONE,
        OK,
        MSG,
        RESEND,
        // -------- WAIT states
        WAIT = 10,
        BUSY,
        // -------- Dysfunctional states
        DISCONNECTED = 20,
        LOCKED,
        // -------- Error states
        ALARM = 100,
        DEV_ERROR,
    };
};

enum class DeviceEvent {
    REFRESH
};

using DeviceObserver = etl::observer<DeviceEvent>;

class GCodeDevice : public etl::observable<DeviceObserver, 2> {
public:
    typedef Buffer<JOB_BUFFER_SIZE * MAX_LINE_LEN> DevBuffer;

    struct Command {
        uint8_t len;
        uint8_t str[99];

        Command(uint8_t _length, const char* buf) : len{_length} {
            memcpy(str, buf, len);
            str[len] = 0;
        };
    };

    struct Config {
        etl::vector<u_int16_t, 10> spindle{0, 1, 10, 100, 1000};
        etl::vector<u_int16_t, 10> feed{50, 100, 500, 1000, 2000};
        etl::vector<float, 10> dist{0.1, 0.5, 1, 5, 10, 50, 150};
    };
    const char* name = nullptr;
    Config config;
    const char* lastResponse = nullptr;
    const char* lastStatusStr = nullptr;
    size_t lastStatus;

    GCodeDevice() : lastStatus{DeviceStatus::NONE} {}

    virtual ~GCodeDevice() { clear_observers(); }

    virtual void begin();

    bool canSchedule() const;

    virtual void scheduleCommand(const char* cmd, size_t len);

    /// Schedule just put it to command var. sendCommands() do work for send.
    ///
    ///  Priority commands are commands for polling status or jogging.
    ///  It has no N<\d> number prefix, if line numbers are used.
    ///
    /// \param cmd
    /// \param len
    /// \return false if fail to schedule.
    virtual void schedulePriorityCommand(const char* cmd, size_t len);

    virtual void jog(uint8_t axis, float dist, uint16_t feed) = 0;

    virtual void reset() = 0;

    void ackResend() { resendLine = -1; };

    virtual void requestStatusUpdate() = 0;

    virtual void step();

    virtual void receiveResponses();

    virtual bool canJog() { return true; }

    bool supportLineNumber() const { return useLineNumber; }

    float getX() const { return x; }

    float getY() const { return y; }

    float getZ() const { return z; }

    uint32_t getSpindleVal() const { return spindleVal; }

    float getFeed() const { return feed; }

    int32_t getResendLine() const { return resendLine; }

    const Config& getConfig() const { return config; }

    // feedrate adjustment percent value.max 200% for Grbl for all
    uint8_t feedrate = 100;
    // flowrate adjustment percent value.
    uint8_t flowrate = 100;
    // flowrate adjustment percent value.
    uint8_t rapidrate = 100;
    //only in grbl
    uint8_t spindlerate = 100;


protected:
    DevBuffer buffer;
    bool canTimeout,
        xoff,
        xoffEnabled = false,
        useLineNumber = false;

    bool wantUpdate = false;
    size_t responseLen = 0;
    char responseBuffer[MAX_LINE_LEN * 2];
    Command priorityCmd{0, {0}};

    float x = 0.0,
        y = 0.0,
        z = 0.0,
        feed = 0.0;

    uint32_t spindleVal = 0,
        serialRxTimeout = 0;

    int32_t resendLine = -1;

    int ack = 0, prevAck;
    uint8_t statusCount =0;

    bool isRxTimeoutEnabled() const;

    void cleanupQueue();

    virtual void trySendCommand();

    virtual void tryParseResponse(char* cmd, size_t len) = 0;

    void readLockedStatus();

private:
    void extendRxTimeout();

    void disarmRxTimeout();

    void checkTimeout();
};
