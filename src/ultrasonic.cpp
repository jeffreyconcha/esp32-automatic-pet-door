#include "ultrasonic.h"

#include "Arduino.h"

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
    return distance;
}

bool ult::UltraSonic::isClear() {
    float distance = readDistance();
    if (distance > 0) {
        bool isClear = distance >= MIN_DISTANCE && distance <= MAX_DISTANCE;
        if (!isClear) {
            Serial.print("DISTANCE (cm): ");
            Serial.println(distance);
        }
        return isClear;
    }
    return true;
}