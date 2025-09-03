#pragma once

#include "../../Global/GlobalDefinitions.hpp"
#include <string>
#include "../../Utils/CoreUtilities.hpp"
#include "../../../Outer/Print/Print.hpp"

namespace MF::Runtime::Session {
    inline bool Validate(bool Exit = false) {
        struct CurrentInfo {
            std::string Architecture = Internal::Utils::CoreUtilities::NormalizeString(Internal::Utils::CoreUtilities::GetArchitecture());
            std::string Platform = Internal::Utils::CoreUtilities::NormalizeString(Internal::Utils::CoreUtilities::GetPlatform());
        } Current;

        Print::Out(Print::LogLevel::Debug, "Current.Platform = \"" + Current.Platform + "\"");
        Print::Out(Print::LogLevel::Debug, "Current.Architecture = \"" + Current.Architecture + "\"");

        bool platformSupported = false;
        for (const auto& Platform : Global::GlobalSettings.Project.App.Support.OperatingSystems) {
            std::string normalized = Internal::Utils::CoreUtilities::NormalizeString(Platform, true);
            Print::Out(Print::LogLevel::Debug, "Found support for platform " + normalized);
            if (normalized == "any" || normalized == Current.Platform) {
                Global::Validation.Platform.Append(normalized == "any" ? Current.Platform : normalized, true);
                platformSupported = true;
                break;
            }
        }

        bool architectureSupported = false;
        for (const auto& Architecture : Global::GlobalSettings.Project.App.Support.Architectures) {
            std::string normalized = Internal::Utils::CoreUtilities::NormalizeString(Architecture, true);
            Print::Out(Print::LogLevel::Debug, "Found support for architecture " + normalized);
            if (normalized == "any" || normalized == Current.Architecture) {
                Global::Validation.Architecture.Append(normalized == "any" ? Current.Architecture : normalized, true);
                architectureSupported = true;
                break;
            }
        }

        if (platformSupported && architectureSupported) return true;

        std::string CapitalizedAppName = Internal::Utils::CoreUtilities::Capitalize(Global::GlobalSettings.Project.App.Name);
        if (!platformSupported) {
            std::string CapitalizedPlatform = Internal::Utils::CoreUtilities::Capitalize(Current.Platform);
            Print::Out(Print::LogLevel::Error, "Platform " + CapitalizedPlatform + " isn't supported on " + CapitalizedAppName + "!");
        }
        if (!architectureSupported) {
            Print::Out(Print::LogLevel::Error, "Architecture " + Current.Architecture + " isn't supported on " + CapitalizedAppName + "!");
        }

        if (Exit) exit(-1);
        return false;
    }
}
