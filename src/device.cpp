#include "device.h"

#include <Arduino.h>

#include <iostream>

#include "utils.h"

dvc::Device::Device(std::string _mac) {
    mac = _mac;
    timeCreated = utl::Utils::getCurrentTime();
    std::string log = "NEW DEVICE FOUND OUTSIDE: " + mac;
    Serial.println(log.c_str());
}

void dvc::Device::setRssi(int _rssi, bool isDoorClosed) {
    timeUpdated = utl::Utils::getCurrentTime();
    rssi = _rssi;
    if (rssi >= RSSI_NORMAL_THRESHOLD) {
        if (forInRangeRechecking) {
            inRangeCount++;
            std::string log = "IN RANGE COUNT(" + mac + "): " + utl::Utils::toString(inRangeCount);
            Serial.println(log.c_str());
            if (inRangeCount >= IN_RANGE_CHECK || rssi >= RSSI_HIGH_THRESHOLD) {
                forInRangeRechecking = false;
                inRangeCount = 0;
                hasBeenInRange = true;
                outOfRangeCount = 0;
                Serial.println("IN RANGE CONFIRMED...");
            }
        } else {
            //Check for rssi history before opening the door.
            if (isDoorClosed && hasBadHistory() && rssi < RSSI_HIGH_THRESHOLD) {
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
        updateChances();
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
            if (rssi >= RSSI_NORMAL_THRESHOLD) {
                Serial.println("WITH GOOD HISTORY");
                return false;
            }
        }
        Serial.println("WITH BAD HISTORY");
        return true;
    }
    return false;
}

int dvc::Device::getRssi() {
    return rssi;
}

bool dvc::Device::inRange() {
    return rssi >= RSSI_NORMAL_THRESHOLD && !forInRangeRechecking;
}

bool dvc::Device::hasChance() {
    if (forInRangeRechecking) {
        Serial.println("UNABLE TO USE CHANCES DUE TO IN RANGE RECHECKING...");
        return false;
    } else {
        return hasBeenInRange && outOfRangeCount <= MAX_OOR_COUNT;
    }
}

int64_t dvc::Device::getUpdateDuration() {
    return utl::Utils::getCurrentTime() - timeUpdated;
}

bool dvc::Device::isActive() {
    return getUpdateDuration() <= MAX_INACTIVE_TIME;
}

bool dvc::Device::hasUpdate() {
    return getUpdateDuration() <= MAX_NO_UPDATE_DURATION;
}

int64_t dvc::Device::getCreateDuration() {
    return utl::Utils::getCurrentTime() - timeCreated;
}

std::string dvc::Device::getMac() {
    return mac;
}

void dvc::Device::updateChances() {
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
}