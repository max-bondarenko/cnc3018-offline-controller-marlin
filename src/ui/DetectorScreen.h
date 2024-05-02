#pragma once

#include "constants.h"
#include "Screen.h"
#include "debug.h"
#include "DeviceDetector.h"

class DetectorScreen : public Screen {
public:
    explicit DetectorScreen(DeviceDetector* _detector) : Screen(), nextRefresh{1}, detector{_detector} {}

    ~DetectorScreen() override {
        delete detector;
    }

    void step() override;

protected:
    uint32_t nextRefresh;
    DeviceDetector* detector;

    void drawContents() override;
};
