#ifndef CNC_3018_JOBFSM_H
#define CNC_3018_JOBFSM_H

#include <SD.h>

#include "etl/fsm.h"
#include "etl/message.h"
#include "constants.h"

#include "WString.h"

const etl::message_router_id_t JOB_BUS_NUMBER = 1;

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
    explicit CompleteMessage(bool byErr): byError(byErr){};
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
#include <etl/observer.h>
#include <math.h>

#include "debug.h"

class Job;

#include "devices/GCodeDevice.h"

typedef int JobStatusEvent;

enum JobStatus {
    REFRESH_SIG
};

class JobFsm : public etl::fsm {

public:
    JobFsm() : etl::fsm(JOB_BUS_NUMBER) {}

    ~JobFsm() {
        closeFile();
    }

    static constexpr size_t MAX_LINE_LEN = 100;
    static constexpr size_t MAX_READ_CHUNK = 4;

    File gcodeFile;
    GCodeDevice* dev;
    //file size + file pos used as % of done
    uint32_t fileSize;
    uint32_t filePos;
    uint32_t startTime;
    uint32_t endTime;
    // add LineNumber and CRC to command
    bool addLineN = false;
    bool pause = false;

    char curLine[MAX_LINE_LEN + 1];
    String* buffer[JOB_BUFFER_SIZE] = {nullptr};

    size_t readLineNum = 0 ;
    size_t currentLineNum = 0 ;

    bool readCommandsToBuffer();

    void setFile(const char* file);

    void closeFile();

    uint32_t getPrintDuration() const;

};

#endif //CNC_3018_JOBFSM_H
