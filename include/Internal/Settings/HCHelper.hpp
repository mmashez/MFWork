#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <variant>
#include <vector>
#include "../Configuration/ConfigManager.hpp"
#include "IntSettingsStack.hpp"
#include "../Print/LogLevel.hpp"

namespace MF::InternalSettings::Internal {

    class HCHelper {
    public:
        static std::string normalize(const std::string& s) {
            std::string out = s;
            std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return std::tolower(c); });
            return out;
        }

        static bool as_bool(const ValType& v, bool fallback) {
            if (std::holds_alternative<bool>(v)) return std::get<bool>(v);
            if (std::holds_alternative<int>(v)) return std::get<int>(v) != 0;
            if (std::holds_alternative<double>(v)) return std::get<double>(v) != 0.0;
            if (std::holds_alternative<std::string>(v)) {
                std::string s = normalize(std::get<std::string>(v));
                if (s == "true" || s == "1" || s == "yes" || s == "y" || s == "on") return true;
                if (s == "false" || s == "0" || s == "no" || s == "n" || s == "off") return false;
            }
            return fallback;
        }

        static std::string as_string(const ValType& v, const std::string& fallback = "") {
            if (std::holds_alternative<std::string>(v)) return std::get<std::string>(v);
            if (std::holds_alternative<bool>(v)) return std::get<bool>(v) ? "true" : "false";
            if (std::holds_alternative<int>(v)) return std::to_string(std::get<int>(v));
            if (std::holds_alternative<double>(v)) return std::to_string(std::get<double>(v));
            return fallback;
        }

        static Print::LogLevel parse_loglevel(const ValType& v, Print::LogLevel fallback) {
            if (std::holds_alternative<int>(v)) {
                int n = std::get<int>(v);
                if (n <= 0) return Print::LogLevel::Debug;
                if (n == 1) return Print::LogLevel::Info;
                if (n == 2) return Print::LogLevel::Warning;
                return Print::LogLevel::Error;
            }
            std::string s = normalize(as_string(v));
            if (s == "debug" || s == "dbg" || s == "d" || s == "verbose" || s == "0") return Print::LogLevel::Debug;
            if (s == "info" || s == "i" || s == "log" || s == "1") return Print::LogLevel::Info;
            if (s == "warn" || s == "warning" || s == "w" || s == "2") return Print::LogLevel::Warning;
            if (s == "error" || s == "err" || s == "e" || s == "fatal" || s == "3") return Print::LogLevel::Error;
            return fallback;
        }

        bool get_first(const std::vector<std::string>& keys, ValType& out) {
            for (const auto& k : keys) {
                if (CfgMgr.Get(k, out)) return true;
            }
            return false;
        }

        Configurations::ConfigManager CfgMgr;

        bool Load(const std::string& filename) {
            return CfgMgr.Load(filename);
        }

        bool Map(SettingsStack* settings) {
            if (!settings) return false;

            SettingsStack oldSettings = *settings;
            settings->Reset();

            if (!CfgMgr.Loaded) {
                *settings = oldSettings;
                return false;
            }

            try {
                ValType v;

                // Initialization settings
                const std::vector<std::string> initRoots = {
                    "Initialization", "InitializationSettings",
                    "Printing.Initialization", "Printing.InitializationSettings"
                };
                auto K = [](const std::string& root, const std::string& leaf) {
                    return root.empty() ? leaf : root + "." + leaf;
                };

                for (const auto& root : initRoots) {
                    if (CfgMgr.TryGetBool(K(root, "StartTimer"), settings->Init.StartTimer)) {}
                    else settings->Init.StartTimer = oldSettings.Init.StartTimer;

                    if (CfgMgr.TryGetBool(K(root, "AllowOverrides"), settings->Init.AllowOverrides)) {}
                    else settings->Init.AllowOverrides = oldSettings.Init.AllowOverrides;

                    if (CfgMgr.TryGetBool(K(root, "CheckCriticalFiles"), settings->Init.CheckCriticalFiles)) {}
                    else settings->Init.CheckCriticalFiles = oldSettings.Init.CheckCriticalFiles;

                    if (CfgMgr.TryGetBool(K(root, "ParseArguments"), settings->Init.ParseArguments)) {}
                    else settings->Init.ParseArguments = oldSettings.Init.ParseArguments;

                    if (CfgMgr.TryGetBool(K(root, "AutoDetermineLogLevel"), settings->Init.AutoDetermineLogLevel)) {}
                    else settings->Init.AutoDetermineLogLevel = oldSettings.Init.AutoDetermineLogLevel;

                    if (CfgMgr.TryGetBool(K(root, "ValidateSession"), settings->Init.ValidateSession)) {}
                    else settings->Init.ValidateSession = oldSettings.Init.ValidateSession;

                    if (CfgMgr.TryGetBool(K(root, "LogBuildChannel"), settings->Init.LogBuildChannel)) {}
                    else settings->Init.LogBuildChannel = oldSettings.Init.LogBuildChannel;

                    if (CfgMgr.TryGetBool(K(root, "AlertOnUnstableChannel"), settings->Init.AlertOnUnstableChannel)) {}
                    else settings->Init.AlertOnUnstableChannel = oldSettings.Init.AlertOnUnstableChannel;

                    if (CfgMgr.Get(K(root, "CriticalFiles"), v)) {
                        settings->Init.CriticalFiles.clear();
                        if (std::holds_alternative<MF::Configurations::Internal::Parser::HCList>(v)) {
                            const auto& lst = std::get<MF::Configurations::Internal::Parser::HCList>(v);
                            for (const auto& item : lst) {
                                std::string s = normalize(item.asString());
                                if (s == "none") {
                                    settings->Init.CriticalFiles.clear();
                                    break;
                                }
                                if (!s.empty()) settings->Init.CriticalFiles.push_back(s);
                            }
                        } else {
                            std::string s = normalize(as_string(v));
                            if (s != "none" && !s.empty()) settings->Init.CriticalFiles.push_back(s);
                        }
                    } else {
                        settings->Init.CriticalFiles = oldSettings.Init.CriticalFiles;
                    }
                }

                // Printing settings
                if (get_first({"Printing.CurrentLogLevel", "Printing.CurrentLevel", "Printing.LogLevel"}, v)) {
                    settings->Print.CurrentLevel = parse_loglevel(v, oldSettings.Print.CurrentLevel);
                } else {
                    settings->Print.CurrentLevel = oldSettings.Print.CurrentLevel;
                }

                if (get_first({"Printing.File.Enabled", "Printing.FileLogging.Enabled"}, v)) {
                    settings->Print.File.Enabled = as_bool(v, oldSettings.Print.File.Enabled);
                } else {
                    settings->Print.File.Enabled = oldSettings.Print.File.Enabled;
                }

                if (get_first({"Printing.File.HClogPath", "Printing.FileLogging.HClogPath"}, v)) {
                    settings->Print.File.HClogPath = as_string(v, oldSettings.Print.File.HClogPath);
                } else {
                    settings->Print.File.HClogPath = oldSettings.Print.File.HClogPath;
                }

                if (get_first({"Printing.Colors.Enabled", "Printing.Palette.Enabled"}, v)) {
                    settings->Print.Colors.Enabled = as_bool(v, oldSettings.Print.Colors.Enabled);
                } else {
                    settings->Print.Colors.Enabled = oldSettings.Print.Colors.Enabled;
                }

                // Project settings
                const std::vector<std::string> projectRoots = {"Project", "Printing.Project"};

                for (const auto& root : projectRoots) {
                    std::vector<std::string> appNameKeys = {
                        K(root, "App.AppName"), K(root, "App.Name"),
                        K(root, "App.appname"), K(root, "App.name")
                    };
                    for (const auto& key : appNameKeys) {
                        if (CfgMgr.Get(key, v) && std::holds_alternative<std::string>(v)) {
                            settings->Project.App.Name = std::get<std::string>(v);
                            break;
                        }
                    }
                    if (settings->Project.App.Name.empty()) {
                        settings->Project.App.Name = oldSettings.Project.App.Name;
                    }

                    if (CfgMgr.Get(K(root, "App.Author"), v)) {
                        settings->Project.App.Author = as_string(v, oldSettings.Project.App.Author);
                    } else {
                        settings->Project.App.Author = oldSettings.Project.App.Author;
                    }

                    if (CfgMgr.Get(K(root, "App.License"), v)) {
                        settings->Project.App.License = as_string(v, oldSettings.Project.App.License);
                    } else {
                        settings->Project.App.License = oldSettings.Project.App.License;
                    }

                    if (CfgMgr.Get(K(root, "App.Support.Architectures"), v)) {
                        settings->Project.App.Support.Architectures.clear();
                        if (std::holds_alternative<MF::Configurations::Internal::Parser::HCList>(v)) {
                            const auto& lst = std::get<MF::Configurations::Internal::Parser::HCList>(v);
                            bool anyFlag = false;
                            for (const auto& item : lst) {
                                std::string s = normalize(item.asString());
                                if (s == "any") {
                                    anyFlag = true;
                                    break;
                                }
                                if (!s.empty()) settings->Project.App.Support.Architectures.push_back(s);
                            }
                            if (anyFlag) settings->Project.App.Support.Architectures = {"any"};
                        } else {
                            std::string s = normalize(as_string(v));
                            if (s == "any") settings->Project.App.Support.Architectures = {"any"};
                            else if (!s.empty()) settings->Project.App.Support.Architectures.push_back(s);
                        }
                    } else {
                        settings->Project.App.Support.Architectures = oldSettings.Project.App.Support.Architectures;
                    }

                    if (CfgMgr.Get(K(root, "App.Support.OperatingSystems"), v)) {
                        settings->Project.App.Support.OperatingSystems.clear();
                        if (std::holds_alternative<MF::Configurations::Internal::Parser::HCList>(v)) {
                            const auto& lst = std::get<MF::Configurations::Internal::Parser::HCList>(v);
                            bool anyFlag = false;
                            for (const auto& item : lst) {
                                std::string s = normalize(item.asString());
                                if (s == "any") {
                                    anyFlag = true;
                                    break;
                                }
                                if (!s.empty()) settings->Project.App.Support.OperatingSystems.push_back(s);
                            }
                            if (anyFlag) settings->Project.App.Support.OperatingSystems = {"any"};
                        } else {
                            std::string s = normalize(as_string(v));
                            if (s == "any") settings->Project.App.Support.OperatingSystems = {"any"};
                            else if (!s.empty()) settings->Project.App.Support.OperatingSystems.push_back(s);
                        }
                    } else {
                        settings->Project.App.Support.OperatingSystems = oldSettings.Project.App.Support.OperatingSystems;
                    }

                    if (CfgMgr.Get(K(root, "Build.Version"), v)) {
                        settings->Project.Build.Version = as_string(v, oldSettings.Project.Build.Version);
                    } else {
                        settings->Project.Build.Version = oldSettings.Project.Build.Version;
                    }

                    if (CfgMgr.Get(K(root, "Build.Channel"), v)) {
                        settings->Project.Build.Channel = normalize(as_string(v, oldSettings.Project.Build.Channel));
                    } else {
                        settings->Project.Build.Channel = oldSettings.Project.Build.Channel;
                    }
                }

                return true;
            } catch (const std::exception& e) {
                // Optionally log the error for debugging
                // std::cerr << "Configuration mapping failed: " << e.what() << std::endl;
                *settings = oldSettings;
                return false;
            }
        }
    };

}