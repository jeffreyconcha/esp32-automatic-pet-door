#include "door.h"

#include <Arduino.h>

#include "motor.h"
#include "utils.h"

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
        while (!isStopperEnabled() && !isMaxDurationReached(openingTime)) {
            delay(TIME_DELAY);
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
        while ((isClear = proximity->isClear()) && !isStopperEnabled() && !isMaxDurationReached(closingTime)) {
            delay(TIME_DELAY);
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
    int64_t duration;
    std::string log;
    switch (state) {
    case dr::DoorState::OPENING:
        openingTime = utl::Utils::getCurrentTime();
        Serial.println("****************** OPENING ******************");
        break;
    case dr::DoorState::OPENED:
        duration = getDuration(openingTime);
        log = "****************** OPENED -- (DURATION: " + utl::Utils::toString(duration) + ")";
        Serial.println(log.c_str());
        break;
    case dr::DoorState::CLOSING:
        closingTime = utl::Utils::getCurrentTime();
        Serial.println("****************** CLOSING ******************");
        break;
    case dr::DoorState::CLOSED:
        duration = getDuration(closingTime);
        log = "****************** CLOSED -- (DURATION: " + utl::Utils::toString(duration) + ")";
        Serial.println(log.c_str());
        break;
    }
}

int64_t dr::Door::getDuration(int64_t time) {
    return utl::Utils::getCurrentTime() - time;
}

bool dr::Door::isMaxDurationReached(int64_t time) {
    return getDuration(time) >= MAX_TIME;
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