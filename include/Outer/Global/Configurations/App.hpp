#pragma once

#include "../../../Internal/Configuration/ConfigManager.hpp"
#include "../../Print/Print.hpp"
#include <vector>
#include <exception>
#include <algorithm>

namespace MF::Global {
    namespace Configurations {
        namespace App {
            inline bool Usable = false;
            inline bool LoadedBefore = false;
            inline MF::Configurations::ConfigManager Config;


            inline bool Load() {
                try {
                    Config.Load("App.hc");
                } catch (std::exception& e) {
                    MF::Print::Out(MF::Print::LogLevel::Error, "Failed to load App.hc: " + std::string(e.what()));
                    return false;
                }

                Usable = true;
                MF::Print::Out(
                    MF::Print::LogLevel::Debug,
                    !LoadedBefore ? "Loaded App.hc successfully." : "Reloaded App.hc successfully."
                );
                MF::Print::Out(MF::Print::LogLevel::Debug, "Checking if App.hc is valid...");

                std::vector<std::string> RequiredFields = {
                    "app",
                    "author",
                    "license",
                    "support",
                    "support.Architecture",
                    "support.OS"
                };

                // check string fields
                for (const auto& field : RequiredFields) {
                    std::string Buffer;
                    if (field == "support" || field == "support.Architecture" || field == "support.OS") break;
                    if (!Config.Get(field, Buffer)) {
                        MF::Print::Out(MF::Print::LogLevel::Error, "Required field missing: " + field + " at App.hc");
                        return false;
                    }
                    if (Buffer.empty()) {
                        MF::Print::Out(MF::Print::LogLevel::Error, "Required field is empty: " + field + " at App.hc");
                        return false;
                    }
                    if ((field == "app" || field == "author") && Buffer.find(' ') != std::string::npos) {
                        MF::Print::Out(MF::Print::LogLevel::Error, field + " must not contain spaces at App.hc!");
                        return false;
                    }
                }

                // check architecture list
                {
                    MF::Configurations::Internal::Parser::HCValue* Architectures = Config.Configuration.get("support.Architecture");
                    if (!Architectures || !Architectures->isList() || Architectures->asList().empty()) {
                        MF::Print::Out(MF::Print::LogLevel::Error, "support.Architecture must have at least one entry at App.hc.");
                        return false;
                    }

                    std::vector<std::string> allowedArch = { "x86_64", "arm64", "arm", "x86" };
                    bool validArchFound = false;

                    for (auto& archVal : Architectures->asList()) {
                        std::string arch = archVal.asString();
                        std::transform(arch.begin(), arch.end(), arch.begin(), ::tolower);
                        for (auto& allowed : allowedArch) {
                            std::string allowedLower = allowed;
                            std::transform(allowedLower.begin(), allowedLower.end(), allowedLower.begin(), ::tolower);
                            if (arch == allowedLower) {
                                validArchFound = true;
                                break;
                            }
                        }
                    }

                    if (!validArchFound) {
                        MF::Print::Out(MF::Print::LogLevel::Error, "No valid architecture found in support.Architecture at App.hc.");
                        return false;
                    }
                }

                // check OS list
                {
                    MF::Configurations::Internal::Parser::HCValue* OperatingSystems = Config.Configuration.get("support.OS");
                    if (!OperatingSystems || !OperatingSystems->isList() || OperatingSystems->asList().empty()) {
                        MF::Print::Out(MF::Print::LogLevel::Error, "support.OS must have at least one entry at App.hc.");
                        return false;
                    }

                    std::vector<std::string> allowedOS = { "linux", "windows", "any" };
                    bool validOSFound = false;

                    for (auto& osVal : OperatingSystems->asList()) {
                        std::string os = osVal.asString();
                        std::transform(os.begin(), os.end(), os.begin(), ::tolower);
                        for (auto& allowed : allowedOS) {
                            std::string allowedLower = allowed;
                            std::transform(allowedLower.begin(), allowedLower.end(), allowedLower.begin(), ::tolower);
                            if (os == allowedLower) {
                                validOSFound = true;
                                break;
                            }
                        }
                    }

                    if (!validOSFound) {
                        MF::Print::Out(MF::Print::LogLevel::Error, "No valid OS found in support.OS at App.hc.");
                        return false;
                    }
                }

                MF::Print::Out(MF::Print::LogLevel::Debug, "App.hc is valid.");
                return true;
            }
        }
    }
}