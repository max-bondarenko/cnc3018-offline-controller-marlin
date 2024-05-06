#pragma once

#include "DRO.h"
#include "devices/MarlinDevice.h"


// TODO done list
// todo done 1 extruder state (extrusion allowed temp)
// todo done 2 show temp & extruder on main DRO while running job
class MarlinDRO : public DRO {
public:
    constexpr static uint8_t MAX_TEMP = 250;

    MarlinDRO(MarlinDevice& d) : DRO(), dev(d) {
        defaultAxisPrecision = 2;
    }

    ~MarlinDRO() {}

    void begin() override;

protected:
    MarlinDevice& dev;

    void drawContents() override;

    void onButton(int bt, Evt arg) override;

    void onButtonTemp(uint8_t bt, Evt evt);

private:
    int expectedTemp = 0;
    int expectedBedTemp = 0;
    bool relative;

    void drawAxisIcons(uint8_t sx, uint8_t sy, uint8_t lineHeight) const;

    void inline drawTemp(uint16_t sx, uint16_t sy, uint8_t lineHeight, uint8_t lineWidth) const;

    void inline drawPower(uint16_t sx, uint16_t sy, uint8_t lineHeight, uint16_t val) const;
};