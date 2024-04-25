#pragma once

#include "DRO.h"
#include "devices/GrblDevice.h"

// todo list
// todo 1 add override feed/spindle for running job

class GrblDRO : public DRO {
public:
    GrblDRO(GrblDevice& d) : DRO(d), dev(d), useWCS(false) {
        defaultAxisPrecision = 3;
    }

    ~GrblDRO() {}

    void begin() override;

protected:
    GrblDevice& dev;

    void drawAxisCoords(int sx, int sy, u_int8_t lineHeight) const override;

private:
    bool useWCS;
};