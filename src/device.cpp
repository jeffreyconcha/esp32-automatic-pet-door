#include "device.h"

#include <Arduino.h>

#include <iostream>

#include "esp_timer.h"
#include "utils.h"

dvc::Device::Device(std::string _mac, int _initialRssi) {
    mac = _mac;
    timeCreated = esp_timer_get_time() / 1000;
    if (_initialRssi <= RSSI_OUTSIDE) {
        outside = true;
        std::string log = "DEVICE FOUND OUTSIDE: " + mac + ", @" + utl::Utils::toString(_initialRssi);
        Serial.println(log.c_str());
    }
}

void dvc::Device::setRssi(int _rssi, bool isDoorClosed) {
    rssi = _rssi;
    updateTime();
    if (rssi >= RSSI_THRESHOLD) {
        outside = false;
        if (forInRangeRechecking) {
            inRangeCount++;
            std::string log = "IN RANGE COUNT(" + mac + "): " + utl::Utils::toString(inRangeCount);
            Serial.println(log.c_str());
            if (inRangeCount >= IN_RANGE_CHECK) {
                forInRangeRechecking = false;
                inRangeCount = 0;
                hasBeenInRange = true;
                outOfRangeCount = 0;
                Serial.println("IN RANGE CONFIRMED...");
            }
        } else {
            //Check for rssi history before opening the door.
            if (isDoorClosed && hasBadHistory()) {
                Serial.println("FOR RECHECKING IN RANGE SIGNAL...");
                forInRangeRechecking = true;
                inRangeCount = 1;
                std::string log = "IN RANGE COUNT(" + mac + "): " + utl::Utils::toString(inRangeCount);
            } else {
                hasBeenInRange = true;
                outOfRangeCount = 0;
            }
        }
    } else {
        //Device has to be in range first before it can use its 5 chances.
        if (hasBeenInRange) {
            outOfRangeCount++;
            std::string log = "OUT OF RANGE COUNT(" + mac + "): " + utl::Utils::toString(outOfRangeCount);
            Serial.println(log.c_str());
            if (outOfRangeCount >= MAX_OOR_COUNT) {
                hasBeenInRange = false;
                outOfRangeCount = 0;
            }
        }
        forInRangeRechecking = false;
        inRangeCount = 0;
    }
    updateHistory(rssi);
}

void dvc::Device::updateHistory(int rssi) {
    history.push_front(rssi);
    if (history.size() > MAX_RSSI_RECORD_HISTORY) {
        history.pop_back();
    }
}

bool dvc::Device::hasBadHistory() {
    if (history.size() >= MAX_RSSI_RECORD_HISTORY) {
        //Bad history requires all rssi record to be out of range
        for (int rssi : history) {
            std::string log = "RSSI HISTORY: " + utl::Utils::toString(rssi);
            Serial.println(log.c_str());
            if (rssi >= RSSI_THRESHOLD) {
                Serial.println("WITH GOOD HISTORY");
                return false;
            }
        }
        Serial.println("WITH BAD HISTORY");
        return true;
    }
    return false;
}

void dvc::Device::updateTime() {
    timeUpdated = esp_timer_get_time() / 1000;
}

int dvc::Device::getRssi() {
    return rssi;
}

bool dvc::Device::inRange() {
    return rssi >= RSSI_THRESHOLD && !forInRangeRechecking;
}

bool dvc::Device::hasChance() {
    if (forInRangeRechecking) {
        Serial.println("UNABLE TO USE CHANCES DUE TO IN RANGE RECHECKING...");
        return false;
    } else {
        return hasBeenInRange && outOfRangeCount <= MAX_OOR_COUNT;
    }
}

int64_t dvc::Device::getLastUpdate() {
    int64_t time = esp_timer_get_time() / 1000;
    return time - timeUpdated;
}

bool dvc::Device::isActive() {
    return getLastUpdate() <= MAX_INACTIVE_TIME;
}

bool dvc::Device::hasUpdate() {
    return getLastUpdate() <= MAX_NO_UPDATE_DURATION;
}

bool dvc::Device::shouldOpenFromOutside() {
    return outside && getAge() <= FROM_OUTSIDE_OPEN_DURATION;
}

int64_t dvc::Device::getAge() {
    int64_t time = esp_timer_get_time() / 1000;
    return time - timeCreated;
}

int dvc::Device::getExpiration() {
    int64_t age = getAge();
    if (age <= FROM_OUTSIDE_OPEN_DURATION) {
        return (FROM_OUTSIDE_OPEN_DURATION - age) / 1000;
    }
    return 0;
}

std::string dvc::Device::getMac() {
    return mac;
}