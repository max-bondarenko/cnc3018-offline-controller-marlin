#pragma once

#include "utils.h"
#include "constants.h"
#include "Screen.h"
#include "HasMenu.h"
#include "FileChooser.h"
#include "devices/GCodeDevice.h"

extern FileChooser fileChooser;
extern Job job;

// TODO list
// TODO 1 dynamic allocation for menu
// TODO 2 and simplify it container
// TODO 3 check for +- menu items. to avoid every menu check for i = 0

class DRO : public HasMenu, public Screen, public JobObserver {
public:

    explicit DRO() : cMode{Mode::AXES}, nextRefresh{1}, cDist{0}, cFeed{0}, cSpindleVal{0} {
        Screen::hasMenu = true;
    }

    virtual ~DRO() {}

    void begin() override;

    void step() override;

    void enableRefresh(bool enable = true) { nextRefresh = (true == enable) ? millis() : 0; }

    bool isRefreshEnabled() const { return nextRefresh != 0; }

    void notification(JobEvent e) override;

protected:
    using Evt = Display::ButtonEvent;
    enum class Mode : uint8_t {
        // 0    1      2
        AXES, SPINDLE, TEMP, N_VALS
    };

    Mode cMode;

    uint32_t nextRefresh;

    uint32_t cDist,
        cFeed,
        cSpindleVal = 0;

    bool buttonWasPressedWithShift;

    uint8_t defaultAxisPrecision;

    void drawAxis(char axis, float value, int x, int y) const;

    void drawContents() override;

    virtual void drawAxisCoords(int sx, int sy, u_int8_t lineHeight) const;

    void onButton(int bt, Evt arg) override;

    void onButtonAxes(int bt, Evt evt);

    void onButtonShift(int bt, Evt evt);
};
