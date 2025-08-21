#pragma once

#include "../../Internal/Print/RawLogging/RawLogging.hpp"
#include "../../Internal/Runtime/Arguments/Global.hpp"
#include "../../Internal/Print/LogLevel.hpp"
#include "../../Internal/Settings/IntSettings.hpp"
#include "../../Internal/Global/GlobalDefinitions.hpp"
#include <fstream>
#include <string>

namespace MF::Print {

    inline void Out(LogLevel level, const std::string& message, bool newline = true) {
        if (static_cast<int>(level) > static_cast<int>(MF::InternalSettings::Printing::CurrentLevel) || message.empty()) return;

        std::string timeStr = "[" + Chrono::Time::GetTimeStr() + "]";
        std::string levelStr = "-" + LogLevelToString(level) + "-";
        std::string completeMessage = timeStr + " " + levelStr + " " + message;
        if (newline) completeMessage += "\n";

        // print to console
        MF::Print::Internal::cout << completeMessage;

        // override log file path if argument exists
        if (MF::Global::ArgumentParser.Has("mf.print.hclogpath")) {
            InternalSettings::Printing::FileLogging::HClogPath = MF::Global::ArgumentParser.Get("mf.print.hclogpath");
        }

        // print to file
        if (InternalSettings::Printing::FileLogging::Enabled) {
            // check if file exists and starts with "logs:"
            static bool wroteHeader = false;
            if (!wroteHeader) {
                bool fileHasHeader = false;
                std::ifstream checkFile(InternalSettings::Printing::FileLogging::HClogPath);
                if (checkFile.is_open()) {
                    std::string firstLine;
                    if (std::getline(checkFile, firstLine)) {
                        if (firstLine.find("logs:") == 0) fileHasHeader = true;
                    }
                    checkFile.close();
                }

                std::ofstream logFile(InternalSettings::Printing::FileLogging::HClogPath, std::ios::app);
                if (logFile.is_open()) {
                    if (!fileHasHeader) logFile << "logs:\n";
                    logFile << "    [" << Global::LaunchTime << "]:\n"; // new section per run
                    wroteHeader = true;
                    logFile.close();
                } else {
                    MF::Print::Internal::cout << "[ERROR] Failed to open " << InternalSettings::Printing::FileLogging::HClogPath << "\n";
                    return;
                }
            }

            // append log line under current launch timestamp
            std::ofstream logFile(InternalSettings::Printing::FileLogging::HClogPath, std::ios::app);
            if (logFile.is_open()) {
                logFile << "        " << completeMessage;
                logFile.close();
            } else {
                MF::Print::Internal::cout << "[ERROR] Failed to open " << InternalSettings::Printing::FileLogging::HClogPath << "\n";
            }
        }
    }
}