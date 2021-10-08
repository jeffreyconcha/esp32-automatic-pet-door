#include "door.h"

#include <Arduino.h>

#include "motor.h"

dr::Door::Door(dr::DoorState _state) {
    state = _state;
}

void dr::Door::init() {
    motorA = new mtr::Motor(mtr::MotorSelection::MOTOR_A);
    motorB = new mtr::Motor(mtr::MotorSelection::MOTOR_B);
    motorA->init();
    motorB->init();
    pinMode(STOP_T, INPUT_PULLUP);
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
        while(digitalRead(STOP_T) == HIGH) {
            Serial.println("**OPENING**");
        }
        stop();
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
        while(digitalRead(STOP_T) == HIGH) {
            Serial.println("**CLOSING**");
        }
        stop();
        closing = false;
        closed = true;
        opened = false;
    }
}

void dr::Door::stop() {
    motorA->stop();
    motorB->stop();
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