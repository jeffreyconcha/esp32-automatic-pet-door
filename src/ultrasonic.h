#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#define TRIGGER_PIN 32
#define ECHO_PIN 33
#define SPEED_OF_SOUND 0.034

namespace ult {
    class UltraSonic {
    private:
    public:
        void init();
        float readDistance();
    };
}

#endif