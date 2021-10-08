#ifndef MOTOR_H
#define MOTOR_H

#define MOTOR_A_T1 16
#define MOTOR_A_T2 17
#define MOTOR_B_T1 18
#define MOTOR_B_T2 19

namespace mtr {

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
        Motor(mtr::MotorSelection);
        MotorSelection getSelection();
        void init();
        void forward();
        void reverse();
        void stop();
    };
}

#endif