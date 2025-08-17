#pragma once

#include "../Integrity/Validation/ValidateSession.hpp"
#include "../Files/FilesManager.hpp"
#include "../../Outer/Print/Print.hpp"
#include "../../Outer/Global/Configurations/App.hpp"
#include "../../Outer/Global/Configurations/Build.hpp"
#include "../../Outer/Global/Initialization.hpp"
#include "../../Outer/Info/MFWork.h"
#include "../Time&Date/Misc.hpp"
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>

namespace MF::Initializer {
    namespace Internal {
        inline std::string toLowerCopy(std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)  
                { 
                    return static_cast<char>(std::tolower(c)); 
                });
            return s;
        }
    }
    inline bool InitializeMFWork(bool AutoDetermineLogLevel = true) {
        // Start timer
        Time::Timer timer("ms");
        timer.start();
        // Check for critical files
        std::vector<std::string> CriticalFiles = { "App.hc", "Build.hc" };
        for (const auto& file : CriticalFiles) {
            if (!MF::FilesManager::Exists(file)) {
                MF::Print::Out(MF::Print::LogLevel::Error, "Critical file \"" + file + "\" not found!");
                return false;
            }
        }

        // Determine log level
        if (AutoDetermineLogLevel) {
            MF::Configurations::ConfigManager probe;
            if (probe.Load("Build.hc")) {
                std::string channel;
                if (probe.Get("channel", channel)) {
                    channel = Internal::toLowerCopy(channel);
                    // production -> allow debug, otherwise suppress debug
                    if (channel != "production") {
                        MF::Print::CurrentLevel = MF::Print::LogLevel::Debug;
                    } else {
                        MF::Print::CurrentLevel = MF::Print::LogLevel::Info;
                    }
                } else {
                    // can't read channel -> be conservative and suppress debug
                    MF::Print::CurrentLevel = MF::Print::LogLevel::Info;
                }
            } else {
                // can't load build.hc (shouldn't happen because we checked FilesManager::Exists) -> be conservative
                MF::Print::CurrentLevel = MF::Print::LogLevel::Info;
            }
        }

        // Load globals
        if (!Global::Configurations::App::Load()) {
            MF::Print::Out(MF::Print::LogLevel::Error, "Failed to load Global App configuration.");
            return false;
        }

        if (!Global::Configurations::Build::Load()) {
            MF::Print::Out(MF::Print::LogLevel::Error, "Failed to load Global Build configuration.");
            return false;
        }

        // Validate session
        if (!MF::Integrity::ValidateSession(true)) {
            MF::Print::Out(MF::Print::LogLevel::Error, "Failed to validate session!");
            return false;
        }

        // Log build channel
        try {
            std::string BuildChannel;
            Global::Configurations::Build::Config.Get("channel", BuildChannel);
            if (!BuildChannel.empty()) {
                std::string ch = Internal::toLowerCopy(BuildChannel);
                ch[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(ch[0])));
                MF::Print::Out(MF::Print::LogLevel::Info, "Running on " + ch + " channel.");
            }
        } catch (std::exception& e) {
            MF::Print::Out(MF::Print::LogLevel::Error, "Failed to get build channel: " + std::string(e.what()));
            return false;
        }
        MF::Global::Initialized = true;
        MF::Print::Out(MF::Print::LogLevel::Debug, "Assigned MF::Global::Initialized to true.");
        timer.stop();
        MF::Print::Out(MF::Print::LogLevel::Debug, "Initialization complete in " + timer.elapsedString());

        // Alert the user if MFWork's build is unstable
        if (Internal::toLowerCopy(BuildInfo::Channel) != "Production") {
            MF::Print::Out(MF::Print::LogLevel::Warning, "MFWork is running on an unstable build (" + BuildInfo::Channel + ").");
            MF::Print::Out(MF::Print::LogLevel::Warning, "Please report any issues to \"" + BuildInfo::GithubRepo + "issues/\".");
        }
        return true;
    }
}