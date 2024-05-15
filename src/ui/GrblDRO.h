#pragma once

#include "DRO.h"
#include "devices/GrblDevice.h"

// todo done 1 add override feed/spindle for running job

class GrblDRO : public DRO {
public:
    GrblDRO(GrblDevice& d) : DRO(), grblDev(d), useWCS(false) {
        defaultAxisPrecision = 3;
    }

    ~GrblDRO() {}

    void begin() override;

protected:
    GrblDevice& grblDev;

    void drawAxisCoords(int sx, int sy, u_int8_t lineHeight) const override;

private:
    bool useWCS;
};