#ifndef CNC_3018_JOBFSM_H
#define CNC_3018_JOBFSM_H

#include <SD.h>
#include <etl/fsm.h>
#include "constants.h"
#include "observeble_states.h"

struct StateId {
    enum {
        INIT,
        FINISH,
        ERROR,
        READY,
        WAIT,
        PAUSED,
        NUMBER_OF_STATES
    };
};

struct EventId {
    enum {
        FILE,
        START,
        PAUSE,
        RESUME,
        COMPLETE,
        SEND,
        ACK,
        RESEND
    };
};

struct SetFileMessage : public etl::message<EventId::FILE> {
    const char* fileName;
};

struct CompleteMessage : public etl::message<EventId::COMPLETE> {
    bool byError;

    explicit CompleteMessage(bool byErr) : byError(byErr) {};
};

struct StartMessage : public etl::message<EventId::START> {
};
struct PauseMessage : public etl::message<EventId::PAUSE> {
};
struct ResumeMessage : public etl::message<EventId::RESUME> {
};
struct SendMessage : public etl::message<EventId::SEND> {
};
struct ContinueMessage : public etl::message<EventId::ACK> {
};

#include <Arduino.h>
#include <SD.h>
#include <math.h>

#include "debug.h"

class Job;

#include "devices/GCodeDevice.h"

struct CmdInFile {
    size_t position;
    // source command no longer 96, usually 40~50  add #: +6, add checksum: +4~5
    uint8_t length;
};

class JobFsm : public etl::fsm, public etl::observable<JobObserver, 1> {

public:
    JobFsm() : etl::fsm(JOB_FSM_NUMBER) {}

    ~JobFsm() {
        closeFile();
    }

    Buffer<sizeof(CmdInFile) * 100, CmdInFile> cmdBuffer;
    File gcodeFile;
    GCodeDevice* dev;
    //fileSize + filePos used as % of done
    uint32_t fileSize;
    uint32_t filePos;
    uint32_t startTime;
    uint32_t endTime;
    // add LineNumber and CRC to command
    bool addLineN = false;
    bool pause = false;
    size_t readLineNum = 0;
    size_t resendLineNum = 0xFFFF;

    bool readCommandsToBuffer();

    void setFile(const char* file);

    void closeFile();

    uint32_t getPrintDuration() const;


private:
    static inline uint8_t calculateChecksum(const char* out, uint8_t count);

    static inline void buildCommand(String& line, const char* curLine, size_t lineNumber, bool addLineN);
};

#endif //CNC_3018_JOBFSM_H
