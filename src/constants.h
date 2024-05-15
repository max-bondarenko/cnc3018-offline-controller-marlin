#ifndef CNC_3018_CONSTANTS_H
#define CNC_3018_CONSTANTS_H

#include <Arduino.h>

// ====================== SD ======================
constexpr uint32_t PIN_CD = PA3;

// ===================== LCD =====================
constexpr uint32_t PIN_LCD_CS = PA2; // not connected
constexpr uint32_t PIN_LCD_RST = PB0;
constexpr uint32_t PIN_LCD_DC = PB1;
constexpr uint32_t PIN_LCD_CLK = PA0;
constexpr uint32_t PIN_LCD_MOSI = PA1;
// ===================== BUTTONS ==================
constexpr int N_BUTT = 8;
constexpr uint32_t PIN_BT_ZDOWN = PB8;
constexpr uint32_t PIN_BT_ZUP = PB9;
constexpr uint32_t PIN_BT_R = PB10;
constexpr uint32_t PIN_BT_L = PB11;
constexpr uint32_t PIN_BT_CENTER = PB12;
constexpr uint32_t PIN_BT_UP = PB13;
constexpr uint32_t PIN_BT_DOWN = PB14;
constexpr uint32_t PIN_BT_STEP = PB15;

constexpr uint32_t PIN_DET = PC13;    ///< 0V=no USB on CNC, 1=CNC connected to USB.

constexpr int STRTOLL_BASE = 10;

constexpr char AXIS[] = {'X', 'Y', 'Z', 'E'};
constexpr char AXIS_WCS[] = {'x', 'y', 'z'};
constexpr char PRINTER[] = {'T', 'B'};

enum DeviceName {
    GRBL = 0,
    MARLIN,
    N_DEVICES
};
static const char* const DEVICE_NAMES[] = {"grbl", "marlin"};


//================ PROTOCOL ==============
constexpr char XON = 0x11;
constexpr char XOFF = 0x13;

constexpr uint32_t buttPins[N_BUTT] = {
    PIN_BT_ZDOWN,
    PIN_BT_ZUP,

    PIN_BT_R,
    PIN_BT_L,
    PIN_BT_CENTER,
    PIN_BT_UP,
    PIN_BT_DOWN,

    PIN_BT_STEP
};

constexpr size_t PROBE_INTERVAL = 303;
constexpr size_t REFRESH_INTL = 400;
constexpr size_t BUTTON_INTL = 19;
constexpr size_t KEEPALIVE_INTERVAL = 5000;    // Marlin defaults to 2 seconds, get a little of margin
constexpr uint8_t JOB_BUFFER_SIZE = 10;
constexpr uint8_t MAX_LINE_LEN = 100;

constexpr uint8_t MAX_ADJUST_PERCENT = 250;
constexpr uint8_t MIN_ADJUST_PERCENT = 50;
constexpr uint8_t ADJUST_PERCENT_STEP = 5;
constexpr uint8_t GRBL_MAX_ADJUST_PERCENT = 200;
constexpr uint8_t GRBL_MIX_ADJUST_PERCENT = 10;
constexpr uint8_t GRBL_ADJUST_PERCENT_STEP = 10;
constexpr uint8_t GRBL_RAPID = 25;


#define PRESETS_FILE "presets.ini"


static const char* const OK_STR = "ok";
static const char* const START_STR = "start";
static const char* const ERR_STR = "Err";
static const char* const ERROR_EXCLAMATION_STR = "!!";
static const char* const BUSY_STR = "busy";
static const char* const ECHO_STR = "echo";
static const char* const RESEND_STR = "Resend";
static const char* const DEBUG_STR = "DEBUG";
static const char* const ALARM_STR = "ALARM";
static const char* const ERROR_STR = "error";
static const char* const MSG_STR = "MSG";
static const char* const Alarm_STR = "Alarm";
static const char* const CHECK_STR = "Check";
static const char* const DOOR_STR = "Door";
static const char* const HOLD_STR = "Hold";
static const char* const HOME_STR = "Home";
static const char* const IDLE_STR = "Idle";
static const char* const JOG_STR = "Jog";
static const char* const RUN_STR = "Run";
static const char* const SLEEP_STR = "Sleep";

#endif //CNC_3018_CONSTANTS_H
