#pragma once

#include <string>
#include "../Configuration/ConfigManager.hpp"

class IntegrityIssue {
public:
    IntegrityIssue(bool _Supported_OperatingSystem, bool _Supported_Architecture, const std::string& _OperatingSystem, const std::string& _Architecture, const MF::Configurations::ConfigManager& _AppConfig)
        : OperatingSystem(_OperatingSystem), Architecture(_Architecture), AppConfig(_AppConfig) {
        Supported.OperatingSystem = _Supported_OperatingSystem;
        Supported.Architecture = _Supported_Architecture;
    }

    struct SupportedFlags { bool OperatingSystem; bool Architecture; } Supported{false,false};

    std::string OperatingSystem;
    std::string Architecture;

    MF::Configurations::ConfigManager AppConfig;
};