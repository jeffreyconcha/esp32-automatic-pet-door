#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#define TRIGGER_PIN 32
#define ECHO_PIN 33
#define SPEED_OF_SOUND 0.034
#define MIN_DISTANCE 45
#define MAX_DISTANCE 49
#define READING_COUNT 7

namespace ult {
    class UltraSonic {
    private:
        bool isClear(float);

    public:
        void init();
        float readDistance();
        bool isClear();
    };
}

#endif