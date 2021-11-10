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
#define NEW_ENTRY_OPEN_DURATION 60000

const string KNOWN_DEVICE_ADDRESSES[] = {
    "ff:ff:aa:05:68:d6",
    "ff:ff:bb:b8:a3:3c",
    "ff:ff:bb:07:7b:07"};

std::map<string, dvc::Device*> devices;
int scanDuration = SCAN_DURATION;
bool isDeviceFound = false;
bool isDoorOpen = false;
ult::UltraSonic* proximity;
dr::Door* door;
BLEScan* scan;

bool isKnownDevicesInRange(int, string);
bool allDevicesOutOfRange();
bool hasDeviceToOpenFromOutside();
void openDoor();
void closeDoor();
void removeInactiveDevices();
dvc::Device* getDeviceToOpenFromOutside();

bool isDeviceRegistered(string identifier) {
    for (int i = 0; i < sizeof(KNOWN_DEVICE_ADDRESSES); i++) {
        if (KNOWN_DEVICE_ADDRESSES[i] == identifier) {
            return true;
        }
    }
    return false;
}

void openDoor() {
    if (door->isClosed()) {
        Serial.println("OPENING DOOR...");
        door->open();
    }
}

void closeDoor() {
    if (door->isOpened()) {
        Serial.println("CLOSING DOOR...");
        door->close();
    }
}

class ScanCallback : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice ads) {
        BLEAddress address = ads.getAddress();
        string mac = address.toString();
        int rssi = ads.getRSSI();
        if (isDeviceRegistered(mac)) {
            if (devices.find(mac) == devices.end()) {
                devices[mac] = new dvc::Device(mac, rssi);
                string log = "NEW DEVICE FOUND: " + mac + ", @" + utl::Utils::toString(rssi);
                Serial.println(log.c_str());
            }
            dvc::Device* device = devices[mac];
            device->setRssi(rssi);
            if (hasDeviceToOpenFromOutside()) {
                Serial.println("HAS DEVICE OUTSIDE...");
                isDeviceFound = true;
                scan->stop();
            } else {
                if (device->inRange()) {
                    isDeviceFound = true;
                    scan->stop();
                    Serial.println("DEVICE FOUND STOPPING SCAN...");
                } else {
                    if (allDevicesOutOfRange()) {
                        isDeviceFound = false;
                        scan->stop();
                        Serial.println("ALL DEVICES GOES OUT OF RANGE!!!");
                    }
                }
            }
            string log = mac + " @ " + utl::Utils::toString(rssi);
            Serial.println(log.c_str());
        }
        removeInactiveDevices();
    }
};

bool allDevicesOutOfRange() {
    for (auto const& p : devices) {
        dvc::Device* device = p.second;
        if (device->hasChance()) {
            return false;
        }
    }
    return true;
}

void removeInactiveDevices() {
    for (auto const& p : devices) {
        string address = p.first;
        dvc::Device* device = p.second;
        if (!device->isActive()) {
            devices.erase(address);
            string log = "INACTIVE DEVICE: " + address;
            Serial.println(log.c_str());
        }
    }
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
    door = new dr::Door(dr::CLOSED, proximity);
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
    if (isDeviceFound) {
        openDoor();
        isDeviceFound = false;
    } else {
        if (proximity->isClear()) {
            closeDoor();
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
}