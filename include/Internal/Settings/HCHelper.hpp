#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <variant>
#include <vector>
#include <iostream>
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

                // helper for keys
                auto K = [](const std::string& root, const std::string& leaf) {
                    return root.empty() ? leaf : root + "." + leaf;
                };

                //  Initialization Settings
                struct InitField { const char* key; bool& target; };
                InitField initFields[] = {
                    {"StartTimer", settings->Init.StartTimer},
                    {"AllowOverrides", settings->Init.AllowOverrides},
                    {"CheckCriticalFiles", settings->Init.CheckCriticalFiles},
                    {"ParseArguments", settings->Init.ParseArguments},
                    {"AutoDetermineLogLevel", settings->Init.AutoDetermineLogLevel},
                    {"ValidateSession", settings->Init.ValidateSession},
                    {"LogBuildChannel", settings->Init.LogBuildChannel},
                    {"AlertOnUnstableChannel", settings->Init.AlertOnUnstableChannel}
                };

                for (auto& root : std::vector<std::string>{"Initialization", "InitializationSettings"}) {
                    for (auto& f : initFields) {
                        if (CfgMgr.TryGetBool(K(root, f.key), f.target)) continue;
                        // fallback to old if missing
                        f.target = f.target;
                    }
                    // CriticalFiles
                    if (CfgMgr.Get(K(root, "CriticalFiles"), v)) {
                        settings->Init.CriticalFiles.clear();
                        if (std::holds_alternative<MF::Configurations::Internal::Parser::HCList>(v)) {
                            for (auto& item : std::get<MF::Configurations::Internal::Parser::HCList>(v)) {
                                std::string s = item.asString();
                                if (!s.empty() && s != "None") settings->Init.CriticalFiles.push_back(s);
                            }
                        } else if (std::holds_alternative<std::string>(v)) {
                            std::string s = std::get<std::string>(v);
                            if (!s.empty() && s != "None") settings->Init.CriticalFiles.push_back(s);
                        }
                    } else {
                        settings->Init.CriticalFiles = oldSettings.Init.CriticalFiles;
                    }
                }

                // Printing Settings
                if (get_first({"Printing.CurrentLogLevel","Printing.CurrentLevel","Printing.LogLevel"},v))
                    settings->Print.CurrentLevel = parse_loglevel(v, oldSettings.Print.CurrentLevel);
                else settings->Print.CurrentLevel = oldSettings.Print.CurrentLevel;

                if (get_first({"Printing.File.Enabled","Printing.FileLogging.Enabled"},v))
                    settings->Print.File.Enabled = as_bool(v, oldSettings.Print.File.Enabled);
                else settings->Print.File.Enabled = oldSettings.Print.File.Enabled;

                if (get_first({"Printing.File.HClogPath","Printing.FileLogging.HClogPath"},v))
                    settings->Print.File.HClogPath = as_string(v, oldSettings.Print.File.HClogPath);
                else settings->Print.File.HClogPath = oldSettings.Print.File.HClogPath;

                if (get_first({"Printing.Colors.Enabled","Printing.Palette.Enabled"},v))
                    settings->Print.Colors.Enabled = as_bool(v, oldSettings.Print.Colors.Enabled);
                else settings->Print.Colors.Enabled = oldSettings.Print.Colors.Enabled;

                // Project Settings
                auto parseList = [&](const std::string& root, const std::string& key, std::vector<std::string>& out) {
                    const MF::Configurations::Internal::Parser::HCList* lst = nullptr;
                    std::string single;
                    if (CfgMgr.TryGetList(K(root,key), lst)) {
                        out.clear();
                        for (auto& item : *lst) {
                            std::string s = item.asString();
                            if (!s.empty()) out.push_back(s);
                        }
                    } else if (CfgMgr.TryGetString(K(root,key), single)) {
                        out.clear();
                        if (!single.empty()) out.push_back(single);
                    }
                };

                for (auto& root : std::vector<std::string>{"Project","Printing.Project"}) {
                    // App
                    if (CfgMgr.Get(K(root,"App.AppName"),v)) {
                        settings->Project.App.Name = as_string(v, oldSettings.Project.App.Name);
                    }
                    if (CfgMgr.Get(K(root,"App.Author"),v)) {
                        settings->Project.App.Author = as_string(v, oldSettings.Project.App.Author);
                    }
                    if (CfgMgr.Get(K(root,"App.License"),v)) {
                        settings->Project.App.License = as_string(v, oldSettings.Project.App.License);
                    }

                    // Support lists
                    parseList(root,"App.Support.Architectures",settings->Project.App.Support.Architectures);
                    if (settings->Project.App.Support.Architectures.empty())
                        settings->Project.App.Support.Architectures = oldSettings.Project.App.Support.Architectures;

                    parseList(root,"App.Support.OperatingSystems",settings->Project.App.Support.OperatingSystems);
                    if (settings->Project.App.Support.OperatingSystems.empty())
                        settings->Project.App.Support.OperatingSystems = oldSettings.Project.App.Support.OperatingSystems;

                    // Build
                    if (CfgMgr.Get(K(root,"Build.Version"),v)) {
                        settings->Project.Build.Version = as_string(v, oldSettings.Project.Build.Version);
                    }
                    if (CfgMgr.Get(K(root,"Build.Channel"),v)) {
                        settings->Project.Build.Channel = as_string(v, oldSettings.Project.Build.Channel);
                    }
                }

                return true;
            } catch (const std::exception& e) {
                std::cerr << "Failed to map settings via " << CfgMgr.Filename << "\n";
                *settings = oldSettings;
                return false;
            }
        }
    };

}
