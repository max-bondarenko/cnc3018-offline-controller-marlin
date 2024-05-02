#include "etl/deque.h"
#include "etl/vector.h"
#include <Arduino.h>
#include <functional>

#include "constants.h"
#include "MarlinDevice.h"
#include "Job.h"
#include "util.h"
#include "debug.h"
#include "etl/string_view.h"

#define LOG_EXTRA_INFO(a) //noop
#ifdef LOG_DEBUG
#define LOG_EXTRA_INFO(a) a;
#endif

extern WatchedSerial serialCNC;

void MarlinDevice::sendProbe(Stream& serial) {
    serial.print("\n");
    serial.print(M115_GET_FIRMWARE_VER);
    serial.print("\n");
}

/// FIRMWARE_NAME:Marlin
bool MarlinDevice::checkProbeResponse(const char* const input) {
    etl::string_view i{input};
    return i.find("Marlin") != etl::string_view::npos;
}

MarlinDevice::MarlinDevice() : GCodeDevice() {
    canTimeout = false;
    useLineNumber = true;
    config.spindle = etl::vector<u_int16_t, 10>{0, 1, 64, 255};
    compatibility.auto_temp = 0;
    compatibility.auto_position = 0;
    compatibility.emergency_parser = 0;
}

bool MarlinDevice::jog(uint8_t axis, float dist, uint16_t feed) {
    constexpr size_t LN = 25;
    char msg[LN];
    // adding G91_SET_RELATIVE_COORDINATES to command does not work.
    int l = snprintf(msg, LN, "G0 F%d %c %.3f", feed, AXIS[axis], dist);
    return scheduleCommand(msg, l);
}

void MarlinDevice::trySendCommand() {
    if (lastStatus >= DeviceStatus::WAIT) {
        return;
    }
    while (ack < outQueue.max_size() && serialCNC.availableForWrite() && !outQueue.empty()) {
        String& front = outQueue.front();
        const char* cmd = front.c_str();
        auto size = (size_t) front.length();
        IO_LOGF("> [%s]\n", cmd);
        serialCNC.write((const unsigned char*) cmd, size);
        serialCNC.write('\n');
        outQueue.pop_front();
        ack++;
    }
    lastResponse = nullptr;
}

bool MarlinDevice::scheduleCommand(const char* cmd, size_t len) {
    if (!outQueue.full()) {
        outQueue.push_back(String(cmd));
        return true;
    }
    return false;
}

bool MarlinDevice::schedulePriorityCommand(const char* cmd, size_t len) {
    return scheduleCommand(cmd, len);
}

void MarlinDevice::begin(SetupFN* const onBegin) {
    SetupFN fn = [this](WatchedSerial& s) {
        char buffer[101]; // assume not longer then 100B
        buffer[100] = 0;
        // do not check readBytesUntil. return value!!, let it read
        for (int i = 50; i > 0 && s.readBytesUntil('\n', buffer, 100) != -1; i--) {
            IO_LOGLN(buffer);
            if (buffer[0] == 'C' && buffer[1] == 'a' && buffer[2] == 'p') {
                buffer[0] = 0;
                String _s(buffer + 4);
                // OPTIMIZATION
                if (_s.indexOf("AUTOREPORT_") != -1) {
                    if (_s.indexOf("TEMP", 11) != -1 && _s.indexOf(":1", 11) != -1) {
                        this->compatibility.auto_temp = 1;
                    } else if (_s.indexOf("POS", 11) != -1 && _s.indexOf(":1", 11) != -1) {
                        this->compatibility.auto_position = 1;
                    }
                } else if (_s.indexOf("EMERGENCY_PARSER") != -1 && _s.indexOf(":1", 16) != -1) {
                    this->compatibility.emergency_parser = 1;
                }
            } else {
                i /= 2;
            }
        }
    };
    GCodeDevice::begin(&fn);
    constexpr size_t LN = 8;
    char msg[LN];
    // OPTIMIZATION
    msg[4] = ' ';
    msg[5] = 'S';
    msg[6] = '1';
    msg[7] = 0;
    if (compatibility.auto_position > 0) {
        memcpy(msg, M154_AUTO_REPORT_POSITION, 4);
        scheduleCommand(msg, 9);
    }
    if (compatibility.auto_temp > 0) {
        memcpy(msg, M155_AUTO_REPORT_TEMP, 4);
        scheduleCommand(msg, 9);
    }
    scheduleCommand(RESET_LINE_NUMBER, 8);
    scheduleCommand(M302_COLD_EXTRUDER_STATUS, 5);
}

void MarlinDevice::requestStatusUpdate() {
    if (compatibility.auto_position == 0) {
        scheduleCommand(M114_GET_CURRENT_POS, 5);
    }
    if (compatibility.auto_temp == 0) {
        scheduleCommand(M105_GET_EXTRUDER_1_TEMP, 8);
    }
}

