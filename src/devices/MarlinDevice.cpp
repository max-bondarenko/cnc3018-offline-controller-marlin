#include <vector>
#include <Arduino.h>
#include <functional>
#include "constants.h"
#include "MarlinDevice.h"

#include "Job.h"
#include "util.h"
#include "debug.h"

#define LOG_EXTRA_INFO(a) //noop
#ifdef LOG_DEBUG
#define LOG_EXTRA_INFO(a) a;
#endif

void MarlinDevice::sendProbe(Stream& serial) {
    serial.print("\n");
    serial.print(M115_GET_FIRMWARE_VER);
    serial.print("\n");
}

/// FIRMWARE_NAME:Marlin
///   In Marlin first 3 numbers is the position for the planner.
///   The other positions are the positions from the stepper function.
///   This helps for debugging a previous stepper function bug.
/// Compatibility example
/* Cap:VOLUMETRIC:1
Cap:AUTOREPORT_POS:1
Cap:AUTOREPORT_TEMP:1
Cap:PROGRESS:0
Cap:PRINT_JOB:1
Cap:AUTOLEVEL:0
Cap:RUNOUT:0
Cap:Z_PROBE:0
Cap:LEVELING_DATA:0
Cap:BUILD_PERCENT:0
Cap:SOFTWARE_POWER:0
Cap:TOGGLE_LIGHTS:0
Cap:CASE_LIGHT_BRIGHTNESS:0
Cap:EMERGENCY_PARSER:0
Cap:HOST_ACTION_COMMANDS:0
Cap:PROMPT_SUPPORT:0
Cap:SDCARD:0
Cap:REPEAT:0
Cap:SD_WRITE:0
Cap:AUTOREPORT_SD_STATUS:0
Cap:LONG_FILENAME:0
Cap:LFN_WRITE:0
Cap:CUSTOM_FIRMWARE_UPLOAD:0
Cap:EXTENDED_M20:0
Cap:THERMAL_PROTECTION:0
Cap:MOTION_MODES:0
Cap:ARCS:1
Cap:BABYSTEPPING:0
Cap:CHAMBER_TEMPERATURE:0
Cap:COOLER_TEMPERATURE:0
Cap:MEATPACK:0
Cap:CONFIG_EXPORT:0
 */
bool MarlinDevice::checkProbeResponse(const String& input) {
    if (input.indexOf("Marlin") != -1) {
        LOGLN(">> Detected Marlin device <<");
        return true;
    }
    return false;
}

MarlinDevice::MarlinDevice(WatchedSerial& _printerSerial, Job& _job) : GCodeDevice(_printerSerial, _job) {
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
    if (lastStatus == DeviceStatus::WAIT) {
        return;
    }
    if (printerSerial.availableForWrite() && !outQueue.empty()) {
        String& front = outQueue.front();
        const char* cmd = front.c_str();
        auto size = (size_t) front.length();
        LOGF("[%s]\n", cmd);
        printerSerial.write((const unsigned char*) cmd, size);
        printerSerial.write('\n');
        // clean
        lastResponse = nullptr;
        lastStatus = DeviceStatus::WAIT;
    }
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
        for (int i = 50; i > 0 && s.readBytesUntil('\n', buffer, 100) != -1; i--) {
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
    lastStatus = DeviceStatus::OK;
}

void MarlinDevice::toggleRelative() {
    relative = !relative;
}

void MarlinDevice::tryParseResponse(char* resp, size_t len) {
    char* lr = nullptr;
    bool need_pop = true;
    char* str;
    lastResponse = "";
    LOGF("> [%s]\n", resp);
    if (startsWith(resp, ERR_str)) {
        lr = resp;
        lr[5] = 0;
        lastResponse = resp + 6;
        parseError(lastResponse);
        lastStatus = DeviceStatus::DEV_ERROR;
        outQueue.clear();
        need_pop = false;
    } else if (startsWith(resp, ErrorExclamation_str)) {
        lr = resp;
        lr[2] = 0;
        lastResponse = resp + 3;
        lastStatus = DeviceStatus::DEV_ERROR;
        outQueue.clear();
        need_pop = false;
    } else if (startsWith(resp, OK_str)) {
        if (len > 2) {
            parseOk(resp + 2, len - 2);
            lr = const_cast<char* >(OK_str);
            LOG_EXTRA_INFO(lastResponse = resp + 2;)
        }
        if (resendLine > 0) { // was resend before
            resendLine = -1;
        }
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
        lastStatus = DeviceStatus::OK;
    } else if (startsWith(resp, RESEND_str)) {
        lr = resp;
        lr[6] = 0;
        // MAY have "Resend:Error
        LOG_EXTRA_INFO(lastResponse = resp + 7)
        resendLine = strtol(lastResponse, nullptr, STRTOLL_BASE);
        lastStatus = DeviceStatus::RESEND;
        // no pop. resend
    } else if (startsWith(resp, DEBUG_str)) {
        lr = resp;
        lr[5] = 0;
        lastResponse = resp + 5;
        lastStatus = DeviceStatus::OK;
    } else {
        // M154 Snn or  M155 Snn
        parseOk(resp, len);
        if (lastStatus == DeviceStatus::BUSY) {
            lr = const_cast<char* >(BUSY_str);
        } else {
            lastStatus = DeviceStatus::OK;
            lr = const_cast<char* >(OK_str);
        }
        need_pop = false;
    }
    if (need_pop && !outQueue.empty())
        outQueue.pop_front();
    if (lr != nullptr)
        lastStatusStr = lr;
}

bool MarlinDevice::tempChange(uint8_t temp) {
    constexpr size_t LN = 11;
    char msg[LN];
    int l = snprintf(msg, LN, "%s S%d", M104_SET_EXTRUDER_TEMP, temp);
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
                    nextBedTemp = true; // TODO check may be bug !!!
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

void MarlinDevice::parseError(const char* input) {
    char cpy[SHORT_BUFFER_LEN];
    strncpy(cpy, input, SHORT_BUFFER_LEN);
    if (strstr(cpy, "Last Line") != nullptr) {
//        int lastResponse = atoi((cpy + 10)); TODO
    }
}
