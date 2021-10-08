#include <Arduino.h>
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>

#include <map>

#include "door.h"

using namespace std;

#define SCAN_DURATION 10
#define RSSI_THRESHOLD -70

const string KNOWN_DEVICE_ADDRESSES[] = {
    "ff:ff:aa:05:68:d6"};

int scanDuration = SCAN_DURATION;
bool isDeviceFound = false;
bool isDoorOpen = false;
std::map<string, int> devices;
BLEScan* scan;
dr::Door* door;

bool isKnownDevicesInRange(int, string);
char* getManufacturerData(string);
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
        string name = ads.getName();
        int rssi = ads.getRSSI();
        string mac = address.toString();
        if (isDeviceRegistered(mac)) {
            if (rssi >= RSSI_THRESHOLD) {
                isDeviceFound = true;
                Serial.println("DEVICE FOUND STOPPING SCAN...");
                scan->stop();
            } else {
                if (devices.find(mac) != devices.end()) {
                    if (devices[mac] >= RSSI_THRESHOLD) {
                        isDeviceFound = false;
                        Serial.println("DEVICE GOES OUT OF RANGE");
                        scan->stop();
                    }
                }
            }
            devices[mac] = rssi;
            // Serial.println(mac.c_str());
            // Serial.println(name.c_str());
            // Serial.println(rssi);
        }
    }
};

void setup() {
    Serial.begin(115200);
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