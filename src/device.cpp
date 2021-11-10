#include "device.h"

#include "esp_timer.h"

dvc::Device::Device(std::string _mac, int _initialRssi) {
    mac = _mac;
    timeCreated = esp_timer_get_time() / 1000;
    if (_initialRssi <= RSSI_OUTSIDE) {
        outside = true;
    }
}

void dvc::Device::setRssi(int _rssi) {
    rssi = _rssi;
    updateTime();
    if (inRange()) {
        outside = false;
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