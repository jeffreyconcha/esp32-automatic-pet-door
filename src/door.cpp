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
        motorA->forward();
        motorB->forward();
        timeCounter = 0;
        do {
            Serial.println("**OPENING**");
            delay(TIME_DELAY);
            timeCounter++;
        } while (digitalRead(STOP_T) == HIGH && !isTimeExpired());
        stop();
        opened = true;
        closed = false;
    }
}

void dr::Door::close() {
    if (!closed && !closing) {
        closing = true;
        motorA->reverse();
        motorB->reverse();
        bool isClear = proximity->isClear();
        timeCounter = 0;
        do {
            Serial.println("**CLOSING**");
            isClear = proximity->isClear();
            delay(TIME_DELAY);
            timeCounter++;
        } while (digitalRead(STOP_T) == HIGH && !isTimeExpired() && isClear);
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

dr::Door::~Door() {
    delete motorA;
    delete motorB;
}