#include "door.h"

#include <Arduino.h>

#include "motor.h"

dr::Door::Door(dr::DoorState _state, ult::UltraSonic* _proximity) {
    state = _state;
    proximity = _proximity;
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
        if (isStopperEnabled()) {
            motorA->forward(75);
            motorB->forward(75);
            delay(500);
        }
        motorA->forward(125);
        motorB->forward(125);
        timeCounter = 0;
        while (!isStopperEnabled() && !isTimeExpired()) {
            Serial.println("**OPENING**");
            delay(TIME_DELAY);
            timeCounter++;
        }
        stop();
        opened = true;
        closed = false;
    }
}

void dr::Door::close() {
    if (!closed && !closing) {
        closing = true;
        if (isStopperEnabled()) {
            motorA->reverse(75);
            motorB->reverse(75);
            delay(500);
        }
        motorA->reverse(75);
        motorB->reverse(75);
        bool isClear = proximity->isClear();
        timeCounter = 0;
        while (!isStopperEnabled() && !isTimeExpired() && isClear) {
            Serial.println("**CLOSING**");
            isClear = proximity->isClear();
            delay(TIME_DELAY);
            timeCounter++;
        }
        stop();
        opened = false;
        if (!isClear) {
            open();
            return;
        }
        closed = true;
    }
}

void dr::Door::stop() {
    motorA->stop();
    motorB->stop();
    opening = false;
    closing = false;
}

bool dr::Door::isTimeExpired() {
    return timeCounter * TIME_DELAY >= MAX_TIME;
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

bool dr::Door::isStopperEnabled() {
    return digitalRead(STOP_T) == LOW;
}

dr::Door::~Door() {
    delete motorA;
    delete motorB;
}