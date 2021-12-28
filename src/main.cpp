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
#define MAX_PROXIMITY_OPENED_DURATION 5000

const BLEUUID SERVICE_UUID("710487f9-e741-4e78-a73f-6cd505bf49cc");

std::map<string, string> tags = {
    {"ff:ff:aa:05:6a:03", "MUNING"},
    {"ff:ff:aa:05:68:d6", "CALI"},
    {"ff:ff:bb:07:7b:07", "DAMULAG"},
    {"ff:ff:18:19:b8:82", "DEPAN"},
};

std::map<string, dvc::Device*> devices;
int64_t timeOpenedByProximity = 0;
int scanDuration = SCAN_DURATION;
bool isDeviceFound = false;
bool isDoorOpen = false;
bool hasResult = false;
TaskHandle_t task;
ult::UltraSonic* proximity;
dr::Door* door;
BLEScan* scan;

bool isKnownDevicesInRange(int, string);
bool hasDeviceWithChance();
bool hasDeviceWithUpdate();
void updateDevicesWithChances();
void removeInactiveDevices();
void openDoor();
void closeDoor();
void runnable(void*);
int64_t getOpenedByProximityDuration();

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
        if (ads.haveServiceUUID() && ads.isAdvertisingService(SERVICE_UUID)) {
            Serial.println("DEVICE FOUND OUTSIDE!!!");
            isDeviceFound = true;
            openDoor();
        } else {
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
                    devices[mac] = new dvc::Device(mac);
                }
                dvc::Device* device = devices[mac];
                device->setRssi(rssi, door->isClosed());
                if (!isDeviceFound) {
                    if (device->inRange()) {
                        isDeviceFound = true;
                        openDoor();
                        Serial.println("DEVICE FOUND!!!");
                    } else {
                        if (door->isOpened() && hasDeviceWithChance()) {
                            isDeviceFound = true;
                            Serial.println("DEVICE WITH CHANCE FOUND!!!");
                        } else {
                            Serial.println("NO DEVICES IN RANGE");
                        }
                    }
                }
            }
        }
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

//Increment interrupted OOR devices if no device detected.
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
        if (!device->isActive()) {
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
    xTaskCreatePinnedToCore(runnable, "Task", 10000, NULL, 0, &task, 0);
}

void loop() {
    Serial.println("SCANNING...");
    scan->start(SCAN_DURATION, false);
    removeInactiveDevices();
    if (hasResult) {
        string result = "SCAN RESULT: " + utl::Utils::toString(isDeviceFound);
        Serial.println(result.c_str());
    } else {
        updateDevicesWithChances();
        Serial.println("NO DEVICE DETECTED");
    }
    if (isDeviceFound) {
        //Always reset the device status for each loop.
        isDeviceFound = false;
    } else {
        bool _hasResult = hasResult || !hasDeviceWithUpdate();
        if (proximity->isClear() && _hasResult && getOpenedByProximityDuration() > MAX_PROXIMITY_OPENED_DURATION) {
            closeDoor();
        }
    }
    if (door->isOpened()) {
        Serial.println("** DOOR IS OPEN **");
    } else {
        Serial.println("** DOOR IS CLOSE **");
    }
    scan->clearResults();
    hasResult = false;
}

int64_t getOpenedByProximityDuration() {
    return utl::Utils::getCurrentTime() - timeOpenedByProximity;
}

void runnable(void* params) {
    while (true) {
        if (door->isClosed()) {
            if (!proximity->isClear()) {
                timeOpenedByProximity = utl::Utils::getCurrentTime();
                openDoor();
            }
        }
        delay(100);
    }
}