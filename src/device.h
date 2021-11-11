#include <iostream>

#ifndef DEVICE_H
#define DEVICE_H

#define IN_RANGE_CHECK 3
#define MAX_OOR_COUNT 5
#define RSSI_THRESHOLD -75
#define RSSI_OUTSIDE -85
#define MAX_INACTIVE_TIME 180000
#define FROM_OUTSIDE_OPEN_DURATION 180000

namespace dvc {
    class Device {
    private:
        std::string mac;
        int rssi;
        int inRangeCount = 0;
        int outOfRangeCount = 0;
        int initialRssi;
        int64_t timeCreated;
        int64_t timeUpdated;
        bool hasBeenInRange;
        bool outside;
        bool forInRangeRechecking;
        void updateTime();
        int64_t getAge();

    public:
        Device(std::string, int);
        void setRssi(int, bool);
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