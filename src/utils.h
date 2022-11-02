#include <algorithm>
#include <iostream>
#include <list>

#include "esp_timer.h"

#ifndef UTILS_H
#define UTILS_H

namespace utl {
    class Utils {
    public:
        static std::string toString(int);
        static int64_t getCurrentTime();
    };
}

#endif