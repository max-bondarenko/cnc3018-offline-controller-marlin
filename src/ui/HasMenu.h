#pragma once

#include <etl/vector.h>

class Display;
extern Display display;

struct MenuItem {
    int8_t id;
    const char* text;
    uint8_t* font;
    bool togglable;
    bool on;
    std::function<void(MenuItem&)> cmd;

    static MenuItem simpleItem(int8_t id, const char* text, std::function<void(MenuItem&)> func) {
        return MenuItem{id, text, nullptr, false, false, func};
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