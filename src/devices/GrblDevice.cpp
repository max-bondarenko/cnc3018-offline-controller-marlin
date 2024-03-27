#include "util.h"
#include "constants.h"
#include "GrblDevice.h"
#include "Job.h"

void GrblDevice::sendProbe(Stream& serial) {
    serial.print("\n$I\n");
}

bool GrblDevice::checkProbeResponse(const String& input) {
    if (input.indexOf("[VER:") != -1) {
        return true;
    }
    return false;
}

bool GrblDevice::jog(uint8_t axis, float dist, uint16_t feed) {
    constexpr size_t LN = 25;
    char msg[LN];
    // "$J=G91 G20 X0.5" will move +0.5 inches (12.7mm) to X=22.7mm (WPos).
    // Note that G91 and G20 are only applied to this jog command
    int l = snprintf(msg, LN, "$J=G91 F%d %c %.3f", feed, AXIS[axis], dist);
    return scheduleCommand(msg, l);
}

bool GrblDevice::canJog() {
    return lastStatus == DeviceStatus::OK && (status == GrblStatus::Idle || status == GrblStatus::Jog);
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

void GrblDevice::trySendCommand() {
    LOGLN("Try send");
    size_t& len = curUnsentPriorityCmdLen != 0 ? curUnsentPriorityCmdLen : curUnsentCmdLen;
    if (len == 0)
        return;
    char* cmd = curUnsentPriorityCmdLen != 0 ? &curUnsentPriorityCmd[0] : &curUnsentCmd[0];
    cmd[len] = 0;
    if (printerSerial->availableForWrite()) {
        LOGLN("> send");
        printerSerial->write((const uint8_t*) cmd, len);
        printerSerial->write('\n');
        len = 0;
    }
}

void GrblDevice::tryParseResponse(char* resp, size_t len) {
    if (startsWith(resp, "ok")) {
        lastStatus = DeviceStatus::OK;
    } else if (startsWith(resp, "<")) {
        parseStatus(resp + 1);
        lastStatus = DeviceStatus::OK;
    } else if (startsWith(resp, "error")) {
        LOGF("ERR '%s'\n", resp);
        lastStatus = DeviceStatus::DEV_ERROR;
        lastResponse = resp;
    } else if (startsWith(resp, "ALARM:")) {
        LOGF("ALARM '%s'\n", resp);
        lastResponse = resp;
        // no mor status updates will come in, so update status.
        status = GrblStatus::Alarm;
        lastStatus = DeviceStatus::ALARM;
    } else if (startsWith(resp, "[MSG:")) {
        LOGF("Msg '%s'\n", resp);
        resp[len - 1] = 0; // strip last ']'
        lastResponse = resp + 5;
        // this is the first message after reset
        lastStatus = DeviceStatus::MSG;
    }
    if (lastStatus <= DeviceStatus::MSG) {
        lastResponse = getStatusStr();
    }
    LOGF("> '%s'\n", resp);
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
    char cpy[BUFFER_LEN];
    strncpy(cpy, input, BUFFER_LEN);
    // status
    char* fromGrbl = strtok(cpy, "|");
    if (fromGrbl == nullptr)
        return;
    setStatus(fromGrbl);
    // MPos:0.000,0.000,0.000 or WPos:0.000,0.000,0.000
    fromGrbl = strtok(nullptr, "|");
    mpos = startsWith(fromGrbl, "MPos");
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
    //    v   v-- spindle or
    //                  v-- feed
    // FS:500,8000  or F:500
    while (fromGrbl != nullptr) {
        if (startsWith(fromGrbl, "FS:") || startsWith(fromGrbl, "F:")) {
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
        } else if (startsWith(fromGrbl, "WCO:")) {
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
    if (startsWith(pch, "Hold")) status = GrblStatus::Hold;
    else if (startsWith(pch, "Door")) status = GrblStatus::Door;
    else if (strcmp(pch, "Idle") == 0) status = GrblStatus::Idle;
    else if (strcmp(pch, "Run") == 0) status = GrblStatus::Run;
    else if (strcmp(pch, "Jog") == 0) status = GrblStatus::Jog;
    else if (strcmp(pch, "Alarm") == 0) status = GrblStatus::Alarm;
    else if (strcmp(pch, "Check") == 0) status = GrblStatus::Check;
    else if (strcmp(pch, "Home") == 0) status = GrblStatus::Home;
    else if (strcmp(pch, "Sleep") == 0) status = GrblStatus::Sleep;
    LOGF("Parsed GrblStatus: %d\n", status);
}

const char* GrblDevice::getStatusStr() const {
    switch (status) {
        case GrblStatus::Idle:
            return "Idle";
        case GrblStatus::Run:
            return "Run";
        case GrblStatus::Jog:
            return "Jog";
        case GrblStatus::Alarm:
            return "Alarm";
        case GrblStatus::Hold:
            return "Hold";
        case GrblStatus::Door:
            return "Door";
        case GrblStatus::Check:
            return "Check";
        case GrblStatus::Home:
            return "Home";
        case GrblStatus::Sleep:
            return "Sleep";
        default:
            return "?";
    }
}

etl::ivector<u_int16_t>* GrblDevice::getSpindleValues() const {
    return (spindleValues->size() > 1) ? spindleValues : (etl::ivector<u_int16_t>*) &(GrblDevice::SPINDLE_VALS);
}
