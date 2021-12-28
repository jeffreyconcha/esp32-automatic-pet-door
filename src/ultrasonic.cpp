#include "ultrasonic.h"

#include "Arduino.h"
#include "utils.h"

void ult::UltraSonic::init() {
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

float ult::UltraSonic::readDistance() {
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH);
    float distance = duration * (SPEED_OF_SOUND / 2);
    if (lastDistance > 0) {
        float difference = lastDistance - distance;
        float value = abs(difference);
        lastDistance = distance;
        if (value <= ALLOWANCE) {
            return distance;
        }
        return 0;
    }
    return lastDistance = distance;
}

bool ult::UltraSonic::isClear() {
    float distance = readDistance();
    std::string log = "DISTANCE (cm): " + utl::Utils::toString(distance);
    Serial.println(log.c_str());
    if (distance > 0 && distance < 1000) {
        bool isClear = distance >= MIN_DISTANCE && distance <= MAX_DISTANCE;
        if (!isClear) {
            // std::string log = "DISTANCE (cm): " + utl::Utils::toString(distance);
            // Serial.println(log.c_str());
        }
        return isClear;
    }
    return true;
}