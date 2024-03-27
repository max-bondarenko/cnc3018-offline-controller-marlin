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

    Display* display;

    using Evt = Display::ButtonEvent;
    etl::vector<MenuItem, 10> menuItems;

    virtual void drawContents() = 0;

    virtual void onButton(int bt, Display::ButtonEvent arg) {};

    virtual void onShow() {};

    virtual void onHide() {};
private:
    uint8_t firstDisplayedMenuItem;
};