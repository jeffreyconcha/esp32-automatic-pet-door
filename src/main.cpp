#include <Arduino.h>
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>

#include <map>

#include "device.h"
#include "door.h"
#include "ultrasonic.h"
#include "utils.h"

using namespace std;

#define SCAN_DURATION 10
#define NO_RESULT_DURATION 30

std::map<string, string> tags = {
    {"ff:ff:aa:05:68:d6", "DAMULAG"},
    {"ff:ff:bb:b8:a3:3c", "CALI"},
    {"ff:ff:bb:07:7b:07", "MUNING"},
};

std::map<string, dvc::Device*> devices;
int scanDuration = SCAN_DURATION;
bool isDeviceFound = false;
bool isDoorOpen = false;
bool hasResult = false;
ult::UltraSonic* proximity;
dr::Door* door;
BLEScan* scan;

bool isKnownDevicesInRange(int, string);
bool hasDeviceWithChance();
bool hasDeviceToOpenFromOutside();
bool hasDeviceWithUpdate();
void updateDevicesWithChances();
void openDoor();
void closeDoor();
void removeInactiveDevices();
dvc::Device* getDeviceToOpenFromOutside();

bool isDeviceRegistered(string identifier) {
    for (auto const& tag : tags) {
        if (tag.first == identifier) {
            return true;
        }
    }
    return false;
}

void openDoor() {
    if (door->isClosed()) {
        door->open();
    }
}

void closeDoor() {
    if (door->isOpened()) {
        door->close();
    }
}

class ScanCallback : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice ads) {
        BLEAddress address = ads.getAddress();
        string mac = address.toString();
        int rssi = ads.getRSSI();
        if (isDeviceRegistered(mac)) {
            hasResult = true;
            string name = tags[mac];
            string info = "REGISTERED DEVICE DETECTED: " + mac + " (" + name + "), @" + utl::Utils::toString(rssi);
            Serial.println("-------------------------------------------------------------");
            Serial.println(info.c_str());
            Serial.println("-------------------------------------------------------------");
            if (devices.find(mac) == devices.end()) {
                devices[mac] = new dvc::Device(mac, rssi);
            }
            dvc::Device* device = devices[mac];
            device->setRssi(rssi, door->isClosed());
            if (hasDeviceToOpenFromOutside()) {
                Serial.println("HAS DEVICE OUTSIDE STOPPING SCAN...");
                isDeviceFound = true;
                scan->stop();
            } else {
                if (device->inRange()) {
                    isDeviceFound = true;
                    scan->stop();
                    Serial.println("DEVICE FOUND STOPPING SCAN...");
                } else {
                    if (door->isOpened() && hasDeviceWithChance()) {
                        isDeviceFound = true;
                        scan->stop();
                        Serial.println("HAS DEVICE WITH CHANCE STOPPING SCAN...");
                    } else {
                        Serial.println("NO DEVICES IN RANGE...");
                    }
                }
            }
        }
        removeInactiveDevices();
    }
};

bool hasDeviceWithChance() {
    for (auto const& p : devices) {
        dvc::Device* device = p.second;
        if (device->hasChance()) {
            return true;
        }
    }
    return false;
}

//Increment interrupted OOR devices if no device detected
void updateDevicesWithChances() {
    for (auto const& p : devices) {
        dvc::Device* device = p.second;
        if (device->hasChance()) {
            device->updateChances();
        }
    }
}

void removeInactiveDevices() {
    for (auto const& p : devices) {
        string address = p.first;
        dvc::Device* device = p.second;
        if (!device->isActive() && !device->shouldOpenFromOutside()) {
            string log = "INACTIVE DEVICE: " + address + " (" + tags[address] + ")";
            Serial.println(log.c_str());
            devices.erase(address);
        }
    }
}

bool hasDeviceWithUpdate() {
    for (auto const& p : devices) {
        dvc::Device* device = p.second;
        if (device->hasUpdate()) {
            return true;
        }
    }
    return false;
}

bool hasDeviceToOpenFromOutside() {
    return getDeviceToOpenFromOutside() != 0;
}

dvc::Device* getDeviceToOpenFromOutside() {
    for (auto const& p : devices) {
        dvc::Device* device = p.second;
        if (device->shouldOpenFromOutside()) {
            return device;
        }
    }
    return 0;
}

void setup() {
    Serial.begin(115200);
    Serial.println("INITIALIZING...");
    BLEDevice::init("ESP32-38");
    proximity = new ult::UltraSonic();
    proximity->init();
    door = new dr::Door(dr::DoorState::CLOSED, proximity);
    door->init();
    scan = BLEDevice::getScan();
    scan->setAdvertisedDeviceCallbacks(new ScanCallback());
    scan->setActiveScan(true);
    scan->setInterval(250);
    scan->setWindow(250);
}

void loop() {
    Serial.println("SCANNING...");
    scan->start(SCAN_DURATION, false);
    string result = "SCAN RESULT: " + utl::Utils::toString(isDeviceFound);
    Serial.println(result.c_str());
    if (isDeviceFound) {
        openDoor();
        //RESET THE STATUS
        isDeviceFound = false;
    } else {
        bool _hasResult = hasResult || !hasDeviceWithUpdate();
        if (!hasDeviceToOpenFromOutside() && proximity->isClear() && _hasResult) {
            closeDoor();
        }
        if (!hasResult) {
            updateDevicesWithChances();
        }
    }
    if (door->isOpened()) {
        dvc::Device* device = getDeviceToOpenFromOutside();
        if (device != 0) {
            string log = "** DOOR IS OPEN FOR " + utl::Utils::toString(device->getExpiration()) + " SEC **";
            Serial.println(log.c_str());

        } else {
            Serial.println("** DOOR IS OPEN **");
        }
    } else {
        Serial.println("** DOOR IS CLOSE **");
    }
    scan->clearResults();
    hasResult = false;
}