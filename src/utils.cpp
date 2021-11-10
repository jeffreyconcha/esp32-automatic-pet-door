#include "utils.h"

std::string utl::Utils::toString(int input) {
    std::ostringstream temp;
    temp << input;
    return temp.str();
}