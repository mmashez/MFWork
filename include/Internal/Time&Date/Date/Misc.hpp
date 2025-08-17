#include "Date.hpp"
#include <string>
#include <iomanip>

namespace MF::Chrono::Date {
    static std::string GetDateString() {
        std::time_t t = MF::Chrono::Date::GetRawDate();
        std::tm tm_local;

        #ifdef _WIN32
            localtime_s(&tm_local, &t);
        #else
            localtime_r(&t, &tm_local);
        #endif

        std::ostringstream oss;
        oss << std::setw(4) << std::setfill('0') << (tm_local.tm_year + 1900) << "-"
            << std::setw(2) << std::setfill('0') << (tm_local.tm_mon + 1) << "-"
            << std::setw(2) << std::setfill('0') << tm_local.tm_mday;
        return oss.str();
    }
}