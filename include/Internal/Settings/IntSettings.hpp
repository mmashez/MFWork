#pragma once

#include <string>
#include <vector>
#include "../Print/LogLevel.hpp"

namespace MF::InternalSettings {
    struct Initialization {
        inline static bool StartTimer = true;
        inline static bool AllowOverrides = false;
        inline static std::vector<std::string> CriticalFiles = {"App.hc", "Build.hc"};
        inline static bool CheckCriticalFiles = true;
        inline static bool ParseArguments = true;
        inline static bool AutoDetermineLogLevel = true;
        struct Globals {
            inline static bool App = true;
            inline static bool Build = true;
        };
        inline static bool ValidateSession = true;
        inline static bool LogBuildChannel = true;
        inline static bool AlertOnUnstableChannel = true;
    };
    struct Printing {
        inline static Print::LogLevel CurrentLevel = Print::LogLevel::Info;
        inline static std::string HClogPath = "mfwork_logs.hclog";
    };
}