#pragma once

#include "Time.hpp"
#include <string>
#include <iomanip>

namespace MF::Chrono::Time {
    static std::string GetTimeStr() {
        std::time_t t = GetRawTime();
        std::tm tm_local;

        #ifdef _WIN32
            localtime_s(&tm_local, &t);
        #else
            localtime_r(&t, &tm_local);
        #endif

        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << tm_local.tm_hour << ":"
            << std::setw(2) << std::setfill('0') << tm_local.tm_min << ":"
            << std::setw(2) << std::setfill('0') << tm_local.tm_sec;
        return oss.str();
    }
}