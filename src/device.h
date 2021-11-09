#include <iostream>

#ifndef DEVICE_H
#define DEVICE_H

#define MAX_OOR_COUNT 3
#define RSSI_THRESHOLD -75
#define MAX_INACTIVE_TIME 60000
#define NEW_ENTRY_EXPIRATION 60000

namespace dvc {
    class Device {
    private:
        std::string mac;
        int rssi;
        int counter;
        int64_t timeCreated;
        int64_t timeUpdated;
        bool withinRange;
        void updateTime();
        int64_t getCreateDuration();

    public:
        Device(std::string);
        void setRssi(int);
        bool inRange();
        bool hasChance();
        bool isActive();
        bool isNewEntry();
        std::string getMac();
        int getRssi();
        int getExpiration();
    };
}

#endif