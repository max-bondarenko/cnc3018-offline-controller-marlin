#pragma once

#include <Arduino.h>
#include <functional>
#include "U8g2lib.h"
#include "etl/vector.h"

#include "constants.h"
#include "devices/GCodeDevice.h"
#include "Job.h"

class DRO;

class Screen;

/// Display abstraction
/// N.B. Display drives device updates (call request status)
/// Device drivers only tries to get and parse response. It is because marlin can
/// work in auto status mode M154/M155.
class Display : public JobObserver, public DeviceObserver {
public:
    static constexpr uint8_t STATUS_BAR_HEIGHT = 16;
    static constexpr uint8_t LINE_HEIGHT = 11;
    static constexpr int HOLD_COUNT = 11; // x19 = ms
    enum {
        BT_ZDOWN = 0,  //
        BT_ZUP,        //
        //                    +--------------------------------------------------------+
        BT_R,          //     |  [Z_UP] +--------------+          [BT_UP]              |
        BT_L,          //     |         |              |                               |
        BT_CENTER,     //     |         |              |  [BT_L] [BT_CENTER]  [BT_R]   |
        BT_UP,         //     |         |              |                               |
        BT_DOWN,       //     | [Z_DOWN]+--------------+          [BT_DOWN]  [BT_STEP] |
        BT_STEP,       //     +--------------------------------------------------------+
        //
        N_BUTTONS
    };
    static U8G2& u8g2;
    static uint16_t buttStates;

    enum class ButtonEvent {
        UP, DOWN, HOLD
    };

    Display();

    ~Display() final {}

    void doDirty() { dirty = true; }

    void notification(const DeviceStatusEvent& e) override {
        devStatus = e.status;
        devStatusString = e.statusStr;
        devLastResponse = e.lastResponse;
        doDirty();
    }

    void notification(const JobStatusEvent e) override {
        doDirty();
    }

    void begin();

    ///
    /// call in each main loop step
    ///
    void step();

    void draw();

    void setScreen(Screen* _screen);

    void processInput();

private:
    Screen* screen;
    bool dirty;
    uint8_t selMenuItem = 0;
    bool menuShown;
    bool menuShownWhenDown;
    String devStatusString;
    String devLastResponse;
    size_t devStatus;

    decltype(buttStates) prevStates;

    int16_t holdCounter[N_BUTTONS];

    void processMenuButton(uint8_t bt, ButtonEvent evt);

    void drawStatusBar();

    void drawMenu();

    void ensureSelMenuVisible(DRO& pMenu);

};
