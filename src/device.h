#include <iostream>

#ifndef DEVICE_H
#define DEVICE_H

#define MAX_OOR_COUNT 5
#define RSSI_THRESHOLD -75
#define RSSI_DOOR -80
#define RSSI_OUTSIDE -90
#define MAX_INACTIVE_TIME 60000
#define FROM_OUTSIDE_OPEN_DURATION 180000

namespace dvc {
    class Device {
    private:
        std::string mac;
        int rssi;
        int counter;
        int initialRssi;
        int64_t timeCreated;
        int64_t timeUpdated;
        bool withinRange;
        bool outside;
        void updateTime();
        int64_t getAge();

    public:
        Device(std::string, int);
        void setRssi(int);
        bool inRange();
        bool hasChance();
        bool isActive();
        bool shouldOpenFromOutside();
        std::string getMac();
        int getRssi();
        int getExpiration();
    };
}

#endif