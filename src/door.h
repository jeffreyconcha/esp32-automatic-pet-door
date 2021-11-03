#include "motor.h"
#include "ultrasonic.h"

#ifndef DOOR_H
#define DOOR_H
#define MAX_TIME 5000
#define TIME_DELAY 100

#define STOP_T 0

namespace dr {

    enum DoorState {
        OPENED,
        CLOSED
    };

    class Door {
    private:
        mtr::Motor* motorA;
        mtr::Motor* motorB;
        int duration = 0;
        int timeCounter = 0;
        bool closing = false;
        bool opening = false;
        bool closed = true;
        bool opened = false;
        dr::DoorState state;
        ult::UltraSonic* proximity;
        bool isTimeExpired();

    public:
        Door(dr::DoorState, ult::UltraSonic*);
        void init();
        void open();
        void close();
        void stop();
        bool isOpening();
        bool isClosing();
        bool isOpened();
        bool isClosed();
        ~Door();
    };
}

#endif