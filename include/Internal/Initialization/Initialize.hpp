#pragma once

#include "../Integrity/Validation/ValidateSession.hpp"
#include "../Files/FilesManager.hpp"
#include "../../Outer/Print/Print.hpp"
#include "../../Outer/Global/Initialization.hpp"
#include "../../Outer/Info/MFWork.h"
#include "../Runtime/Arguments/Global.hpp"
#include "../Settings/IntSettings.hpp"
#include "../Time&Date/Misc.hpp"
#include <variant>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>
#include <type_traits>

namespace MF::Initializer {
    namespace Internal {
        inline std::string toLowerCopy(std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { 
                return static_cast<char>(std::tolower(c)); 
            });
            return s;
        }

        inline std::variant<std::monostate, std::string, bool, double, float> ParseValue(const std::string& raw) {
            std::string lower = toLowerCopy(raw);
            if (lower == "true" || lower == "false") return lower == "true";
            try {
                size_t pos = 0;
                double d = std::stod(raw, &pos);
                if (pos == raw.size()) return d;
            } catch (...) {}
            return raw;
        }

        inline std::variant<std::monostate, std::string, bool, double, float> GetOverride(
            std::string What, bool ForceIsolation = true
        ) {
            std::string norm = toLowerCopy(What);

            if (!ForceIsolation) {
                if (MF::Global::ArgumentParser.Has(norm)) {
                    return ParseValue(MF::Global::ArgumentParser.Get(norm));
                }
            }

            std::string pref = norm;
            if (pref.rfind("mf.init.", 0) != 0) {
                pref = "mf.init." + pref;
            }

            if (MF::Global::ArgumentParser.Has(pref)) {
                return ParseValue(MF::Global::ArgumentParser.Get(pref));
            }

            if (!ForceIsolation) {
                if (MF::Global::ArgumentParser.Has(norm)) {
                    return ParseValue(MF::Global::ArgumentParser.Get(norm));
                }
            }

            return std::monostate{};
        }

