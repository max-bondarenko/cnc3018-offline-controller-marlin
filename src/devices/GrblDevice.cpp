#include "constants.h"
#include "debug.h"
#include "GrblDevice.h"
#include "etl/string_view.h"

extern WatchedSerial serialCNC;

void GrblDevice::sendProbe(Stream& serial) {
    serial.println();
    serial.println(GRBL_INFO);
}

bool GrblDevice::checkProbeResponse(const char* const input) {
    etl::string_view i{input};
    return i.find("[VER:") != etl::string_view::npos;
}

void GrblDevice::jog(uint8_t axis, float dist, uint16_t feed) {
    constexpr size_t LN = 25;
    char msg[LN];
    // "$J=G91 G20 X0.5" will move +0.5 inches (12.7mm) to X=22.7mm (WPos).
    // Note that G91 and G20 are only applied to this jog command
    int l = snprintf(msg, LN, "$J=G91 F%d %c %.3f", feed, AXIS[axis], dist);
    scheduleCommand(msg, l);
}

bool GrblDevice::canJog() {
    return lastStatus == DeviceStatus::OK && (status == GrblStatus::Idle || status == GrblStatus::Jog);
}

void GrblDevice::reset() {
    lastStatus = DeviceStatus::OK;
    GCodeDevice::cleanupQueue();
    // ^x, reset
    schedulePriorityCommand(GRBL_RESET, 1);
}

bool GrblDevice::isCmdRealtime(const char* data, size_t len) {
    if (len != 1)
        return false;

    char c = data[0];
    switch (c) {
        case '?': // status
        case '~': // cycle start/stop
        case '!': // feedhold
        case 0x18: // ^x, reset
        case 0x84: // door
        case 0x85: // jog cancel
        case 0x9E: // toggle spindle
        case 0xA0: // toggle flood coolant
        case 0xA1: // toggle mist coolant
        case 0x90 ... 0x9D: // feed override, rapid override, spindle override
            return true;
        default:
            return false;
    }
}

void GrblDevice::requestStatusUpdate() {
    if (lastStatus == DeviceStatus::ALARM)
        return; // grbl does not respond in panic anyway
    schedulePriorityCommand(GRBL_STATUS, 1);
    wantUpdate = true;
}

void GrblDevice::schedulePriorityCommand(const char* cmd, size_t _len) {
    if (lastStatus != DeviceStatus::LOCKED) {
        size_t len = _len == 0 ? strlen(cmd) : _len;
        if (isCmdRealtime(cmd, len))
            serialCNC.write((const uint8_t*) cmd, len);
        else
            GCodeDevice::schedulePriorityCommand(cmd, len);
    }
}

void GrblDevice::tryParseResponse(char* _resp, size_t len) {
    etl::string_view resp{_resp, len};
    lastResponse = "";
    if (resp.starts_with(OK_STR)) {
        lastStatus = DeviceStatus::OK;
        ack--;
    } else if (resp.starts_with("<")) {
        parseStatus(_resp + 1);
        // make progres happens
        statusCount++;
        if (statusCount % 21 == 0) {
            auto changed = ack ^ prevAck;
            if (!changed && ack >= JOB_BUFFER_SIZE) {
                ack--;
                statusCount = 1;
            }
        }
        prevAck = ack;
        lastStatus = DeviceStatus::OK;
    } else if (resp.starts_with(ERROR_STR)) {
        LOGF("ERR '%s'\n", resp);
        lastStatus = DeviceStatus::DEV_ERROR;
        lastResponse = _resp;
        ack = 0;
    } else if (resp.starts_with(ALARM_STR)) {
        LOGF("ALARM '%s'\n", resp);
        lastResponse = _resp;
        // no mor status updates will come in, so update status.
        status = GrblStatus::Alarm;
        lastStatus = DeviceStatus::ALARM;
        statusCount = 0;
        ack = 0;
    } else if (resp.find(MSG_STR, 1) != etl::string_view::npos) {
        _resp[len - 1] = 0; // strip last ']'
        lastResponse = _resp + 5;
        // this is the first message after reset
        lastStatus = DeviceStatus::MSG;
    }
    lastStatusStr = getStatusStr();
    DEV_LOGF(">>> '%s'\n", lastResponse);
}

