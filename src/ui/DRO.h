#pragma once

#include "utils.h"
#include "constants.h"
#include "Screen.h"
#include "FileChooser.h"
#include "devices/GCodeDevice.h"


extern FileChooser fileChooser;
extern Job job;


class DRO : public Screen {
public:

    explicit DRO(GCodeDevice& d) : dev(d), cMode{Mode::AXES}, nextRefresh{1}, cDist{0}, cFeed{0}, cSpindleVal{0} {}

    virtual ~DRO() {}

    void begin() override;

    void step() override;

    void enableRefresh(bool r = true) { nextRefresh = r ? millis() : 0; }

    bool isRefreshEnabled() const { return nextRefresh != 0; }

protected:
    enum class Mode : uint8_t {
        // 0    1      2
        AXES, SPINDLE, TEMP, N_VALS
    };

    GCodeDevice& dev;
    Mode cMode;

    uint32_t nextRefresh;

    uint32_t cDist,
            cFeed,
            cSpindleVal = 0;

    bool buttonWasPressedWithShift;

    uint8_t defaultAxisPrecision ;

    void drawAxis(char axis, float value, int x, int y);

    void drawContents() override;

    virtual void drawAxisCoords(int sx, int sy, u_int8_t lineHeight);

    void onButton(int bt, Evt arg) override;

    void onButtonAxes(int bt, Evt evt);

    void onButtonShift(int bt, Evt evt);
};
