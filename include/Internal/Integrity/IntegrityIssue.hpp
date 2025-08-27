#pragma once

#include <string>

class IntegrityIssue {
public:
    IntegrityIssue(
        bool _Supported_OperatingSystem,
        bool _Supported_Architecture,
        const std::string& _OperatingSystem,
        const std::string& _Architecture,
        const std::string& _AppName)
        : OperatingSystem(_OperatingSystem),
        Architecture(_Architecture),
        AppName(_AppName)
    {
        Supported.OperatingSystem = _Supported_OperatingSystem;
        Supported.Architecture = _Supported_Architecture;
    }

    struct SupportedFlags { 
        bool OperatingSystem = false; 
        bool Architecture = false; 
    } Supported;

    std::string OperatingSystem;
    std::string Architecture;
    std::string AppName;
};