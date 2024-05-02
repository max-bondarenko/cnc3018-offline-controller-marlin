#include <Arduino.h>
#include "U8g2lib.h"
#include "debug.h"
#include "constants.h"
#include "WatchedSerial.h"

#include "ui/DetectorScreen.h"
#include "ui/GrblDRO.h"
#include "ui/MarlinDRO.h"
#include "ui/FileChooser.h"
#include "ui/Display.h"

#include "etl/deque.h"
#include "devices/Config.h"
#include "devices/DeviceDetector.h"
#include "devices/GrblDevice.h"
#include "devices/MarlinDevice.h"

#include "Job.h"

U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI _u8g2(
    U8G2_R0,
    PIN_LCD_CLK,
    PIN_LCD_MOSI,
    PIN_LCD_CS,
    PIN_LCD_DC,
    PIN_LCD_RST
);

U8G2& Display::u8g2 = _u8g2;
WatchedSerial serialCNC{Serial1, PIN_DET};
Job job;
Display display;
FileChooser fileChooser;

GCodeDevice* dev;
DRO* dro;
DeviceDetector* detector;
DetectorScreen* detectorScreen;

void createDevice(const char* const devName) {
    if (devName == DEVICE_NAMES[DeviceName::GRBL]) {
        auto device = new GrblDevice();
        dro = new GrblDRO(*device);
        dev = device;
    } else {
        auto device = new MarlinDevice();
        dro = new MarlinDRO(*device);
        dev = device;
    }
    dev->name = devName;
    readConfig(dev);
    job.setDevice(dev);
    display.setScreen(dro);
    dev->begin(nullptr);
    dev->add_observer(display);
    dro->begin();
    dro->enableRefresh();
    delete detectorScreen;
    detectorScreen = nullptr;
    detector = nullptr;
    LOGLN("Created");
}

void setup() {
    SerialUSB.begin(115200);

    _u8g2.begin();
    _u8g2.setFontPosTop();
    _u8g2.setFontMode(1);
    _u8g2.setDrawColor(1);

    display.begin();
    detector = new DeviceDetector(serialCNC, (DeviceDetector::DeviceCallback*) createDevice);
    detectorScreen = new DetectorScreen(detector);
    display.setScreen(detectorScreen);
    fileChooser.setCallback([](bool res, const char* path) {
        if (res) {
            LOGF("Starting job %s\n", path);
            job.setFile(path);
            job.start();
        }
        display.setScreen(dro);
    });
    fileChooser.begin();
    job.add_observer(display);
    for (auto pin: buttPins) {
        pinMode(pin, INPUT_PULLUP);
    }
}

void loop() {
    static uint32_t nextRead;
    // poll buttons
    if (int32_t(millis() - nextRead) > 0) {
        for (int i = 0; i < N_BUTT; i++) {
            bitWrite(display.buttStates, i, (digitalRead(buttPins[i]) == 0 ? 1 : 0));
        }
        display.processInput();
        nextRead = millis() + 19;
    }
    //END poll buttons

    display.step();
    if (dev != nullptr) {
        job.step();
        dev->step();
    } else {
        detector->loop();
    }

#ifdef USB_TO_SERIAL
    //send all data from pc to device
    if (SerialUSB.available()) {
        while (SerialUSB.available()) {
            serialCNC.write(SerialUSB.read());
        }
    }
#endif
}

