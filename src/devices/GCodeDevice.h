#pragma once

#include <Arduino.h>
#include <etl/vector.h>
#include "WString.h"
#include <etl/observer.h>
#include "WatchedSerial.h"

class Job;

#include "util.h"
#include "debug.h"


// TODO LIST
// TODO 1 switch to states from flags for protocol state  90%
// TODO 2 make observable events meaningful 50%
// TODO 3 check RxTimeout , make it work as heartbeat

///
/// Device abstraction statuses.
/// Real Device can have different statuses,
/// but they can be mapped to this.
/// Use it for general communication with DRO and Job.
struct DeviceStatus {
    enum {
        NONE,
        DISCONNECTED,
        LOCKED,
        OK,
        MSG,
        RESEND,
        // -------- WAIT states
        WAIT = 10,
        BUSY,
        // -------- Error states
        ALARM = 100,
        DEV_ERROR,
    };
};
struct DeviceStatusEvent {
    size_t status;
    String str;
};

using DeviceObserver = etl::observer<const DeviceStatusEvent&>;

class GCodeDevice : public etl::observable<DeviceObserver, 2> {
public:
    constexpr static uint8_t MAX_GCODE_LINE = 96;
    constexpr static uint8_t BUFFER_LEN = 255;
    constexpr static uint8_t SHORT_BUFFER_LEN = 100;

    GCodeDevice(WatchedSerial* s, Job* job_) : printerSerial(s), job(job_) {
        spindleValues = new etl::vector<u_int16_t, 10>{0};
    }

    virtual ~GCodeDevice() { clear_observers(); }

    virtual void begin();

    virtual bool scheduleCommand(const char* cmd, size_t len);

    /// Schedule just put it to command var. sendCommands() do work for send.
    ///
    ///  Priority commands are commands for polling status or jogging.
    ///  It has no N<\d> number prefix, if line numbers are used.
    ///
    /// \param cmd
    /// \param len
    /// \return false if fail to schedule.
    virtual bool schedulePriorityCommand(const char* cmd, size_t len);

    virtual bool jog(uint8_t axis, float dist, uint16_t feed) = 0;

    virtual void reset() = 0;

    virtual void requestStatusUpdate() = 0;

    virtual const char* getStatusStr() const = 0;

    virtual void step();

    virtual void receiveResponses();

    virtual bool canJog() { return true; }

    bool supportLineNumber() const { return useLineNumber; }

    virtual etl::ivector<u_int16_t>* getSpindleValues() const = 0;

    float getX() const { return x; }

    float getY() const { return y; }

    float getZ() const { return z; }

    uint32_t getSpindleVal() const { return spindleVal; }

    float getFeed() const { return feed; }

    size_t getLastStatus() const { return lastStatus; }

protected:
    WatchedSerial* printerSerial;
    Job* job;
    etl::ivector<u_int16_t>* spindleValues;

    bool canTimeout,
            xoff,
            xoffEnabled = false,
            txLocked = false,
            useLineNumber = false;

    char curUnsentCmd[MAX_GCODE_LINE + 1], curUnsentPriorityCmd[MAX_GCODE_LINE + 1];

    size_t curUnsentCmdLen = 0,
            curUnsentPriorityCmdLen = 0;

    const char* lastResponse = nullptr;
    size_t lastStatus = DeviceStatus::NONE;


    float x = 0.0,
            y = 0.0,
            z = 0.0,
            feed = 0.0;

    uint32_t spindleVal = 0,
            serialRxTimeout = 0;

    void armRxTimeout();

    void disarmRxTimeout();

    bool isRxTimeoutEnabled() const;

    void checkTimeout();

    void cleanupQueue();

    virtual void trySendCommand() = 0;

    virtual void tryParseResponse(char* cmd, size_t len) = 0;

    void readLockedStatus();
};
