#pragma once

#include <Arduino.h>
#include <etl/vector.h>
#include "Display.h"

extern Display display;

class Screen {
public:
    struct MenuItem {
        typedef std::function<void(MenuItem&, int8_t inc)> Cmd;

        const char* text;
        Cmd cmd;

        static MenuItem* simpleItem(const char* text, Cmd&& func) {
            return new MenuItem{text, func};
        }
    };

    Screen() : firstDisplayedMenuItem{0} {}

    virtual ~Screen() = default;

    inline void doDirty() {
        display.doDirty();
    }

    virtual void begin() { display.doDirty(); }

    virtual void step() {};

protected:
    friend class Display;

    using Evt = Display::ButtonEvent;

    struct Menu {
        MenuItem** items;
        uint8_t size;
        // never be used
        /*~Menu() {
            for (int i = 0; i < size; ++i) {
                delete arr[i];
            }
            delete[] arr;
        }*/

        MenuItem& operator[](size_t i) const {
            return *items[(uint8_t) i]; //yes, no checks. WALHAAALLLAAAA!!!!1111
        }
    };

    Menu menuItems;
    uint8_t firstDisplayedMenuItem;

    virtual void drawContents() = 0;

    virtual void onButton(int bt, Evt arg) {};

    virtual void onShow() {};

    virtual void onHide() {};
};