void MarlinDevice::reset() {
    outQueue.clear();
    lastStatus = DeviceStatus::OK;
    resendLine = -1;
    lastResponse = nullptr;
    // will not read compatibilities. reset should not change it
    begin(nullptr);
}

void MarlinDevice::toggleRelative() {
    relative = !relative;
}

void MarlinDevice::tryParseResponse(char* resp, size_t len) {
    char* lr = nullptr;
    char* str;
    lastResponse = "";
    LOGF("> [%s]\n", resp);
    if (startsWith(resp, ERR_str)) {
        lr = resp;
        lr[5] = 0;
        lastResponse = resp + 6;
        char cpy[SHORT_BUFFER_LEN];
        strncpy(cpy, lastResponse, SHORT_BUFFER_LEN);
        if (char* s = strstr(cpy, "Last Line:")) {
            // next is resend number
            JOB_LOGF("d> %s", lastResponse);
            lastStatus = DeviceStatus::WAIT;
        } else {
            outQueue.clear();
            lastStatus = DeviceStatus::DEV_ERROR;
        }
    } else if (startsWith(resp, ERROR_EXCLAMATION_str)) {
        lr = resp;
        lr[2] = 0;
        lastResponse = resp + 3;
        lastStatus = DeviceStatus::DEV_ERROR;
        outQueue.clear();
    } else if (startsWith(resp, START_str) &&
               len == ((sizeof(START_str)) / (sizeof(START_str[0]))) + 1) { // START_str.len + 1
        //restart everything on marlin hard reset
        job.stop();
        ack = 0;
        reset();
        return;
    } else if (startsWith(resp, OK_str)) {
        if (len > 2) {
            parseOk(resp + 2, len - 2);
            LOG_EXTRA_INFO(lastResponse = resp + 2;)
        }
        if (lastStatus == DeviceStatus::RESEND && resendLine > -1) {
            ack = 0;
            return;
        }
        lr = const_cast<char* >(OK_str);
        ack--;
        lastStatus = DeviceStatus::OK;
    } else if ((str = strstr(resp, BUSY_str)) != nullptr) {
        // "echo:busy: processing"
        lr = const_cast<char* >(BUSY_str);
        lastResponse = str + 6;
        lastStatus = DeviceStatus::BUSY;
    } else if (startsWith(resp, ECHO_str)) {
        // echo: busy must be before this
        lr = resp;
        lr[4] = 0;
        lastResponse = resp + 5;// has space after ':'
        //echo:Cold extrudes are enabled (min temp 170C)
        if (!outQueue.empty() && outQueue.front().indexOf(M302_COLD_EXTRUDER_STATUS) != -1) {
            if (strstr(lastResponse, "disabled") != nullptr) {
                char* string = strstr(lastResponse, "min temp ");
                if (string != nullptr) {
                    minExtrusionTemp = strtol(string + 9, nullptr, STRTOLL_BASE);
                }
            }
        }
        if (strstr(lastResponse, "cold extrusion prevented") != nullptr) {
            lastStatus = DeviceStatus::BUSY;
            return;
        }
        lastStatus = DeviceStatus::OK;
        //> [N20 G1 X13.259 Y1.557 E4.08391*109]
        //Error:Line Number is not Last Line Number+1, Last Line: 11
        //Resend: 12
        //ok
    } else if (startsWith(resp, RESEND_str)) {
        lr = resp;
        lr[6] = 0;
        // MAY have "Resend:Error
        LOG_EXTRA_INFO(lastResponse = resp + 7)
        lastResponse = resp + 7;
        resendLine = strtol(lastResponse, nullptr, STRTOLL_BASE);
        outQueue.clear();
        lastStatus = DeviceStatus::RESEND;
    } else if (startsWith(resp, DEBUG_str)) {
        lr = resp;
        lr[5] = 0;
        lastResponse = resp + 5;
        lastStatus = DeviceStatus::OK;
    } else {
        // M154 Snn or  M155 Snn
        parseOk(resp, len);
    }
    if (lr != nullptr)
        lastStatusStr = lr;
}

bool MarlinDevice::tempChange(uint8_t temp) {
    constexpr size_t LN = 11;
    char msg[LN];
    int l = snprintf(msg, LN, "%s S%d", M104_SET_EXTRUDER_TEMP, temp);
    return scheduleCommand(msg, l);
}

bool MarlinDevice::bedTempChange(uint8_t temp) {
    constexpr size_t LN = 11;
    char msg[LN];
    int l = snprintf(msg, LN, "%s S%d", M140_SET_BED_TEMP, temp);
    return scheduleCommand(msg, l);
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
    char cpy[BUFFER_LEN];
    strncpy(cpy, input, MIN(len, BUFFER_LEN));
    cpy[MIN(len, BUFFER_LEN)] = 0;

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

