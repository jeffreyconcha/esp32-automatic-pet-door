#include "door.h"

#include <Arduino.h>

#include "motor.h"

dr::Door::Door(dr::DoorState _state) {
    state = _state;
}

void dr::Door::runTimer() {
    // while (duration < DURATION) {
    //     delay(1000);
    //     duration++;
    // }
    // duration = 0;
    delay(1000);
    motorA->stop();
    motorB->stop();
}

void dr::Door::init() {
    motorA = new mtr::Motor(mtr::MotorSelection::MOTOR_A);
    motorB = new mtr::Motor(mtr::MotorSelection::MOTOR_B);
    motorA->init();
    motorB->init();
    switch (state) {
    case OPENED:
        open();
        break;
    case CLOSED:
        close();
        break;
    }
}

void dr::Door::open() {
    if (!opened && !opening) {
        opening = true;
        motorA->forward();
        motorB->forward();
        runTimer();
        opening = false;
        opened = true;
        closed = false;
    }
}

void dr::Door::close() {
    if (!closed && !closing) {
        closing = true;
        motorA->reverse();
        motorB->reverse();
        runTimer();
        closing = false;
        closed = true;
        opened = false;
    }
}

bool dr::Door::isOpening() {
    return opening;
}

bool dr::Door::isClosing() {
    return closing;
}

bool dr::Door::isOpened() {
    return !opening && opened;
}

bool dr::Door::isClosed() {
    return !closing && closed;
}

dr::Door::~Door() {
    delete motorA;
    delete motorB;
}