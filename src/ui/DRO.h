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

    explicit DRO(GCodeDevice& d) : dev(d), cMode{Mode::AXES}, nextRefresh{1}, cDist{3}, cFeed{3}, cSpindleVal{0} {}

    virtual ~DRO() {}

    void begin() override {
        menuItems.push_back(MenuItem::simpleItem(0, "Open", [this](MenuItem&) {
            if (job.isRunning()) return;
            display->setScreen(&fileChooser); // this will reset the card
        }));
        menuItems.push_back(MenuItem::simpleItem(1, "Pause job", [this](MenuItem& m) {
            if (job.isRunning()) {
                job.setPaused(true);
                m.text = "Resume job";
            } else {
                job.setPaused(false);
                m.text = "Pause job";
            }
            doDirty();
        }));
    };

    void step() override {
        if (nextRefresh != 0 && millis() > nextRefresh) {
            nextRefresh = millis() + REFRESH_INTL + 9;
            dev.requestStatusUpdate();
        }
    }

    void enableRefresh(bool r = true) { nextRefresh = r ? millis() : 0; }

    bool isRefreshEnabled() const { return nextRefresh != 0; }

protected:
    enum class Mode : uint8_t {
        // 0    1      2
        AXES, SPINDLE, TEMP, N_VALS
    };
    constexpr static uint16_t JOG_FEEDS[] = {50, 100, 500, 1000, 2000};
    constexpr static uint8_t N_JOG_FEEDS = sizeof(JOG_FEEDS) / sizeof(JOG_FEEDS[0]);

    constexpr static uint32_t JOG_DISTS[] = {/*0.1*/ 100, /*0.5*/ 500, 1000, 5000, 10000, 50000};
    constexpr static uint8_t N_JOG_DISTS = sizeof(JOG_DISTS) / sizeof(JOG_DISTS[0]);

    GCodeDevice& dev;
    Mode cMode;

    uint32_t nextRefresh,
            cDist,
            cFeed,
            cSpindleVal;

    bool buttonWasPressedWithShift;

    uint8_t defaultAxisPrecision;

    void drawAxis(char axis, int32_t value, int x, int y);

    void drawContents() override;

    virtual void drawAxisCoords(int sx, int sy, u_int8_t lineHeight);

    void onButton(int bt, Evt arg) override;

    void onButtonAxes(int bt, Evt evt);

    void onButtonShift(int bt, Evt evt);
};
