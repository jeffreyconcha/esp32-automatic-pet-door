#include "motor.h"

#include <Arduino.h>

mtr::Motor::Motor(MotorSelection input) {
    selection = input;
    switch (input) {
    case MOTOR_A:
        terminal1 = MOTOR_A_T1;
        terminal2 = MOTOR_A_T2;
        break;
    case MOTOR_B:
        terminal1 = MOTOR_B_T1;
        terminal2 = MOTOR_B_T2;
        break;
    }
}

void mtr::Motor::init() {
    pinMode(terminal1, OUTPUT);
    pinMode(terminal2, OUTPUT);
    ledcSetup(CHANNEL, FREQUENCY, RESOLUTION);
    ledcAttachPin(MOTOR_EN_AB, CHANNEL);
}

void mtr::Motor::forward(int speed) {
    digitalWrite(terminal1, HIGH);
    digitalWrite(terminal2, LOW);
    ledcWrite(CHANNEL, speed);
}

void mtr::Motor::reverse(int speed) {
    digitalWrite(terminal1, LOW);
    digitalWrite(terminal2, HIGH);
    ledcWrite(CHANNEL, speed);
}

void mtr::Motor::stop() {
    digitalWrite(terminal1, LOW);
    digitalWrite(terminal2, LOW);
}

mtr::MotorSelection mtr::Motor::getSelection() {
    return selection;
}
