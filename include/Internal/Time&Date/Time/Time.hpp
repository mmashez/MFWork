#pragma once

#include <ctime>

namespace MF::Chrono::Time {
    static std::time_t GetRawTime() {
        return std::time(nullptr);
    }
}