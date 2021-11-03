#include <iostream>

#ifndef DEVICE_H
#define DEVICE_H

#define MAX_OOR_COUNT 3
#define RSSI_THRESHOLD -70

namespace dvc {
    class Device {
    private:
        std::string mac;
        int rssi;
        int counter;
        bool withinRange;

    public:
        Device(std::string);
        void setRssi(int);
        bool inRange();
        bool hasChance();
        std::string getMac();
        int getRssi();
    };
}

#endif