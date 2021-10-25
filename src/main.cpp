#include <Arduino.h>
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>

#include <map>

#include "device.h"
#include "door.h"

using namespace std;

#define SCAN_DURATION 10

const string KNOWN_DEVICE_ADDRESSES[] = {
    "ff:ff:aa:05:68:d6",
    "ff:ff:bb:b8:a3:3c",
    "ff:ff:bb:07:7b:07"};

std::map<string, dvc::Device*> devices;
int scanDuration = SCAN_DURATION;
bool isDeviceFound = false;
bool isDoorOpen = false;
BLEScan* scan;
dr::Door* door;

bool isKnownDevicesInRange(int, string);
bool allDevicesOutOfRange();
void openDoor();
void closeDoor();

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
                devices[mac] = new dvc::Device(mac);
            }
            dvc::Device* device = devices[mac];
            device->setRssi(rssi);
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
            Serial.print(mac.c_str());
            Serial.print(" @ ");
            Serial.println(rssi);
        }
    }
};

bool allDevicesOutOfRange() {
    for (auto const& p : devices) {
        dvc::Device* device = p.second;
        if (device->isTryValid()) {
            return true;
        }
    }
    return true;
}

void setup() {
    Serial.begin(115200);
    Serial.println("SETUP...");
    BLEDevice::init("ESP32-38");
    door = new dr::Door(dr::CLOSED);
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
        closeDoor();
    }
    if (door->isOpened()) {
        Serial.println("** DOOR IS OPEN **");
    } else {
        Serial.println("** DOOR IS CLOSE **");
    }
    scan->clearResults();
}