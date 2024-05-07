#include "Config.h"

auto spindleSetF = [](const char* token, GCodeDevice::Config& conf) {
    auto val = (uint16_t) strtoul(token, nullptr, STRTOLL_BASE);
    conf.spindle.push_back(val);
};
auto feedSetF = [](const char* token, GCodeDevice::Config& conf) {
    auto val = (uint16_t) strtoul(token, nullptr, STRTOLL_BASE);

    conf.feed.push_back(val);
};
auto distanceSetF = [](const char* token, GCodeDevice::Config& conf) {
    auto val = (float) atof(token);
    conf.dist.push_back(abs(val));
};

static int configHandler(void* store, const char* section, const char* name, const char* value) {
    auto& device = *(GCodeDevice*) (store);
    if (strcmp(section, device.name) == 0) {
        void (* pFunction)(const char*, GCodeDevice::Config&);
        if (strcmp(name, "s") == 0) {
            device.config.spindle.clear();
            device.config.spindle.push_back(0);
            pFunction = spindleSetF;
        } else if (strcmp(name, "f") == 0) {
            device.config.feed.clear();
            pFunction = feedSetF;
        } else if (strcmp(name, "d") == 0) {
            device.config.dist.clear();
            pFunction = distanceSetF;
        } else
            return 1;
        size_t len = strlen(value);
        char buf[len + 1];
        buf[len] = 0;
        memcpy(buf, value, len);
        char* t = strtok(buf, ",");
        while (t != nullptr) {
            pFunction(t, device.config);
            t = strtok(nullptr, ",");
        }
        return 0;
    }
    return 1;   /* unknown section/name, error */
}


void readConfig(GCodeDevice* dev) {
    if (SD.begin(PIN_CD)) {
        File file = SD.open(PRESETS_FILE);
        if (file.available()) {
            //ignore output
            ini_parse_stream([](char* str, int num, void* stream_) -> char* {
                auto stream = *(File*) stream_; //always present
                size_t i = stream.readBytesUntil('\n', str, num);
                if (i > 0) {
                    str[i] = 0;
                    return str;
                } else
                    return nullptr;
            }, &file, (ini_handler) (configHandler), dev);
        }
        file.close();
    }
}