#include "utils.h"

#include <Arduino.h>

std::string utl::Utils::toString(int input) {
    return String(input).c_str();
}

int64_t utl::Utils::getCurrentTime() {
    return esp_timer_get_time() / 1000;
}