#pragma once

#include <string>
#include <vector>
#include "../Time&Date/Date/Misc.hpp"
#include "../Time&Date/Time/Misc.hpp"
#include "../Utils/CoreUtilities.hpp"
#include "../Settings/IntSettingsStack.hpp"
#include "../Runtime/Arguments/Parser.hpp"

namespace MF::Global {
    inline static InternalSettings::SettingsStack GlobalSettings;
    inline static int LaunchTime = Chrono::Time::GetRawTime();
    inline static std::string LaunchTimeStr = Chrono::Date::GetDateStr() + " " + Chrono::Time::GetTimeStr();
    inline Runtime::Arguments::Parser ArgumentParser;

    struct ValidationInfo {
        class ArchitectureInfo {
        public:
            void Append(std::string _Arch, bool _Supported) {
                _Arch = Internal::Utils::CoreUtilities::NormalizeString(_Arch, true);
                // append only if not already in list
                for (auto& existing : Architectures)
                    if (existing.first == _Arch) return;
                Architectures.emplace_back(_Arch, _Supported);
            }

            const std::vector<std::pair<std::string, bool>>& GetAll() const { return Architectures; }
            void Reset() { Architectures.clear(); }

        private:
            std::vector<std::pair<std::string, bool>> Architectures;
        } Architecture;

        class PlatformInfo {
        public:
            void Append(std::string _Platform, bool _Supported) {
                _Platform = Internal::Utils::CoreUtilities::NormalizeString(_Platform, true);
                for (auto& existing : Platforms)
                    if (existing.first == _Platform) return;
                Platforms.emplace_back(_Platform, _Supported);
            }

            const std::vector<std::pair<std::string, bool>>& GetAll() const { return Platforms; }
            void Reset() { Platforms.clear(); }

        private:
            std::vector<std::pair<std::string, bool>> Platforms;
        } Platform;
    };

    inline ValidationInfo Validation;
}
