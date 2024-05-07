#pragma once

#include <Arduino.h>
#include <etl/vector.h>
#include "Display.h"

extern Display display;

class Screen {
public:
    Screen() {}

    virtual ~Screen()= default;

    inline void doDirty() {
        display.doDirty();
    }

    virtual void begin() { display.doDirty(); }

    virtual void step() {};

protected:
    bool hasMenu = false;
    friend class Display;
    using Evt = Display::ButtonEvent;

    virtual void drawContents() = 0;

    virtual void onButton(int bt, Evt arg) {};

    virtual void onShow() {};

    virtual void onHide() {};
};