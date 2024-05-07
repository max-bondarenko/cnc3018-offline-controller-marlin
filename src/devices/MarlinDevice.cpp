#include "constants.h"
#include "MarlinDevice.h"
#include "util.h"
#include "debug.h"


#ifdef LOG_DEBUG
    #define LOG_EXTRA_INFO(a) a;
#else
    #define LOG_EXTRA_INFO(a) //noop
#endif

extern WatchedSerial serialCNC;

void MarlinDevice::sendProbe(Stream& serial) {
    serial.println();
    serial.println(M115_GET_FIRMWARE_VER);
}

/// FIRMWARE_NAME:Marlin
bool MarlinDevice::checkProbeResponse(const char* const input) {
    etl::string_view i{input};
    return i.find("Marlin") != etl::string_view::npos;
}

MarlinDevice::MarlinDevice() : GCodeDevice() {
    canTimeout = false; //todo switch it on after
    useLineNumber = true;
    config.spindle = etl::vector<u_int16_t, 10>{0, 1, 64, 255};
}

void MarlinDevice::jog(uint8_t axis, float dist, uint16_t feed) {
    constexpr size_t LN = 25;
    char msg[LN];
    // adding G91_SET_RELATIVE_COORDINATES to command does not work.
    int l = snprintf(msg, LN, "G0 F%d %c %.3f", feed, AXIS[axis], dist);
    scheduleCommand(msg, l);
}

void MarlinDevice::trySendCommand() {
    if (lastStatus >= DeviceStatus::WAIT) {
        return;
    }
    if (!buffer.empty()) {
        for (auto i = buffer.readEnd<Command>(), end = buffer.end<Command>();
             ack < JOB_BUFFER_SIZE && i != end && serialCNC.availableForWrite();
             ++i) {
            const Command& cmd = *i;
            uint8_t len = cmd.len;
            IO_LOGF("> [%s]\n", cmd.str);
            serialCNC.write(cmd.str, len);
            serialCNC.write('\n');
            --buffer;
            ack++;
        }
        lastResponse = nullptr;
    }
}

bool MarlinDevice::canSchedule() const {
    return !buffer.full<Command>();
}

void MarlinDevice::scheduleCommand(const char* cmd, size_t len) {
    Command a;
    a.len = len;
    memcpy(a.str, cmd, len);
    a.str[len] = 0;
    buffer.push(a);
}

void MarlinDevice::schedulePriorityCommand(const char* cmd, size_t len) {
    return scheduleCommand(cmd, len);
}

void MarlinDevice::begin() {
    GCodeDevice::begin();
    constexpr size_t LN = 8;
    char msg[LN];
    // OPTIMIZATION
    msg[4] = ' ';
    msg[5] = 'S';
    msg[6] = '1';
    msg[7] = 0;
    if (compatibility.auto_position) {
        memcpy(msg, M154_AUTO_REPORT_POSITION, 4);
        scheduleCommand(msg, 9);
    }
    if (compatibility.auto_temp) {
        memcpy(msg, M155_AUTO_REPORT_TEMP, 4);
        scheduleCommand(msg, 9);
    }
    scheduleCommand(RESET_LINE_NUMBER, 8);
    scheduleCommand(M302_COLD_EXTRUDER_STATUS, 5);
}

void MarlinDevice::setup() {
    WatchedSerial& s = serialCNC;
    char buffer[MAX_LINE_LEN + 1]; // assume less than 100B
    // do not check readBytesUntil.return value, let it read
    for (int i = 50; i > 0; i--) {
        size_t read = s.readBytesUntil('\n', buffer, 100);
        if (read > 0) {
            IO_LOGLN(buffer);
            etl::string_view compatString{buffer};
            if (compatString.starts_with("Cap:")) {
                if (compatString.find("AUTOREPORT_") != etl::string_view::npos) {
                    if (compatString.find("TEMP", 14) != etl::string_view::npos &&
                        compatString.find(":1", 15) != etl::string_view::npos) {
                        this->compatibility.auto_temp = true;
                    } else if (compatString.find("POS", 14) != etl::string_view::npos &&
                               compatString.find(":1", 15) != etl::string_view::npos) {
                        this->compatibility.auto_position = true;
                    }
                } else if (compatString.find("EMERGENCY_PARSER") != etl::string_view::npos &&
                           compatString.find(":1", 16) != etl::string_view::npos) {
                    this->compatibility.emergency_parser = true;
                }
            }
        } else {
            i /= 2;
        }
    }
}

void MarlinDevice::requestStatusUpdate() {
    if (compatibility.auto_position == 0) {
        scheduleCommand(M114_GET_CURRENT_POS, 5);
    }
    if (compatibility.auto_temp == 0) {
        scheduleCommand(M105_GET_EXTRUDER_1_TEMP, 8);
    }
    wantUpdate = true;
}

void MarlinDevice::reset() {
    buffer.clear();
    ack = 0;
    lastStatus = DeviceStatus::OK;
    lastResponse = nullptr;
    resendLine = -1;
}

void MarlinDevice::toggleRelative() {
    relative = !relative;
}

