#include "utils.h"

#include <Arduino.h>

std::string utl::Utils::toString(int input) {
    // std::ostringstream temp;
    // temp << input;
    // return temp.str();
    return String(input).c_str();
}