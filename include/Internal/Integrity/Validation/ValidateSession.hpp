#pragma once

#include "../../SystemInfo/Architecture/Misc.hpp"
#include "../../SystemInfo/OS/Misc.hpp"
#include "../Action/TakeAction.hpp"
#include "../../Configuration/ConfigManager.hpp"
#include "../../../Outer/Print/Print.hpp"
#include "../IntegrityIssue.hpp"
#include "../../../Outer/Global/Configurations/App.hpp" // use the global app config
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
        // ensure global config is loaded and usable
        if (!Global::Configurations::App::Usable) {
            MF::Print::Out(MF::Print::LogLevel::Debug, "Global App configuration not loaded - attempting load...");
            if (!Global::Configurations::App::Load()) {
                MF::Print::Out(MF::Print::LogLevel::Error, "Failed to load global App configuration.");
                return false;
            }
        }

        // reference to the global config manager
        MF::Configurations::ConfigManager& cfg = Global::Configurations::App::Config;

        // load architectures
        MF::Configurations::Internal::Parser::HCValue* Architectures = cfg.Configuration.get("support.Architecture");
        std::vector<std::string> SupportedArchitectures;
        if (Architectures && Architectures->isList()) {
            for (auto& Architecture : Architectures->asList()) {
                std::string arch = normalize(Architecture.asString());
                if (!arch.empty()) {
                    SupportedArchitectures.push_back(arch);
                    MF::Print::Out(MF::Print::LogLevel::Debug, std::string("Found supported architecture: ") + arch);
                }
            }
        } else {
            MF::Print::Out(MF::Print::LogLevel::Debug, "No support.Architecture list found in App configuration.");
        }

        // load operating systems
        MF::Configurations::Internal::Parser::HCValue* OperatingSystems = cfg.Configuration.get("support.OS");
        std::vector<std::string> SupportedOperatingSystems;
        if (OperatingSystems && OperatingSystems->isList()) {
            for (auto& OS : OperatingSystems->asList()) {
                std::string osn = normalize(OS.asString());
                if (!osn.empty()) {
                    SupportedOperatingSystems.push_back(osn);
                    MF::Print::Out(MF::Print::LogLevel::Debug, std::string("Found supported OS: ") + osn);
                }
            }
        } else {
            MF::Print::Out(MF::Print::LogLevel::Debug, "No support.OS list found in App configuration.");
        }

        // current system info
        SystemInfo::OS::OSInfo info = SystemInfo::OS::GetOSInfo();
        std::string currentOS = normalize(info.os);
        std::string currentArch = normalize(SystemInfo::Arch::GetStrArchitecture());

        MF::Print::Out(MF::Print::LogLevel::Debug, std::string("Current OS: ") + currentOS);
        MF::Print::Out(MF::Print::LogLevel::Debug, std::string("Current Arch: ") + currentArch);

        bool os_ok = false;
        bool arch_ok = false;

        for (const auto& s : SupportedOperatingSystems) {
            if (fuzzyMatch(s, currentOS)) { os_ok = true; break; }
        }
        for (const auto& s : SupportedArchitectures) {
            if (fuzzyMatch(s, currentArch)) { arch_ok = true; break; }
        }

        if (os_ok && arch_ok) {
            MF::Print::Out(MF::Print::LogLevel::Info, "Session validation: OK");
            return true;
        }

        if (TakeAction) {
            // pass a copy of the global config manager into the integrity issue (constructor expects ConfigManager)
            IntegrityIssue issue(os_ok, arch_ok, currentOS, currentArch, cfg);
            MF::Integrity::TakeActionOnIntegrityIssue(&issue);
        } else {
            if (!os_ok) MF::Print::Out(MF::Print::LogLevel::Error, std::string("Not supported OS: ") + currentOS);
            if (!arch_ok) MF::Print::Out(MF::Print::LogLevel::Error, std::string("Not supported Arch: ") + currentArch);
        }

        return false;
    }
}