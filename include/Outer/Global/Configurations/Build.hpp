#pragma once

#include "../../../Internal/Configuration/ConfigManager.hpp"
#include "../../Print/Print.hpp"
#include <vector>
#include <exception>
#include <algorithm>

namespace MF::Global {
    namespace Configurations {
        namespace Build {
            inline bool Usable = false;
            inline bool LoadedBefore = false;
            inline MF::Configurations::ConfigManager Config;

            inline bool Load() {
                try {
                    Config.Load("Build.hc");
                } catch (std::exception& e) {
                    MF::Print::Out(MF::Print::LogLevel::Error, "Failed to load Build.hc: " + std::string(e.what()));
                    return false;
                }

                Usable = true;
                MF::Print::Out(
                    MF::Print::LogLevel::Debug,
                    !LoadedBefore ? "Loaded Build.hc successfully." : "Reloaded Build.hc successfully."
                );
                MF::Print::Out(MF::Print::LogLevel::Debug, "Checking if Build.hc is valid...");

                // required fields
                std::vector<std::string> RequiredFields = {
                    "version",
                    "channel"
                };

                for (const auto& field : RequiredFields) {
                    std::string Buffer;
                    if (!Config.Get(field, Buffer)) {
                        MF::Print::Out(MF::Print::LogLevel::Error, "Required field missing: " + field + " at Build.hc");
                        return false;
                    }
                    if (Buffer.empty()) {
                        MF::Print::Out(MF::Print::LogLevel::Error, "Required field is empty: " + field + " at Build.hc");
                        return false;
                    }
                    if (Buffer.find(' ') != std::string::npos) {
                        MF::Print::Out(MF::Print::LogLevel::Error, field + " must not contain spaces at Build.hc!");
                        return false;
                    }
                }

                // validate channel
                {
                    std::string channel;
                    Config.Get("channel", channel);
                    std::transform(channel.begin(), channel.end(), channel.begin(), ::tolower);

                    std::vector<std::string> allowedChannels = { "developing", "unstable", "beta", "production" };
                    bool validChannel = false;

                    for (auto& allowed : allowedChannels) {
                        if (channel == allowed) {
                            validChannel = true;
                            break;
                        }
                    }

                    if (!validChannel) {
                        MF::Print::Out(MF::Print::LogLevel::Error, "Invalid channel in Build.hc: " + channel);
                        return false;
                    }
                }

                MF::Print::Out(MF::Print::LogLevel::Debug, "Build.hc is valid.");
                return true;
            }
        }
    }
}
