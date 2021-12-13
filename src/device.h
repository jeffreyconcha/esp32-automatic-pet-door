#include <iostream>
#include <list>

#ifndef DEVICE_H
#define DEVICE_H

#define IN_RANGE_CHECK 3
#define MAX_OOR_COUNT 5
#define RSSI_HIGH_THRESHOLD -65
#define RSSI_NORMAL_THRESHOLD -75
#define RSSI_LOW_THRESHOLD -80
#define MAX_RSSI_RECORD_HISTORY 3
#define MAX_NO_UPDATE_DURATION 30000
#define MAX_INACTIVE_TIME 300000
#define FROM_OUTSIDE_OPEN_DURATION 60000

namespace dvc {
    class Device {
    private:
        std::string mac;
        int rssi;
        int initialRssi;
        int inRangeCount = 0;
        int outOfRangeCount = 0;
        int64_t timeCreated;
        int64_t timeUpdated;
        std::list<int> history;
        bool outside = false;
        bool hasBeenInRange = false;
        bool forInRangeRechecking = false;
        bool hasBadHistory();
        void updateHistory(int);
        int64_t getCreateDuration();
        int64_t getUpdateDuration();

    public:
        Device(std::string, int);
        void setRssi(int, bool);
        void updateChances();
        bool inRange();
        bool hasChance();
        bool isActive();
        bool hasUpdate();
        bool shouldOpenFromOutside();
        std::string getMac();
        int getRssi();
        int getExpiration();
    };
}

#endif