void MarlinDevice::tryParseResponse(char* _resp, size_t len) {
    char* str;
    etl::string_view resp{_resp, len};
    lastResponse = "";
    LOGF("> [%s]\n", resp);
    if (resp.starts_with(ERR_str)) {
        lastStatusStr = ERR_str;
        lastResponse = _resp + 6;
        char cpy[MAX_LINE_LEN];
        strncpy(cpy, lastResponse, MAX_LINE_LEN);
        if (char* s = strstr(cpy, "Last Line:")) {
            // next is resend number
            JOB_LOGF("d> %s", lastResponse);
            lastStatus = DeviceStatus::WAIT;
        } else {
            buffer.clear();
            lastStatus = DeviceStatus::DEV_ERROR;
        }
    } else if (resp.starts_with(ERROR_EXCLAMATION_str)) {
        lastStatusStr = ERR_str;
        lastResponse = _resp + 3;
        lastStatus = DeviceStatus::DEV_ERROR;
        buffer.clear();
    } else if (resp.starts_with(START_str) && resp.length() == strlen(START_str)) {
        //restart everything on marlin hard reset
        job.stop();
        ack = 0;
        reset();
        begin();
        return;
    } else if (resp.starts_with(OK_str)) {
        if (len > 2) {
            parseOk(_resp + 2, len - 2);
            LOG_EXTRA_INFO(lastResponse = resp + 2;)
        }
        if (lastStatus == DeviceStatus::RESEND && resendLine > -1) {
            ack = 0;
            return;
        }
        lastStatusStr = OK_str;
        ack > 0 ? ack-- : 0;
        lastStatus = DeviceStatus::OK;
    } else if (resp.find(BUSY_str) != etl::string_view::npos) {
        // "echo:busy: processing"
        lastStatusStr = BUSY_str;
        lastResponse = str + 6;
        lastStatus = DeviceStatus::BUSY;
    } else if (resp.starts_with(ECHO_str)) {
        // echo: busy must be before this
        lastStatusStr = ECHO_str;
        lastResponse = _resp + 5;// has space after ':'
        //echo:Cold extrudes are enabled (min temp 170C)
        if (resp.find("disabled", 5) != etl::string_view::npos) {
            char* minTemp = strstr(lastResponse, "min temp ");
            if (minTemp != nullptr) {
                minExtrusionTemp = strtol(minTemp + 9, nullptr, STRTOLL_BASE);
            }
        }
        if (resp.find("cold extrusion prevented") != etl::string_view::npos) {
            lastStatus = DeviceStatus::BUSY;
            return;
        }
        lastStatus = DeviceStatus::OK;
    } else if (resp.starts_with(RESEND_str)) {
        //Error:Line Number is not Last Line Number+1, Last Line: 11
        //Resend: 12
        //ok
        lastStatusStr = RESEND_str;
        // MAY have "Resend:Error
        LOG_EXTRA_INFO(lastResponse = resp + 7)
        resendLine = strtol(lastResponse, nullptr, STRTOLL_BASE);
        buffer.clear();
        lastStatus = DeviceStatus::RESEND;
    } else if (resp.starts_with(DEBUG_str)) {
        lastStatusStr = DEBUG_str;
        lastResponse = _resp + 5;
        lastStatus = DeviceStatus::OK;
    } else {
        // M154 Snn or  M155 Snn
        parseOk(_resp, len);
    }
}

void MarlinDevice::tempChange(uint8_t temp) {
    constexpr size_t LN = 11;
    char msg[LN];
    int l = snprintf(msg, LN, "%s S%d", M104_SET_EXTRUDER_TEMP, temp);
    return scheduleCommand(msg, l);
}

void MarlinDevice::bedTempChange(uint8_t temp) {
    constexpr size_t LN = 11;
    char msg[LN];
    int l = snprintf(msg, LN, "%s S%d", M140_SET_BED_TEMP, temp);
    scheduleCommand(msg, l);
}

///  marlin dont jog, just do G0
/// \return
bool MarlinDevice::canJog() {
    return lastStatus == DeviceStatus::OK;
}
/// ok T:201 /202 B:117 /120 @:255
///
///  "ok C: X:0.00 Y:0.00 Z:0.00 E:0.00"
///  " X:0.00 Y:0.00 RZ:0.00 LZ:0.00 Count X:0.00 Y:0.00 RZ:41.02 LZ:41.02"

/// \param input
/// \param len
void MarlinDevice::parseOk(const char* input, size_t len) {
    char cpy[MAX_LINE_LEN * 2];
    strncpy(cpy, input, MIN(len, MAX_LINE_LEN * 2));
    cpy[MIN(len, MAX_LINE_LEN * 2)] = 0;

    bool nextTemp = false,
        nextBedTemp = false;
    char* fromMachine = strtok(cpy, " ");

    while (fromMachine != nullptr) {
        switch (fromMachine[0]) {
            case 'T':
                hotendTemp = strtof(fromMachine + 2, nullptr);
                nextTemp = true;
                break;
            case 'B':
                if (fromMachine[1] == '@') {
                    bedPower = strtol(fromMachine + 3, nullptr, STRTOLL_BASE);
                } else {
                    bedTemp = strtol(fromMachine + 2, nullptr, STRTOLL_BASE);
                    nextBedTemp = true;
                }
                break;
            case 'X':
                x = strtof(fromMachine + 2, nullptr);
                break;
            case 'Y':
                y = strtof(fromMachine + 2, nullptr);
                break;
            case 'Z':
                z = strtof(fromMachine + 2, nullptr);
                break;
            case 'E':
                e = strtof(fromMachine + 2, nullptr);
                break;
            case '@':
                hotendPower = strtol(fromMachine + 2, nullptr, STRTOLL_BASE);
                break;
            case 'C': // next is coords
                if (fromMachine[1] == 'o') {
                    goto end;
                }
            case '/':
                if (nextTemp) {
                    hotendRequestedTemp = strtol(fromMachine + 1, nullptr, STRTOLL_BASE);
                } else if (nextBedTemp) {
                    bedRequestedTemp = strtol(fromMachine + 1, nullptr, STRTOLL_BASE);
                }
                nextTemp = false;
                nextBedTemp = false;
            default:
                break;
        }
        fromMachine = strtok(nullptr, " ");
    }
    end:;// noop
}

