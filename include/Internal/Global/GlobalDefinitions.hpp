#pragma once

#include <string>
#include "../Time&Date/Date/Misc.hpp"
#include "../Time&Date/Time/Misc.hpp"

namespace MF::Global {
    inline static std::string LaunchTime = Chrono::Date::GetDateStr() + " " + Chrono::Time::GetTimeStr();
}