#include <Arduino.h>
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>

#include "door.h"

using namespace std;

#define SCAN_DURATION 5
#define RSSI_THRESHOLD -65

const string KNOWN_DEVICE_ADDRESSES[] = {
    "ff:ff:aa:05:68:d6"};

int scanDuration = SCAN_DURATION;
bool isDeviceFound = false;
bool isDoorOpen = false;
BLEScan *scan;
dr::Door *door;

bool isKnownDevicesInRange(int, string);
char *getManufacturerData(string);
void openDoor();
void closeDoor();

bool isKnownDevicesInRange(string identifier) {
    for (int i = 0; i < sizeof(KNOWN_DEVICE_ADDRESSES); i++) {
        if (KNOWN_DEVICE_ADDRESSES[i] == identifier) {
            return true;
        }
    }
    return false;
}

void openDoor() {
    Serial.println(door->isClosed());
    if (door->isClosed()) {
        Serial.println("OPENING DOOR");
        door->open();
    }
}

void closeDoor() {
    if (door->isOpened()) {
        Serial.println("CLOSING");
        door->close();
    }
}

class ScanCallback : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice device) {
        BLEAddress address = device.getAddress();
        BLEUUID uuid = device.getServiceUUID();
        string name = device.getName();
        int rssi = device.getRSSI();
        string mac = address.toString();
        string id = uuid.toString();
        string data = device.getManufacturerData();
        if (isKnownDevicesInRange(mac)) {
            if (rssi >= RSSI_THRESHOLD) {
                isDeviceFound = true;
                openDoor();
            }
            Serial.println(mac.c_str());
            Serial.println(name.c_str());
            Serial.println(rssi);
        }
    }
};

void setup() {
    Serial.begin(115200);
    pinMode(GPIO_NUM_32, OUTPUT);
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
    Serial.println("SCAN");
    scan->start(SCAN_DURATION, false);
    if (!isDeviceFound) {
        closeDoor();
    } else {
        isDeviceFound = false;
    }
    if (door->isOpened()) {
        Serial.println("DOOR IS OPEN");
    } else {
        Serial.println("DOOR IS CLOSED");
    }
    scan->clearResults();
}