#include "GrblDRO.h"
#include "FileChooser.h"

void GrblDRO::begin() {
    DRO::begin();
    uint8_t indx = 2U;
    menuItems.push_back(MenuItem::simpleItem(indx++, "Unlock", [this](MenuItem&, int8_t dir) {
        if (dir == 0)
            dev.schedulePriorityCommand("$X", 3);
    }));
    menuItems.push_back(MenuItem::simpleItem(indx++, "Reset", [this](MenuItem&, int8_t dir) {
        if (dir == 0)
            dev.reset();
    }));
    menuItems.push_back(MenuItem::simpleItem(indx++, "Home", [this](MenuItem&, int8_t dir) {
        if (dir == 0)
            dev.schedulePriorityCommand("$H", 3);
    }));
    menuItems.push_back(MenuItem::simpleItem(indx++, "XYZ=0", [this](MenuItem&, int8_t dir) {
        // <G10 L2 P.. X.. Y.. Z..> L2 tells the G10 we’re setting standard work offsets
        //      P0 = Active coordinate system
        //      P1..6  = G54..59
        //      If it’s G90 (absolute)
        //      "G10 L2 P1 X10 Y20 Z0" Will set G54 to X10, Y20, and Z0.
        //        G91 (relative), then XYZ offset the current work offset’s
        //  <G10 L20 P.. X.. Y.. Z..> P can be used to select G54.1 P1..G54.1 P48
        if (dir == 0)
            dev.scheduleCommand("G10 L20 P1 X0Y0Z0", 18);
    }));
    menuItems.push_back(MenuItem::simpleItem(indx++, "Goto XY=0", [this](MenuItem&, int8_t dir) {
        if (dir == 0)
            dev.scheduleCommand("G0 X0Y0", 8);
    }));
    menuItems.push_back(MenuItem::simpleItem(indx++, "Machine/Work", [this](MenuItem&, int8_t dir) {
        useWCS = !useWCS;
        // todo ???
//        dev.scheduleCommand(useWCS ? G54_USE_COORD_SYSTEM_1 : G53_USE_MACHINE_COORD, 4);
    }));
}

void GrblDRO::drawAxisCoords(int sx, int sy, u_int8_t lineHeight) const {
    float t[3] = {0, 0, 0};
    char ax[3];

    if (useWCS) {
        memcpy(ax, AXIS_WCS, 3);
        t[0] = dev.getXOfs();
        t[1] = dev.getYOfs();
        t[2] = dev.getZOfs();
    } else {
        memcpy(ax, AXIS, 3);
    }
    drawAxis(ax[0], dev.getX() - t[0], sx, sy);
    drawAxis(ax[1], dev.getY() - t[1], sx, sy + 11);
    drawAxis(ax[2], dev.getZ() - t[2], sx, sy + 11 * 2);
}