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
        struct FileLogging {
            inline static bool Enabled = false;
            inline static std::string HClogPath = "mfwork_logs.hclog";
        };
    };

    class SettingsStack {
    public:
        static inline void Setup
        (
            bool startTimer = Initialization::StartTimer,
            bool allowOverrides = Initialization::AllowOverrides,
            bool checkCriticalFiles = Initialization::CheckCriticalFiles,
            bool parseArguments = Initialization::ParseArguments,
            bool autoDetermineLogLevel = Initialization::AutoDetermineLogLevel,
            bool validateSession = Initialization::ValidateSession,
            bool logBuildChannel = Initialization::LogBuildChannel,
            bool alertOnUnstableChannel = Initialization::AlertOnUnstableChannel,
            Print::LogLevel logLevel = Printing::CurrentLevel,
            bool fileLoggingEnabled = Printing::FileLogging::Enabled,
            const std::string& hclogPath = Printing::FileLogging::HClogPath
        ) 
        {
            Initialization::StartTimer = startTimer;
            Initialization::AllowOverrides = allowOverrides;
            Initialization::CheckCriticalFiles = checkCriticalFiles;
            Initialization::ParseArguments = parseArguments;
            Initialization::AutoDetermineLogLevel = autoDetermineLogLevel;
            Initialization::ValidateSession = validateSession;
            Initialization::LogBuildChannel = logBuildChannel;
            Initialization::AlertOnUnstableChannel = alertOnUnstableChannel;

            Printing::CurrentLevel = logLevel;
            Printing::FileLogging::Enabled = fileLoggingEnabled;
            Printing::FileLogging::HClogPath = hclogPath;
        }
    };
}