#include "device.h"

#include "esp_timer.h"

dvc::Device::Device(std::string _mac) {
    mac = _mac;
    timeCreated = esp_timer_get_time() / 1000;
}

void dvc::Device::setRssi(int _rssi) {
    rssi = _rssi;
    updateTime();
    if (inRange()) {
        withinRange = true;
        counter = 0;
    } else {
        counter++;
        if (counter > MAX_OOR_COUNT) {
            withinRange = false;
        }
    }
}

void dvc::Device::updateTime() {
    timeUpdated = esp_timer_get_time() / 1000;
}

int dvc::Device::getRssi() {
    return rssi;
}

bool dvc::Device::inRange() {
    return rssi >= RSSI_THRESHOLD;
}

bool dvc::Device::hasChance() {
    return withinRange && counter <= MAX_OOR_COUNT;
}

bool dvc::Device::isActive() {
    int64_t time = esp_timer_get_time() / 1000;
    int64_t duration = time - timeUpdated;
    return duration <= MAX_INACTIVE_TIME;
}

bool dvc::Device::isNewEntry() {
    return getCreateDuration() <= NEW_ENTRY_EXPIRATION;
}

int64_t dvc::Device::getCreateDuration() {
    int64_t time = esp_timer_get_time() / 1000;
    return time - timeCreated;
}

int dvc::Device::getExpiration() {
    int64_t duration = getCreateDuration();
    if (duration <= NEW_ENTRY_EXPIRATION) {
        return (NEW_ENTRY_EXPIRATION - duration) / 1000;
    }
    return 0;
}

std::string dvc::Device::getMac() {
    return mac;
}