#include "motor.h"

#include <Arduino.h>

MotorSelection selection;
int terminal1, terminal2;

Motor::Motor(MotorSelection input) {
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
void initialize() {
    pinMode(terminal1, OUTPUT);
    pinMode(terminal2, OUTPUT);
}

void forward() {
    digitalWrite(terminal1, HIGH);
    digitalWrite(terminal2, LOW);
}

void reverse() {
    digitalWrite(terminal1, LOW);
    digitalWrite(terminal2, HIGH);
}

void stop() {
    digitalWrite(terminal1, LOW);
    digitalWrite(terminal2, LOW);
}

MotorSelection getSelection() {
    return selection;
}
