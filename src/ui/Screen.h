#pragma once

#include <Arduino.h>
#include <etl/vector.h>
#include "Display.h"


class Screen {
public:
    Screen() : firstDisplayedMenuItem{0} {}

    virtual ~Screen() {}

    void doDirty() {
        display->doDirty();
    }

    virtual void begin() { doDirty(); }

    virtual void step() {};

    void setDisplay(Display* _display) {
        display = _display;
    }

protected:
    friend class Display;
    using Evt = Display::ButtonEvent;

    Display* display;
    etl::vector<MenuItem, 10> menuItems;

    virtual void drawContents() = 0;

    virtual void onButton(int bt, Evt arg) {};

    virtual void onShow() {};

    virtual void onHide() {};
private:
    uint8_t firstDisplayedMenuItem;
};