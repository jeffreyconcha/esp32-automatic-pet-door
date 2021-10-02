#ifndef MOTOR_H
#define MOTOR_H

#define MOTOR_A_T1 32
#define MOTOR_A_T2 33
#define MOTOR_B_T1 34
#define MOTOR_B_T2 35

enum MotorSelection {
    MOTOR_A,
    MOTOR_B
};

class Motor {
private:
    MotorSelection selection;
    int terminal1;
    int terminal2;
public:
    Motor(MotorSelection);
    MotorSelection getSelection();
    void initialize();
    void forward();
    void reverse();
    void stop();
};

#endif