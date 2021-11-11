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

void dvc::Device::setRssi(int _rssi, bool isClosed) {
    int lastRssi = rssi;
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
            //Check if the last rssi is out of range before opening the door.
            if (isClosed && lastRssi < RSSI_THRESHOLD) {
                Serial.println("FOR RECHECKING IN RANGE SIGNAL...");
                forInRangeRechecking = true;
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

bool dvc::Device::isActive() {
    int64_t time = esp_timer_get_time() / 1000;
    int64_t duration = time - timeUpdated;
    return duration <= MAX_INACTIVE_TIME;
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