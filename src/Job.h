#pragma once

#include "JobFsm.h"
#include "gcode/gcode.h"
#include "debug.h"

// TODO done 1 add prev state. added as transition from WAIT to READY, then PAUSE
// TODO done refactor buffers
// TODO done fix Pause

enum class JobEvent {
    REFRESH,
    DONE
};
typedef etl::observer<JobEvent> JobObserver;

class InitState : public etl::fsm_state<JobFsm, InitState, StateId::INIT, SetFileMessage> {
public:
    etl::fsm_state_id_t on_event(const SetFileMessage& event) {
        get_fsm_context().setFile(event.fileName);
        return StateId::READY;
    }

    etl::fsm_state_id_t on_event_unknown(const etl::imessage& msg) {
        LOGF("error state: %d", msg.get_message_id());
        return STATE_ID;
    }
};

class FinishState : public etl::fsm_state<JobFsm, FinishState, StateId::FINISH, SetFileMessage> {
public:
    etl::fsm_state_id_t on_enter_state() {
        get_fsm_context().endTime = millis();
        get_fsm_context().dev->reset();
//        dro->enableRefresh(true);
        return STATE_ID;
    }

    etl::fsm_state_id_t on_event(const SetFileMessage& event) {
        get_fsm_context().closeFile();
        get_fsm_context().setFile(event.fileName);
        get_fsm_context().dev->scheduleCommand(RESET_LINE_NUMBER, strlen(RESET_LINE_NUMBER));
        return StateId::READY;
    }

    etl::fsm_state_id_t on_event_unknown(const etl::imessage& msg) {
        LOGF("error state: %d", msg.get_message_id());
        return STATE_ID;
    }
};

class ErrorState : public etl::fsm_state<JobFsm, ErrorState, StateId::ERROR, SetFileMessage> {
public:
    etl::fsm_state_id_t on_enter_state() {
        get_fsm_context().endTime = millis();
        get_fsm_context().dev->reset();
//        dro->enableRefresh(true);
        return STATE_ID;
    }

    etl::fsm_state_id_t on_event(const SetFileMessage& event) {
        get_fsm_context().closeFile();
        get_fsm_context().setFile(event.fileName);
        get_fsm_context().dev->scheduleCommand(RESET_LINE_NUMBER, strlen(RESET_LINE_NUMBER));
        return StateId::READY;
    }

    etl::fsm_state_id_t on_event_unknown(const etl::imessage& msg) {
        LOGF("error state: %d", msg.get_message_id());
        return STATE_ID;
    }
};


class ReadyState : public etl::fsm_state<JobFsm, ReadyState, StateId::READY,
    StartMessage, SendMessage, PauseMessage, CompleteMessage> {
public:

    etl::fsm_state_id_t on_event(const StartMessage& event) {
        get_fsm_context().startTime = millis();
        return StateId::READY;
    }

    etl::fsm_state_id_t on_event(const SendMessage& event) {
        return StateId::WAIT;
    }

    etl::fsm_state_id_t on_event(const PauseMessage& event) {
        get_fsm_context().endTime = millis() - get_fsm_context().startTime;
        return StateId::PAUSED;
    }

    etl::fsm_state_id_t on_event(const CompleteMessage& event) {
        return event.byError ? StateId::ERROR : StateId::FINISH;
    }

    etl::fsm_state_id_t on_event_unknown(const etl::imessage& msg) {
        LOGF("error state: %d", msg.get_message_id());
        return STATE_ID;
    }
};

class PauseState : public etl::fsm_state<JobFsm, PauseState, StateId::PAUSED, ResumeMessage, CompleteMessage> {
public:
    etl::fsm_state_id_t on_enter_state() {
//        dro->enableRefresh(true);
        return STATE_ID;
    }

    etl::fsm_state_id_t on_event(const ResumeMessage& event) {
//        dro->enableRefresh(false);
        return StateId::READY;
    }

    etl::fsm_state_id_t on_event(const CompleteMessage& event) {
        return StateId::FINISH;
    }

