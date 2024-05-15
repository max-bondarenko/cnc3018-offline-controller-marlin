#include "GrblDRO.h"
#include "FileChooser.h"

extern GCodeDevice* dev;

void GrblDRO::begin() {
    menuItems.size = 2 + 9;
    menuItems.items = new MenuItem* [menuItems.size]; // not going to be deleted
    DRO::begin();
    uint8_t indx = 2U;
    menuItems.items[indx++] = MenuItem::simpleItem("Unlock", [](MenuItem&, int8_t) {
        dev->schedulePriorityCommand("$X", 3);
    });
    menuItems.items[indx++] = MenuItem::simpleItem("Reset", [this](MenuItem&, int8_t) {
        grblDev.reset();
    });
    menuItems.items[indx++] = MenuItem::simpleItem("Home", [](MenuItem&, int8_t) {
        dev->scheduleCommand("$H", 3);
    });
    menuItems.items[indx++] = MenuItem::simpleItem("XYZ=0", [](MenuItem&, int8_t) {
        // <G10 L2 P.. X.. Y.. Z..> L2 tells the G10 we’re setting standard work offsets
        //      P0 = Active coordinate system
        //      P1..6  = G54..59
        //      If it’s G90 (absolute)
        //      "G10 L2 P1 X10 Y20 Z0" Will set G54 to X10, Y20, and Z0.
        //        G91 (relative), then XYZ offset the current work offset’s
        //  <G10 L20 P.. X.. Y.. Z..> P can be used to select G54.1 P1..G54.1 P48

        dev->scheduleCommand("G10 L20 P1 X0Y0Z0", 18);
    });
    menuItems.items[indx++] = MenuItem::simpleItem("Goto XY=0", [](MenuItem&, int8_t) {
        dev->scheduleCommand("G0 X0Y0", 8);
    });
    menuItems.items[indx++] = MenuItem::simpleItem("Machine/Work", [this](MenuItem&, int8_t) {
        useWCS = !useWCS;
        grblDev.scheduleCommand(useWCS ? G54_USE_COORD_SYSTEM_1 : G53_USE_MACHINE_COORD, 4); // TODO need to check
    });
    menuItems.items[indx++] = MenuItem::valueItem("Spdl100<100%>", [](MenuItem& m, int8_t dir) {
        constexpr uint8_t LABEL_LEN = 14;
        static char buf[LABEL_LEN];
        // NB this is coupled with real device state by dev.spindlerate!!
        if (dir == 0) {
            dev->schedulePriorityCommand(GRBL_SPINDLE_100, 1);
            dev->spindlerate = 100;
        } else if (dir > 0) {
            dev->spindlerate += GRBL_ADJUST_PERCENT_STEP;
            if (dev->spindlerate >= GRBL_MAX_ADJUST_PERCENT)
                dev->spindlerate = GRBL_MAX_ADJUST_PERCENT;
            dev->schedulePriorityCommand(GRBL_SPINDLE_PLUS10, 1);
        } else {
            dev->spindlerate -= GRBL_ADJUST_PERCENT_STEP;
            if (dev->spindlerate <= GRBL_MIX_ADJUST_PERCENT)
                dev->spindlerate = GRBL_MIX_ADJUST_PERCENT;
            dev->schedulePriorityCommand(GRBL_SPINDLE_MINUS10, 1);
        }
        int l = snprintf(buf, LABEL_LEN, "Spdl100<%d%%>", dev->spindlerate);
        buf[l] = 0;
        m.text = buf;
    });
    menuItems.items[indx++] = MenuItem::valueItem("Feed100<100%>", [](MenuItem& m, int8_t dir) {
        constexpr uint8_t LABEL_LEN = 14;
        static char buf[LABEL_LEN];
        // NB this is coupled with real device state by dev.feedrate!!
        if (dir == 0) {
            dev->schedulePriorityCommand(GRBL_FEED_100, 1);
            dev->feedrate = 100;
        } else if (dir > 0) {
            dev->feedrate += GRBL_ADJUST_PERCENT_STEP;
            if (dev->feedrate >= GRBL_MAX_ADJUST_PERCENT)
                dev->feedrate = GRBL_MAX_ADJUST_PERCENT;
            dev->schedulePriorityCommand(GRBL_FEED_PLUS10, 1);
        } else {
            dev->feedrate -= GRBL_ADJUST_PERCENT_STEP;
            if (dev->feedrate <= GRBL_MIX_ADJUST_PERCENT)
                dev->feedrate = GRBL_MIX_ADJUST_PERCENT;
            dev->schedulePriorityCommand(GRBL_FEED_MINUS10, 1);
        }
        int l = snprintf(buf, LABEL_LEN, "Feed100<%d%%>", dev->feedrate);
        buf[l] = 0;
        m.text = buf;
    });
    menuItems.items[indx++] = MenuItem::valueItem("Rapd100<100%=", [](MenuItem& m, int8_t dir) {
        constexpr uint8_t LABEL_LEN = 14;
        static char buf[LABEL_LEN];
        // NB this is coupled with real device state by dev.rapidrate!!
        // Grbl does not support increase rapids.
        if (dir == 0) {
            dev->schedulePriorityCommand(GRBL_RAPID_100, 1);
            dev->rapidrate = 100;
        } else if (dir < 0) {
            dev->rapidrate -= GRBL_RAPID;
            if (dev->rapidrate <= GRBL_RAPID)
                dev->rapidrate = GRBL_RAPID;
            dev->schedulePriorityCommand(GRBL_RAPID_MINUS25, 1);
        }
        int l = snprintf(buf, LABEL_LEN, "Rapd100<%d%%=", dev->rapidrate);
        buf[l] = 0;
        m.text = buf;
    });
}

void GrblDRO::drawAxisCoords(int sx, int sy, u_int8_t lineHeight) const {
    float t[3] = {0, 0, 0};
    char ax[3];

    if (useWCS) {
        memcpy(ax, AXIS_WCS, 3);
        t[0] = grblDev.getXOfs();
        t[1] = grblDev.getYOfs();
        t[2] = grblDev.getZOfs();
    } else {
        memcpy(ax, AXIS, 3);
    }
    drawAxis(ax[0], grblDev.getX() - t[0], sx, sy);
    drawAxis(ax[1], grblDev.getY() - t[1], sx, sy + 11);
    drawAxis(ax[2], grblDev.getZ() - t[2], sx, sy + 11 * 2);
}
