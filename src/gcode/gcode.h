#pragma once

#ifndef GCODE__
#define GCODE__

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
#define M114_GET_CURRENT_POS_REALTIME    "M114 R"

#define M154_AUTO_REPORT_POSITION        "M154"
#define M155_AUTO_REPORT_TEMP            "M155"
#endif // GCODE__