void GrblDevice::parseStatus(char* input) {
    //        + work position
    //        v                             v- work coords offset
    //<Idle|WPos:9.800,0.000,0.000|FS:0,0|WCO:0.000,0.000,0.000>
    //<Idle|MPos:9.800,0.000,0.000|FS:0,0|WCO:0.000,0.000,0.000>
    //        ^ machine position            + buffer state ,ignored
    //                                      |             override values in percent of programmed values for
    //                                      v          v-- feed, rapids, and spindle speed. ignored
    //<Idle|MPos:9.800,0.000,0.000|FS:0,0|Bf:15,128|Ov:100,100,100>
    bool mpos = false;
    char cpy[MAX_LINE_LEN * 2];
    strncpy(cpy, input, MAX_LINE_LEN * 2);
    // status
    char* fromGrbl = strtok(cpy, "|");
    if (fromGrbl == nullptr)
        return;
    setStatus(fromGrbl);
    // MPos:0.000,0.000,0.000 or WPos:0.000,0.000,0.000
    fromGrbl = strtok(nullptr, "|");
    mpos = fromGrbl[0] == 'M' && fromGrbl[1] == 'P'; // starts with "MPos"
    {
        unsigned fromGrblLen = strlen(fromGrbl) + 1;
        char* pos = strtok(fromGrbl + 5, ",");
        for (int i = 0; i < 3 && pos != nullptr; ++i) {
            uint16_t len = strlen(pos) + 1;
            switch (i) {
                case 0:
                    x = strtof(pos, nullptr);
                    break;
                case 1:
                    y = strtof(pos, nullptr);
                    break;
                case 2:
                    z = strtof(pos, nullptr);
                default:
                    continue;
            }
            pos = strtok(pos + len, ",");
        }
        fromGrbl = strtok(fromGrbl + fromGrblLen, "|");
    }
    //    +--  feed
    //    |    +-- spindle or
    //    v    v         v-- feed
    // FS:500,8000  or F:500
    while (fromGrbl != nullptr) {
        if (fromGrbl[0] == 'F') {
            if (fromGrbl[1] == 'S') {
                unsigned fromGrblLen = strlen(fromGrbl) + 1;
                char* pos = strtok(fromGrbl + 3, ",");
                for (int i = 0; i < 2 && pos != nullptr; ++i) {
                    uint16_t len = strlen(pos) + 1;
                    switch (i) {
                        case 0:
                            feed = strtof(pos, nullptr);
                            break;
                        case 1:
                            spindleVal = strtol(pos, nullptr, STRTOLL_BASE);
                        default:
                            continue;
                    }
                    pos = strtok(pos + len, ",");
                }
                fromGrbl = strtok(fromGrbl + fromGrblLen, "|");
                continue;
            } else {
                feed = strtof(fromGrbl + 2, nullptr);
            }
        } else if (fromGrbl[0] == 'W' && fromGrbl[1] == 'C' && fromGrbl[2] == 'O') {
            unsigned fromGrblLen = strlen(fromGrbl) + 1;
            char* pos = strtok(fromGrbl + 4, ",");
            for (int i = 0; i < 3 && pos != nullptr; ++i) {
                uint16_t len = strlen(pos) + 1;
                switch (i) {
                    case 0:
                        ofsX = strtof(pos, nullptr);
                        break;
                    case 1:
                        ofsY = strtof(pos, nullptr);
                        break;
                    case 2:
                        ofsZ = strtof(pos, nullptr);
                    default:
                        continue;
                }
                pos = strtok(pos + len, ",");
            }
            fromGrbl = strtok(fromGrbl + fromGrblLen, "|");
            continue;
        } else if (fromGrbl[0] == '0' && fromGrbl[1] == 'v') {
            //                                                  +-- feed
            //                                                  |   +--rapid moves max 100, min 25
            //                                                  v   v   v-- spindle speed
            //<Idle|MPos:9.800,0.000,0.000|FS:0,0|Bf:15,128|Ov:100,100,100>
            unsigned fromGrblLen = strlen(fromGrbl) + 1;
            char* pos = strtok(fromGrbl + 3, ",");
            for (int i = 0; i < 3 && pos != nullptr; ++i) {
                uint16_t len = strlen(pos) + 1;
                switch (i) {
                    //min 10%, max 200%
                    case 0:
                        feedrate = (uint8_t) strtol(pos, nullptr, STRTOLL_BASE);
                        break;
                    case 1:
                        rapidrate = (uint8_t) strtol(pos, nullptr, STRTOLL_BASE);
                        break;
                    case 2:
                        spindlerate = (uint8_t) strtol(pos, nullptr, STRTOLL_BASE);
                    default:
                        continue;
                }
                pos = strtok(pos + len, ",");
            }
            fromGrbl = strtok(fromGrbl + fromGrblLen, "|");
            continue;
        }
        fromGrbl = strtok(nullptr, "|");
    }
/*
N.B. From Grbl wiki:

Machine position and work position are related by this simple equation per axis: WPos = MPos - WCO

GUI Developers: Simply track and retain the last __WCO vector__ and use the above
 equation to compute the other position vector for your position readouts. If Grbl's
 status reports show either WPos or MPos, just follow the equations below. It's as easy as that!
    If WPos: is given, use MPos = WPos + WCO.
    If MPos: is given, use WPos = MPos - WCO.
 */
    if (!mpos) {
        x -= ofsX;
        y -= ofsY;
        z -= ofsZ;
    }
}

void GrblDevice::setStatus(const char* pch) {
    etl::string_view statusStr{pch};
    if (statusStr.starts_with(HOLD_STR)) status = GrblStatus::Hold;
    else if (statusStr.starts_with(DOOR_STR)) status = GrblStatus::Door;
    else if (statusStr.starts_with(IDLE_STR)) status = GrblStatus::Idle;
    else if (statusStr.starts_with(RUN_STR)) status = GrblStatus::Run;
    else if (statusStr.starts_with(JOG_STR)) status = GrblStatus::Jog;
    else if (statusStr.starts_with(Alarm_STR)) status = GrblStatus::Alarm;
    else if (statusStr.starts_with(CHECK_STR)) status = GrblStatus::Check;
    else if (statusStr.starts_with(HOME_STR)) status = GrblStatus::Home;
    else if (statusStr.starts_with(SLEEP_STR)) status = GrblStatus::Sleep;
}

const char* GrblDevice::getStatusStr() const {
    switch (status) {
        case GrblStatus::Idle:
            return IDLE_STR;
        case GrblStatus::Run:
            return RUN_STR;
        case GrblStatus::Jog:
            return JOG_STR;
        case GrblStatus::Alarm:
            return Alarm_STR;
        case GrblStatus::Hold:
            return HOLD_STR;
        case GrblStatus::Door:
            return DOOR_STR;
        case GrblStatus::Check:
            return CHECK_STR;
        case GrblStatus::Home:
            return HOME_STR;
        case GrblStatus::Sleep:
            return SLEEP_STR;
        default:
            return "?";
    }
}