    etl::fsm_state_id_t on_event_unknown(const etl::imessage& msg) {
        LOGF("error state: %d", msg.get_message_id());
        return STATE_ID;
    }
};

class WaitState
    : public etl::fsm_state<JobFsm, WaitState, StateId::WAIT, ContinueMessage, PauseMessage, CompleteMessage> {
public:

    etl::fsm_state_id_t on_event(const PauseMessage& event) {
        get_fsm_context().pause = true;
        return STATE_ID;
    }

    etl::fsm_state_id_t on_event(const CompleteMessage& event) {
        return event.byError ? StateId::ERROR : StateId::FINISH;
    }

    etl::fsm_state_id_t on_event(const ContinueMessage& event) {
        return StateId::READY;
    }

    etl::fsm_state_id_t on_event_unknown(const etl::imessage& msg) {
        LOGF("error state: %d", msg.get_message_id());
        return STATE_ID;
    }
};

///
/// Represents gcode program read from SD card.
/// Shim class abstracting FSM as method calls.
///
class Job : public etl::observable<JobObserver, 2> {
    JobFsm* fsm;
    InitState initState;
    FinishState finishState;
    ErrorState errorState;
    ReadyState readyState;
    WaitState waitState;
    PauseState pauseState;
    etl::ifsm_state* stateList[StateId::NUMBER_OF_STATES] =
        {&initState, &finishState, &errorState, &readyState, &waitState, &pauseState};
public:

    Job() {
        fsm = new JobFsm();
        fsm->set_states(stateList, etl::size(stateList));
        fsm->start(false);
    }

    void inline setDevice(GCodeDevice* dev_) {
        fsm->dev = dev_;
        fsm->addLineN = dev_->supportLineNumber();
    }

    void inline setFile(const char* file) {
        auto m = SetFileMessage{};
        m.fileName = file;
        fsm->pause = false; //reset pause if any
        fsm->receive(m);
    }

    void start() {
        fsm->receive(StartMessage{});
//        dro->enableRefresh(false);
    }

    void stop() {
        fsm->receive(CompleteMessage{false});
    }

    void setPaused(bool v) {
        if (v)
            fsm->receive(PauseMessage{});
        else if (fsm->get_state_id() == StateId::PAUSED)
            fsm->receive(ResumeMessage{});
    }

    bool inline isRunning() {
        etl::fsm_state_id_t i = fsm->get_state_id();
        return i == StateId::READY || i == StateId::WAIT;
    }

    bool inline isValid() {
        return fsm->get_state_id() != StateId::INIT;
    }

    bool inline isError() {
        return StateId::ERROR == fsm->get_state_id();
    }

    uint8_t getCompletion() {
        if (isValid())
            return round((100.0 * fsm->filePos) / (fsm->fileSize > 0 ? fsm->fileSize : 1));
        else
            return 0;
    }

    void step() {
        switch (fsm->get_state_id()) {
            case StateId::READY:
                if (fsm->pause) {
                    fsm->pause = false;
                    fsm->receive(PauseMessage{});
                } else if (fsm->readCommandsToBuffer()) {
                    fsm->receive(SendMessage{});
                } else {
                    fsm->receive(CompleteMessage{false});
                }
                break;
            case StateId::WAIT:
                switch (fsm->dev->lastStatus) {
                    case DeviceStatus::OK:
                        fsm->receive(ContinueMessage{});
                        break;
                    case DeviceStatus::DEV_ERROR:
                        fsm->receive(CompleteMessage{true});
                        break;
                    case DeviceStatus::RESEND: {
                        int32_t line = fsm->dev->getResendLine();
                        if (line >= 0) { // TODO hack
                            fsm->resendLineNum = line;
                            fsm->dev->ackResend();
                        }
                        fsm->receive(ContinueMessage{});
                        break;
                    }
                    case DeviceStatus::BUSY:
                    case DeviceStatus::WAIT:
                    default:
                        break;
                }
                break;
            case StateId::PAUSED:
            default:
                return;
        }
    }
};