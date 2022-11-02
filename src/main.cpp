#include <Arduino.h>
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>

#include <map>

#include "ESPAsyncWebServer.h"
#include "WiFi.h"
#include "device.h"
#include "door.h"
#include "ultrasonic.h"
#include "utils.h"

using namespace std;

#define SCAN_DURATION 10
#define NO_RESULT_DURATION 30
#define PROXIMITY_OPENED_DURATION 5000
#define MAX_NO_BLACKLIST_NEARBY_DURATION 10000

const char* SSID = "SKYbroadband224B";
const char* PASSWORD = "086329969";
const BLEUUID SERVICE_UUID("710487f9-e741-4e78-a73f-6cd505bf49cc");

std::map<string, string> tags = {
    {"ff:ff:10:3a:4f:2a", "CALI"},
    {"ff:ff:10:3a:50:9d", "DAMULAG"},
};

std::map<string, string> blacklist = {
    {"ff:ff:aa:05:68:d6", "CHONKI"},
    {"ff:ff:aa:05:6a:03", "CHAKO"},
};

std::map<string, dvc::Device*> devices;
int64_t timeOpenedByProximity = 0;
int64_t timeOfLastBlacklistCheck = 0;
int blacklistCheck = 0;
int scanDuration = SCAN_DURATION;
bool isDeviceFound = false;
bool isBlacklistedFound = false;
bool isDoorOpen = false;
bool hasResult = false;
TaskHandle_t task;
AsyncWebServer server(80);
ult::UltraSonic* proximity;
dr::Door* door;
BLEScan* scan;

bool isDeviceRegistered(string);
bool isDeviceBlacklisted(string);
bool hasDeviceWithChance();
bool hasDeviceWithUpdate();
void updateDevicesWithChances();
void removeInactiveDevices();
void openDoor();
void closeDoor();
void runnable(void*);
int64_t getOpenedByProximityDuration();
int64_t getTimeFromLastBlacklistCheck();

bool isDeviceRegistered(string identifier) {
    for (auto const& tag : tags) {
        if (tag.first == identifier) {
            return true;
        }
    }
    return false;
}

bool isDeviceBlacklisted(string identifier) {
    for (auto const& tag : blacklist) {
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
            if (isDeviceBlacklisted(mac)) {
                std::string log = "BLACKLISTED RSSI: " + utl::Utils::toString(rssi);
                Serial.println(log.c_str());
            }
            bool isBlacklisted = isDeviceBlacklisted(mac) && rssi >= RSSI_BLACKLISTED_THRESHOLD;
            if (isBlacklisted) {
                Serial.println("BLACKLISTED FOUND!!! PROXIMITY SENSOR IS DISABLED FOR 10 SECONDS...");
                isBlacklistedFound = true;
                timeOfLastBlacklistCheck = utl::Utils::getCurrentTime();

            } else {
                int64_t duration = getTimeFromLastBlacklistCheck();
                if (duration >= MAX_NO_BLACKLIST_NEARBY_DURATION) {
                    isBlacklistedFound = false;
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

// Increment interrupted OOR devices if no device detected.
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
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.println("CONNECTING TO WIFI...");
        delay(1000);
    }
    Serial.println("WIFI CONNECTED!!!");
    Serial.println(WiFi.localIP());
    server.on("open", HTTP_GET, [](AsyncWebServerRequest *request) {
        // openDoor();
        request->send(200, "text/plan", "ok");
        Serial.println("WEB SERVER COMMAND: OPEN DOOR");
    });
    server.on("close", HTTP_GET, [](AsyncWebServerRequest *request) {
        // closeDoor();
        request->send(200, "text/plan", "ok");
        Serial.println("WEB SERVER COMMAND: CLOSE DOOR");
    });
    //-------------------BLUETOOTH--------------------//
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
    xTaskCreatePinnedToCore(runnable, "Task", 10000, (void*)&proximity, 1, &task, 1);
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
        // Always reset the device status for each loop.
        isDeviceFound = false;
    } else {
        bool _hasResult = hasResult || !hasDeviceWithUpdate();
        if (proximity->isClear() && _hasResult &&
            getOpenedByProximityDuration() > PROXIMITY_OPENED_DURATION + MAX_TIME) {
            closeDoor();
        }
        Serial.println("IS CLEAR");
        Serial.println(proximity->isClear());
        Serial.println(proximity->readDistance());
    }
    if (door->isOpened()) {
        Serial.println("** DOOR IS OPEN **");
    } else {
        Serial.println("** DOOR IS CLOSE **");
    }
    if (isBlacklistedFound) {
        Serial.println("** BLACKLISTED NEARBY **");
    } else {
        Serial.println("** NO BLACKLISTED NEARBY **");
    }
    scan->clearResults();
    hasResult = false;
}

int64_t getOpenedByProximityDuration() {
    return utl::Utils::getCurrentTime() - timeOpenedByProximity;
}

int64_t getTimeFromLastBlacklistCheck() {
    return utl::Utils::getCurrentTime() - timeOfLastBlacklistCheck;
}

void runnable(void* parameter) {
    for (;;) {
        // if (door->isClosed()) {
        if (door->isClosing()) {
            ult::UltraSonic* sensor = (ult::UltraSonic*)parameter;
            if (!sensor->isClear()) {
                if (!isBlacklistedFound) {
                    timeOpenedByProximity = utl::Utils::getCurrentTime();
                    openDoor();
                } else {
                    Serial.println("UNABLE TO OPEN DOOR, BLACKLISTED FOUND!!!");
                }
            }
        }
        delay(300);
    }
}