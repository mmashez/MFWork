#pragma once

#include <string>
#include <vector>
#include "../Print/LogLevel.hpp"

namespace MF::InternalSettings {

    struct SettingsStack {
        bool Usable = false;
        struct Initialization {
            bool StartTimer = true;
            bool AllowOverrides = false;
            std::vector<std::string> CriticalFiles = {};
            bool CheckCriticalFiles = true;
            bool ParseArguments = true;
            bool AutoDetermineLogLevel = true;
            bool ValidateSession = true;
            bool LogBuildChannel = true;
            bool AlertOnUnstableChannel = true;
        };

        struct Printing {
            Print::LogLevel CurrentLevel = Print::LogLevel::Info;
            struct FileLogging {
                bool Enabled = false;
                std::string HClogPath = "mfwork_logs.hclog";
            } File;
        };

        struct Project {
            struct App {
                std::string Name;
                std::string Author;
                std::string License;
                struct Support {
                    std::vector<std::string> Architectures;
                    std::vector<std::string> OperatingSystems;
                } Support;
            } App;

            struct Build {
                std::string Version;
                std::string Channel;
            } Build;
        };

        Initialization Init;
        Printing Print;
        Project Project;

        void Setup(const SettingsStack& custom) {
            *this = custom;
            Usable = true;
        }
    };

    // global access
    inline SettingsStack GlobalSettings;

}