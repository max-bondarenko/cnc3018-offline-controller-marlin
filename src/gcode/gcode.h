#pragma once

#ifndef GCODE__
#define GCODE__

#define GRBL_INFO                        "$I"
#define GRBL_RESET                       "\x18"
#define GRBL_STATUS                      "?"
/*
If override value does not change, the command is ignored.
Feed override value can not be 10% or greater than 200%.

Does not alter rapid rates, which include G0, G28, and G30, or jog motions.
 */
#define GRBL_FEED_100                    "\x90"
#define GRBL_FEED_PLUS10                 "\x91"
#define GRBL_FEED_MINUS10                "\x92"
//Only effects rapid motions, which include G0, G28, and G30.
#define GRBL_RAPID_100                   "\x95"
#define GRBL_RAPID_MINUS50               "\x96"
#define GRBL_RAPID_MINUS25               "\x97"
//Spindle override value can not be 10% or greater than 200%
#define GRBL_SPINDLE_100                 "\x99"
#define GRBL_SPINDLE_PLUS10              "\x9A"
#define GRBL_SPINDLE_MINUS10             "\x9B"


#define G28_START_HOMING                 "G28"
#define G53_USE_MACHINE_COORD            "G53"
#define G54_USE_COORD_SYSTEM_1           "G54"
#define G90_SET_ABS_COORDINATES          "G90"
#define G91_SET_RELATIVE_COORDINATES     "G91"
#define M0_STOP_UNCONDITIONAL_FOR_60SEC  "M0 S60"

#define M104_SET_EXTRUDER_TEMP           "M104"
#define M105_GET_EXTRUDER_1_TEMP         "M105 T1"
#define M108_CONTINUE                    "M108"
#define M109_SET_EXTRUDER_TEMP_WAIT      "M109"
#define M110_SET_LINE_NUMBER             "M110"
#define M140_SET_BED_TEMP                "M140"
#define M302_COLD_EXTRUDER_STATUS        "M302"
#define RESET_LINE_NUMBER                "M110 N0"
#define M115_GET_FIRMWARE_VER            "M115"
#define M220_FEEDRATE_ADJUST             "M220"
#define M221_FLOW_ADJUST                 "M221"
/*
Get the “current position” of the active tool. Stepper values are included. If M114_LEGACY is enabled the planner will
be synchronized before reporting so that the reported position is not be ahead of the actual planner position. Normally
M114 reports the “projected position” which is the last position Marlin was instructed to move to. With the
M114_REALTIME option you can send R to get the “real” current position at the moment that the request was processed.
This position comes directly from the steppers in the midst of motion, so when the printer is moving you can consider
this the “recent position.” For debugging it can be useful to enable M114_DETAIL which adds D and E parameters to
get extra details.
*/
#define M114_GET_CURRENT_POS             "M114"
// need to enable M114_REALTIME in marlin ConfigAdvanced.h
// this UI is not so fast to show realtime.
#define M114_GET_CURRENT_POS_REALTIME    "M114 R"

#define M154_AUTO_REPORT_POSITION        "M154"
#define M155_AUTO_REPORT_TEMP            "M155"
#endif // GCODE__