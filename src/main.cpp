#include <Arduino.h>
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>

using namespace std;

#define LONG_SCAN_DURATION 10
#define SHORT_SCAN_DURATION 1
#define RSSI_THRESHOLD -70

const string KNOWN_DEVICES[] = {
    "4c000215337cd77911c84c029a9bdaa11804b6bf00000000bf",
    "4c0002154853b0e46c144ba996bf65b301407b1b00000000b0"};

int scanDuration = LONG_SCAN_DURATION;
bool isDeviceFound = false;
bool isDoorOpen = false;
BLEScan *scan;

bool isKnownDevicesInRange(int, string);
char *getManufacturerData(string);
void openDoor();
void closeDoor();

bool isKnownDevicesInRange(int rssi, string identifier) {
    if (rssi >= RSSI_THRESHOLD) {
        for (int i = 0; i < sizeof(KNOWN_DEVICES); i++) {
            if (KNOWN_DEVICES[i] == identifier) {
                return true;
            }
        }
    }
    return false;
}

char *getManufacturerData(string data) {
    return BLEUtils::buildHexData(nullptr, (uint8_t *)data.data(), data.length());
}

void openDoor() {
    if (!isDoorOpen) {
        isDoorOpen = true;
        digitalWrite(GPIO_NUM_32, HIGH);
        scanDuration = SHORT_SCAN_DURATION;
        Serial.println("SLIDE UP");
    }
}

void closeDoor() {
    if (isDoorOpen) {
        isDoorOpen = false;
        digitalWrite(GPIO_NUM_32, LOW);
        scanDuration = LONG_SCAN_DURATION;
        Serial.println("SLIDE DOWN");
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
        char *hex = getManufacturerData(data);
        if (isKnownDevicesInRange(rssi, hex)) {
            isDeviceFound = true;
            openDoor();
            Serial.println(mac.c_str());
            // Serial.println(name.c_str());
            // Serial.println(id.c_str());
        }
        Serial.println(rssi);
        Serial.println(hex);
    }
};

void setup() {
    Serial.begin(115200);
    pinMode(GPIO_NUM_32, OUTPUT);
    BLEDevice::init("ESP32-38");
    scan = BLEDevice::getScan();
    scan->setAdvertisedDeviceCallbacks(new ScanCallback());
    scan->setActiveScan(true);
    scan->setInterval(250);
    scan->setWindow(250);
}

void loop() {
    Serial.println("SCAN");
    scan->start(scanDuration, false);
    if (!isDeviceFound) {
        closeDoor();
    } else {
        isDeviceFound = false;
    }
    scan->clearResults();
}