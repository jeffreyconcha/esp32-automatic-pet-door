#include "door.h"

#include <Arduino.h>

#include "motor.h"

dr::Door::Door(dr::DoorState _initialState, ult::UltraSonic* _proximity) {
    initialState = _initialState;
    proximity = _proximity;
}

void dr::Door::init() {
    motorA = new mtr::Motor(mtr::MotorSelection::MOTOR_A);
    motorB = new mtr::Motor(mtr::MotorSelection::MOTOR_B);
    motorA->init();
    motorB->init();
    pinMode(STOP_T, INPUT_PULLUP);
    switch (initialState) {
    case dr::DoorState::OPENED:
        open();
        break;
    case dr::DoorState::CLOSED:
        close();
        break;
    default:
        break;
    }
}

void dr::Door::open() {
    if (state != dr::DoorState::OPENED && state != dr::DoorState::OPENING) {
        setState(dr::DoorState::OPENING);
        motorA->forward(200);
        motorB->forward(200);
        if (isStopperEnabled()) {
            delay(500);
        }
        timeCounter = 0;
        while (!isStopperEnabled() && !isTimeExpired()) {
            delay(TIME_DELAY);
            timeCounter++;
        }
        stop();
        setState(dr::DoorState::OPENED);
    }
}

void dr::Door::close() {
    if (state != dr::DoorState::CLOSED && state != dr::DoorState::CLOSING) {
        setState(dr::DoorState::CLOSING);
        motorA->reverse(75);
        motorB->reverse(75);
        if (isStopperEnabled()) {
            delay(500);
        }
        bool isClear;
        timeCounter = 0;
        while ((isClear = proximity->isClear()) && !isStopperEnabled() && !isTimeExpired()) {
            delay(TIME_DELAY);
            timeCounter++;
        }
        if (isClear) {
            stop();
            setState(dr::DoorState::CLOSED);
        } else {
            open();
        }
    }
}

void dr::Door::stop() {
    motorA->stop();
    motorB->stop();
}

void dr::Door::setState(dr::DoorState _state) {
    state = _state;
    switch (state) {
    case dr::DoorState::OPENED:
        Serial.println("***************************** OPENED  *****************************");
        break;
    case dr::DoorState::CLOSED:
        Serial.println("***************************** CLOSED  *****************************");
        break;
    case dr::DoorState::OPENING:
        Serial.println("***************************** OPENING *****************************");
        break;
    case dr::DoorState::CLOSING:
        Serial.println("***************************** CLOSING *****************************");
        break;
    }
}

bool dr::Door::isTimeExpired() {
    return timeCounter * TIME_DELAY >= MAX_TIME;
}

bool dr::Door::isOpening() {
    return state == dr::DoorState::OPENING;
}

bool dr::Door::isClosing() {
    return state == dr::DoorState::CLOSING;
}

bool dr::Door::isOpened() {
    return state == dr::DoorState::OPENED;
}

bool dr::Door::isClosed() {
    return state == dr::DoorState::CLOSED;
}

bool dr::Door::isStopperEnabled() {
    return digitalRead(STOP_T) == LOW;
}

dr::Door::~Door() {
    delete motorA;
    delete motorB;
}