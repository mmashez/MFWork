#pragma once

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
            case LogLevel::Debug:   return "DEBUG";
            case LogLevel::Info:    return "INFO";
            case LogLevel::Warning: return "WARNING";
            case LogLevel::Error:   return "ERROR";
        }
        return "UNKNOWN";
    }
}