        inline void applyBoolOverride(const std::string& key, bool& target, bool ForceIsolation = true) {
            auto v = GetOverride(key, ForceIsolation);
            if (std::holds_alternative<std::monostate>(v)) return;
            std::visit([&](auto&& val) {
                using V = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<V, bool>) {
                    target = val;
                } else if constexpr (std::is_same_v<V, std::string>) {
                    std::string lower = toLowerCopy(val);
                    target = (lower == "true" || lower == "1");
                } else if constexpr (std::is_same_v<V, double> || std::is_same_v<V, float>) {
                    target = (val != 0.0);
                }
            }, v);
            MF::Print::Out(MF::Print::LogLevel::Warning, "Applied override \"" + key + "\" -> " + (target ? "true" : "false"));
        }
    }

    inline bool InitializeMFWork(int argc, char* argv[]) {
        Time::Timer timer("ms");
        if (MF::InternalSettings::GlobalSettings.Init.StartTimer) {
            timer.start();
        }

        if (!MF::InternalSettings::GlobalSettings.Usable) {
            MF::Print::Out(MF::Print::LogLevel::Error, "MFWork could not be initialized: InternalSettings not setup!");
            return false;
        }

        if (MF::InternalSettings::GlobalSettings.Init.ParseArguments) {
            MF::Print::Out(MF::Print::LogLevel::Debug, "Parsing arguments...");
            MF::Global::ArgumentParser.Parse(argc, argv);
        }

        if (MF::InternalSettings::GlobalSettings.Init.AllowOverrides) {
            Internal::applyBoolOverride("checkCriticalFiles", MF::InternalSettings::GlobalSettings.Init.CheckCriticalFiles, true);
            Internal::applyBoolOverride("autoDetermineLogLevel", MF::InternalSettings::GlobalSettings.Init.AutoDetermineLogLevel, true);
            Internal::applyBoolOverride("validateSession", MF::InternalSettings::GlobalSettings.Init.ValidateSession, true);
            Internal::applyBoolOverride("logBuildChannel", MF::InternalSettings::GlobalSettings.Init.LogBuildChannel, true);
            Internal::applyBoolOverride("alertOnUnstableChannel", MF::InternalSettings::GlobalSettings.Init.AlertOnUnstableChannel, true);
        }

        if (MF::InternalSettings::GlobalSettings.Init.CheckCriticalFiles) {
            std::vector<std::string> CriticalFiles = MF::InternalSettings::GlobalSettings.Init.CriticalFiles;
            for (const auto& file : CriticalFiles) {
                if (!MF::FilesManager::Exists(file)) {
                    MF::Print::Out(MF::Print::LogLevel::Error, "Critical file \"" + file + "\" not found!");
                    return false;
                }
            }
        }

        if (MF::InternalSettings::GlobalSettings.Init.AutoDetermineLogLevel) {
            MF::Configurations::ConfigManager probe;
            if (probe.Load("Build.hc")) {
                std::string channel = "";
                try {
                    channel = probe.Configuration.get("channel")->asString();
                    channel = Internal::toLowerCopy(channel);
                    if (channel != "production") {
                        MF::InternalSettings::GlobalSettings.Print.CurrentLevel = MF::Print::LogLevel::Debug;
                    } else {
                        MF::InternalSettings::GlobalSettings.Print.CurrentLevel = MF::Print::LogLevel::Info;
                    }
                } catch (std::exception& e) {
                    MF::InternalSettings::GlobalSettings.Print.CurrentLevel = MF::Print::LogLevel::Info;
                    MF::Print::Out(MF::Print::LogLevel::Warning, "Failed to get build channel: " + std::string(e.what()));
                }
            } else {
                MF::InternalSettings::GlobalSettings.Print.CurrentLevel = MF::Print::LogLevel::Info;
                MF::Print::Out(MF::Print::LogLevel::Warning, "Failed to load Build.hc!");
            }
        }

        if (MF::InternalSettings::GlobalSettings.Init.ValidateSession) {
            if (!MF::Integrity::ValidateSession(true)) {
                MF::Print::Out(MF::Print::LogLevel::Error, "Failed to validate session!");
                return false;
            }
        }

        if (MF::InternalSettings::GlobalSettings.Init.LogBuildChannel) {
            try {
                std::string BuildChannel = MF::InternalSettings::GlobalSettings.Project.Build.Channel;
                if (!BuildChannel.empty()) {
                    std::string ch = Internal::toLowerCopy(BuildChannel);
                    ch[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(ch[0])));
                    MF::Print::Out(MF::Print::LogLevel::Info, "Running on " + ch + " channel.");
                }
            } catch (std::exception& e) {
                MF::Print::Out(MF::Print::LogLevel::Error, "Failed to get build channel: " + std::string(e.what()));
                return false;
            }
        }

        MF::Global::Initialized = true;
        MF::Print::Out(MF::Print::LogLevel::Debug, "Assigned MF::Global::Initialized to true.");
        if (MF::InternalSettings::GlobalSettings.Init.StartTimer) {
            timer.stop();
            MF::Print::Out(MF::Print::LogLevel::Debug, "Initialization complete in " + timer.elapsedString());
        }
        else {
            MF::Print::Out(MF::Print::LogLevel::Debug, "Initialization complete.");
        }

        if (Internal::toLowerCopy(BuildInfo::Channel) != "production" && MF::InternalSettings::GlobalSettings.Init.AlertOnUnstableChannel) {
            MF::Print::Out(MF::Print::LogLevel::Warning, "MFWork is running on an unstable build (" + BuildInfo::Channel + ").");
            MF::Print::Out(MF::Print::LogLevel::Warning, "Please report any issues to \"" + BuildInfo::GithubRepo + "issues/\".");
        }
        return true;
    }
}