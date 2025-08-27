#pragma once

#include <cstdio>
#include <string>

namespace MF::Print {
    namespace Internal {
        inline std::string normalize(const std::string& s) {
            std::string out;
            out.reserve(s.size());
            for (char c : s) {
                if (std::isspace(static_cast<unsigned char>(c))) continue;
                if (c == '-') out.push_back('_');
                else out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            }
            return out;
        }
    }
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
    inline LogLevel StringToLogLevel(std::string level) {
        level = Internal::normalize(level);
        if (level == "DEBUG")   return LogLevel::Debug;
        if (level == "INFO")    return LogLevel::Info;
        if (level == "WARNING") return LogLevel::Warning;
        if (level == "ERROR")   return LogLevel::Error;
        printf("[??:??:??] -WARNING- Unknown log level string '%s', defaulting to INFO.\n", level.c_str());
        return LogLevel::Info; // Default to Info if unknown
    }
}