#pragma once

#include "../../Internal/RawLogging/RawLogging.hpp"
#include "../../Internal/Time&Date/Time/Misc.hpp"
#include <string>

namespace MF::Print {
    enum class LogLevel {
        Debug = 2,
        Info = 1,
        Warning = 0,
        Error = -1
    };

    inline std::string LogLevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Info: return "INFO";
            case LogLevel::Warning: return "WARNING";
            case LogLevel::Error: return "ERROR";
        }
        return "UNKNOWN";
    }

    inline LogLevel CurrentLevel = LogLevel::Debug;

    inline void Out(LogLevel level, const std::string& message, bool newline = true) {
        if (static_cast<int>(level) > static_cast<int>(CurrentLevel) || message.empty()) return;
        std::string Time = "[" + Chrono::Time::GetTimeStr() + "]";
        std::string Level = "-" + LogLevelToString(level) + "-";
        std::string CompleteMessage = Time + " " + Level + " " + message;
        if (newline) CompleteMessage += "\n";
        MF::Print::Internal::cout << CompleteMessage;
    }
}