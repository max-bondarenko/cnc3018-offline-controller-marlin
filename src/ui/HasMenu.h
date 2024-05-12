#pragma once

#include <etl/vector.h>

class Display;

extern Display display;



struct MenuItem {
    typedef std::function<void(MenuItem&, int8_t inc)> Cmd;

    uint8_t id;
    const char* text;
    Cmd cmd;

    static MenuItem simpleItem(uint8_t id, const char* text, Cmd&& func) {
        return MenuItem{id, text, func};
    }
};


class HasMenu {
public:
    HasMenu() : firstDisplayedMenuItem{0} {};

protected:
    friend class Display;

    etl::vector<MenuItem, 10> menuItems;

private:
    uint8_t firstDisplayedMenuItem;
};