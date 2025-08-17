#pragma once

#include <ctime>

namespace MF::Chrono::Date {
    static std::time_t GetRawDate() {
        return std::time(nullptr);
    }

    static std::tm GetLocalTm() {
        std::time_t t = GetRawDate();
        std::tm tm_local;

        #ifdef _WIN32
            localtime_s(&tm_local, &t);
        #else
            localtime_r(&t, &tm_local);
        #endif

        return tm_local;
    }
}