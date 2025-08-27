#pragma once

#include "../../Internal/Print/RawLogging/RawLogging.hpp"
#include "../../Internal/Runtime/Arguments/Global.hpp"
#include "../../Internal/Print/LogLevel.hpp"
#include "../../Internal/Settings/IntSettings.hpp"
#include "../../Internal/Global/GlobalDefinitions.hpp"
#include "../../Internal/Files/FilesManager.hpp"
#include <fstream>
#include <string>

namespace MF::Print {
    inline MF::InternalSettings::SettingsStack::Printing::Palette palette;

    inline void Out(LogLevel level, const std::string& message, bool newline = true) {
        if (static_cast<int>(level) > static_cast<int>(InternalSettings::GlobalSettings.Print.CurrentLevel) || message.empty()) return;

        std::string timeStr = Chrono::Time::GetTimeStr();
        std::string levelStr = LogLevelToString(level);

        if (!InternalSettings::GlobalSettings.Print.Colors.Enabled) {
            std::string completeMessage =
                "[" + timeStr + "] -" + levelStr + "- " + message;

            if (newline) completeMessage += "\n";

            MF::Print::Internal::cout << completeMessage;
        } else {
            MF::InternalSettings::SettingsStack::Printing::Palette::LevelStyle style;
            switch (level) {
                case LogLevel::Debug:   style = palette.Debug; break;
                case LogLevel::Info:    style = palette.Info; break;
                case LogLevel::Warning: style = palette.Warning; break;
                case LogLevel::Error:   style = palette.Error; break;
                default: style = palette.Info; break;
            }

            std::string colorTime    = palette.ColorCode(palette.Time, true);
            std::string colorLevel   = palette.ColorCode(style.color, style.bold);
            std::string colorBracket = palette.ColorCode(palette.Brackets, false);
            std::string colorMessage = palette.ColorCode(palette.Message, false);
            std::string reset        = palette.Reset;

            std::string completeMessage =
                colorBracket + "[" + reset +
                colorTime + timeStr + reset +
                colorBracket + "]" + reset +
                " " +
                colorLevel + "-" + levelStr + "-" + reset +
                " " +
                colorMessage + message + reset;

            if (newline) completeMessage += "\n";

            MF::Print::Internal::cout << completeMessage;
        }

        if (MF::Global::ArgumentParser.Has("mf.print.hclogpath") && InternalSettings::GlobalSettings.Init.AllowOverrides) {
            InternalSettings::GlobalSettings.Print.File.HClogPath = MF::Global::ArgumentParser.Get("mf.print.hclogpath");
        }

        if (InternalSettings::GlobalSettings.Print.File.Enabled && MF::FilesManager::Exists(InternalSettings::GlobalSettings.Print.File.HClogPath)) {
            static bool wroteHeader = false;
            if (!wroteHeader) {
                bool fileHasHeader = false;
                std::ifstream checkFile(InternalSettings::GlobalSettings.Print.File.HClogPath);
                if (checkFile.is_open()) {
                    std::string firstLine;
                    if (std::getline(checkFile, firstLine)) {
                        if (firstLine.find("logs:") == 0) fileHasHeader = true;
                    }
                    checkFile.close();
                }

                std::ofstream logFile(InternalSettings::GlobalSettings.Print.File.HClogPath, std::ios::app);
                if (logFile.is_open()) {
                    if (!fileHasHeader) logFile << "logs:\n";
                    logFile << "    [" << Global::LaunchTime << "]:\n";
                    wroteHeader = true;
                    logFile.close();
                } else {
                    MF::Print::Internal::cout << "[ERROR] Failed to open " << InternalSettings::GlobalSettings.Print.File.HClogPath << "\n";
                    return;
                }
            }

            std::ofstream logFile(InternalSettings::GlobalSettings.Print.File.HClogPath, std::ios::app);
            if (logFile.is_open()) {
                logFile << "        [" << timeStr << "] -" << levelStr << "- " << message << "\n";
                logFile.close();
            } else {
                MF::Print::Internal::cout << "[ERROR] Failed to open " << InternalSettings::GlobalSettings.Print.File.HClogPath << "\n";
            }
        }
    }
}
