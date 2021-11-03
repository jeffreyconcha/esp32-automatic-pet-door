#include "device.h"

dvc::Device::Device(std::string _mac) {
    mac = _mac;
}

void dvc::Device::setRssi(int _rssi) {
    rssi = _rssi;
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

int dvc::Device::getRssi() {
    return rssi;
}

bool dvc::Device::inRange() {
    return rssi >= RSSI_THRESHOLD;
}

bool dvc::Device::hasChance() {
    return withinRange && counter <= MAX_OOR_COUNT;
}

std::string dvc::Device::getMac() {
    return mac;
}