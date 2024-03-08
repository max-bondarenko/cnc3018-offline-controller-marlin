#pragma once

#include <printfloat.h>
#include "constants.h"
#include "Screen.h"
#include "FileChooser.h"
#include "devices/GCodeDevice.h"


extern FileChooser fileChooser;
extern Job job;


class DRO : public Screen {
public:

    DRO(GCodeDevice& d) : dev(d), nextRefresh{1}, cDist{3}, cFeed{3}, cSpindleVal{0}, cMode{Mode::AXES} {
    }

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
    GCodeDevice& dev;

    uint32_t nextRefresh;
    constexpr static float JOG_DISTS[] = {0.1, 0.5, 1, 5, 10, 50};
    constexpr static size_t N_JOG_DISTS = sizeof(JOG_DISTS) / sizeof(JOG_DISTS[0]);
    uint32_t cDist;
    constexpr static int JOG_FEEDS[] = {50, 100, 500, 1000, 2000};
    constexpr static size_t N_JOG_FEEDS = sizeof(JOG_FEEDS) / sizeof(JOG_FEEDS[0]);
    uint32_t cFeed;
    uint32_t cSpindleVal = 0;

    Mode cMode;
    bool buttonWasPressedWithShift;

    void drawAxis(char axis, float value, int x, int y);

    void drawContents() override;

    virtual void drawAxisCoords(int sx, int sy, u_int8_t lineHeight);

    void onButton(int bt, Evt arg) override;

    void onButtonAxes(int bt, Evt evt);

    void onButtonShift(int bt, Evt evt);
};
