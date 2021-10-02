#include "door.h"
#include "motor.h"

#include <Arduino.h>

bool opening = false;
bool closing = false;
int duration = 0;
Motor* motorA;
Motor* motorB;

void runTimer() {
    while (duration < DURATION) {
        delay(1000);
        duration++;
    }
    duration = 0;
}

void prepare() {
    motorA = new Motor(MOTOR_A);
    motorB = new Motor(MOTOR_B);
    motorA->initialize();
    motorB->initialize();
}

void open() {
    opening = true;
    motorA->forward();
    motorB->forward();
    runTimer();
    opening = false;
}

void close() {
    closing = true;
    motorA->reverse();
    motorB->reverse();
    runTimer();
    closing = false;
}

bool isOpening() {
    return opening;
}

bool isClosing() {
    return closing;
}

bool isOpened() {
    return true;
}

bool isClosed() {
    return true;
}

Door::~Door() {
    delete motorA;
    delete motorB;
}