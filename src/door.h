#include <iostream>

#include "motor.h"
#include "ultrasonic.h"

#ifndef DOOR_H
#define DOOR_H
#define MAX_TIME 5000
#define TIME_BOOST 4000
#define TIME_DELAY 10

#define STOP_T 23

namespace dr {

    enum DoorState {
        OPENED,
        CLOSED,
        OPENING,
        CLOSING,
    };

    class Door {
    private:
        mtr::Motor* motorA;
        mtr::Motor* motorB;
        int duration = 0;
        dr::DoorState state;
        dr::DoorState initialState;
        ult::UltraSonic* proximity;
        int64_t openingTime;
        int64_t closingTime;
        int64_t getDuration(int64_t);
        bool isMaxDurationReached(int64_t);
        bool isStopperEnabled();

    public:
        Door(dr::DoorState, ult::UltraSonic*);
        void init();
        void open();
        void close();
        void stop();
        void setState(dr::DoorState);
        bool isOpening();
        bool isClosing();
        bool isOpened();
        bool isClosed();
        ~Door();
    };
}

#endif