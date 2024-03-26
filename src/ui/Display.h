#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <etl/vector.h>
#include <functional>

#include "constants.h"
#include "devices/GCodeDevice.h"
#include "Job.h"


class Screen;

struct MenuItem {
    int16_t id;
    String text;
    bool togglable;
    bool on;
    uint8_t* font;
    std::function<void(MenuItem&)> cmd;

    static MenuItem simpleItem(int16_t id, const char* text, std::function<void(MenuItem&)> func) {
        return MenuItem{id, text, false, false, nullptr, func};
    }
};


class Display : public JobObserver, public DeviceObserver {
public:
    static constexpr uint8_t STATUS_BAR_HEIGHT = 16;
    static constexpr uint8_t LINE_HEIGHT = 11;
    static constexpr int HOLD_COUNT = 30; // x10 = ms
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

    static uint16_t buttStates;

    enum class ButtonEvent {
        UP, DOWN, HOLD
    };

    Display(Job& _job, U8G2& _u8g2);

    ~Display() final {}

    void doDirty() { dirty = true; }

    void notification(const DeviceStatusEvent& e) override {
        if (devStatus != e.status) {
            devStatus = e.status;
            devStatusString = e.str;
        }
        doDirty();
    }

    void notification(const JobStatusEvent e) override {
        // TODO use events to update screen
        doDirty();
    }

    U8G2& getU8G2() const {
        return u8g2;
    }

    void begin();

    ///
    /// call in each main loop step
    ///
    void step();

    void draw();

    void setScreen(Screen* screen);

    void processInput();

private:
    U8G2& u8g2;
    Job& job;
    Screen* cScreen;
    bool dirty;
    uint8_t selMenuItem = 0;
    bool menuShown;
    bool menuShownWhenDown;
    String devStatusString;
    size_t devStatus;

    decltype(buttStates) prevStates;

    int16_t holdCounter[N_BUTTONS];

    void processMenuButton(uint8_t bt, ButtonEvent evt);

    void drawStatusBar();

    void drawMenu();

    void ensureSelMenuVisible();

};
