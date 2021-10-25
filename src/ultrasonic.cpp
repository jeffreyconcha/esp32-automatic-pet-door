#include "ultrasonic.h"

#include "Arduino.h"

void ult::UltraSonic::init() {
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

float ult::UltraSonic::readDistance() {
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH);
    float distance = duration * (SPEED_OF_SOUND / 2);
    Serial.print("DISTANCE (cm): ");
    Serial.println(distance);
    return distance;
}