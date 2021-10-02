#include "motor.h"

#ifndef DOOR_H
#define DOOR_H

#define DURATION 10

class Door {
private:
    Motor* motorA;
    Motor* motorB;
    int duration;
    bool closing;
    bool opening;
    void runTimer();

public:
    void prepare();
    void open();
    void close();
    bool isOpening();
    bool isClosing();
    bool isOpened();
    bool isClosed();
    ~Door();
};

#endif