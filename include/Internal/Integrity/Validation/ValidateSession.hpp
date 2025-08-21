#pragma once

#include "../../SystemInfo/Architecture/Misc.hpp"
#include "../../SystemInfo/OS/Misc.hpp"
#include "../Action/TakeAction.hpp"
#include "../../../Outer/Print/Print.hpp"
#include "../IntegrityIssue.hpp"
#include "../../Settings/IntSettings.hpp"
#include <string>
#include <vector>
#include <cctype>

// normalize strings for comparison
namespace MF::Integrity {
    static std::string normalize(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (std::isspace(static_cast<unsigned char>(c))) continue;
            if (c == '-') out.push_back('_');
            else out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
        if (out == "amd64" || out == "x64" || out == "x86-64") return "x86_64";
        if (out == "aarch64") return "arm64";
        return out;
    }

    // lenient matching: exact or substring both ways
    static bool fuzzyMatch(const std::string& a, const std::string& b) {
        if (a == b) return true;
        if (a.find(b) != std::string::npos) return true;
        if (b.find(a) != std::string::npos) return true;
        return false;
    }

    inline bool ValidateSession(bool TakeAction = false) {
        if (!InternalSettings::GlobalSettings.Usable) {
            MF::Print::Out(MF::Print::LogLevel::Debug, "User fault while trying to validate session: InternalSettings not setup!");
            exit(1);
            return false;
        }

        std::string CurrentOS = normalize(SystemInfo::OS::GetOSInfo().os);
        std::string CurrentArch = normalize(SystemInfo::Arch::GetStrArchitecture());

        MF::Print::Out(MF::Print::LogLevel::Debug, "Current OS: " + CurrentOS);
        MF::Print::Out(MF::Print::LogLevel::Debug, "Current Arch: " + CurrentArch);

        auto SupportedOperatingSystems = InternalSettings::GlobalSettings.Project.App.Support.OperatingSystems;
        auto SupportedArchitectures = InternalSettings::GlobalSettings.Project.App.Support.Architectures;

        // normalize supported lists
        for (auto& os : SupportedOperatingSystems) os = normalize(os);
        for (auto& arch : SupportedArchitectures) arch = normalize(arch);

        bool os_ok = false;
        bool arch_ok = false;

        for (const auto& s : SupportedOperatingSystems) {
            if (fuzzyMatch(s, CurrentOS)) { os_ok = true; break; }
        }
        for (const auto& s : SupportedArchitectures) {
            if (fuzzyMatch(s, CurrentArch)) { arch_ok = true; break; }
        }

        if (std::find(SupportedOperatingSystems.begin(), SupportedOperatingSystems.end(), "any") != SupportedOperatingSystems.end()) {
            os_ok = true;
        }
        if (std::find(SupportedArchitectures.begin(), SupportedArchitectures.end(), "any") != SupportedArchitectures.end()) {
            arch_ok = true;
        }

        if (os_ok && arch_ok) {
            MF::Print::Out(MF::Print::LogLevel::Info, "Session validation: OK");
            return true;
        }

        if (TakeAction) {
            IntegrityIssue issue(os_ok, arch_ok, CurrentOS, CurrentArch, InternalSettings::GlobalSettings.Project.App.Name);
            MF::Integrity::TakeActionOnIntegrityIssue(&issue);
        } else {
            if (!os_ok) MF::Print::Out(MF::Print::LogLevel::Error, "Not supported OS: " + CurrentOS);
            if (!arch_ok) MF::Print::Out(MF::Print::LogLevel::Error, "Not supported Arch: " + CurrentArch);
        }

        return false;
    }
}