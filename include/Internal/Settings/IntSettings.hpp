#pragma once

#include <string>
#include <vector>
#include <cstdlib>
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

            struct Palette {
                struct RGB { int r, g, b; };
                struct LevelStyle { RGB color; bool bold; };

                RGB Time      = {128, 0, 128};
                RGB Brackets  = {255, 255, 255};
                RGB Message   = {255, 255, 255};

                LevelStyle Debug   = {{128, 128, 255}, true};
                LevelStyle Info    = {{160, 160, 160}, true};
                LevelStyle Warning = {{255, 255, 0}, true};
                LevelStyle Error   = {{255, 0, 0}, true};

                std::string Reset    = "\033[0m";
                std::string BoldCode = "\033[1m";

                std::string ColorCode(const RGB& c, bool bold = false) const {
                    std::string s = bold ? BoldCode : "";
                    if (SupportsTrueColor()) {
                        s += "\033[38;2;" + std::to_string(c.r) + ";" + std::to_string(c.g) + ";" + std::to_string(c.b) + "m";
                    } else if (SupportsBasicColor()) {
                        if (c.r > 200 && c.g < 50 && c.b < 50) s += "\033[31m";
                        else if (c.r > 200 && c.g > 200 && c.b < 50) s += "\033[33m";
                        else if (c.r < 50 && c.g < 50 && c.b > 200) s += "\033[34m";
                        else if (c.r > 200 && c.g < 200 && c.b > 200) s += "\033[35m";
                        else s += "\033[37m";
                    }
                    return s;
                }

                static bool SupportsTrueColor() {
                    const char* colorterm = std::getenv("COLORTERM");
                    return colorterm && (std::string(colorterm) == "truecolor" || std::string(colorterm) == "24bit");
                }

                static bool SupportsBasicColor() {
                    const char* term = std::getenv("TERM");
                    return term && (std::string(term).find("xterm") != std::string::npos || std::string(term).find("screen") != std::string::npos);
                }
            } Colors;
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

    inline SettingsStack GlobalSettings;

}