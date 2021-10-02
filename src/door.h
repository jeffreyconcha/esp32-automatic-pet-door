#include "motor.h"

#ifndef DOOR_H
#define DOOR_H

#define DURATION 10

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
        bool closing = false;
        bool opening = false;
        bool closed = true;
        bool opened = false;
        dr::DoorState state;
        void runTimer();

    public:
        Door(dr::DoorState);
        void init();
        void open();
        void close();
        bool isOpening();
        bool isClosing();
        bool isOpened();
        bool isClosed();
        ~Door();
    };
}

#endif