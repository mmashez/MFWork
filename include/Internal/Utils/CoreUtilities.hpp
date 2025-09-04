#pragma once

#include <string>
#include <cctype>
#include <algorithm>
#include <vector>
#ifdef __linux__
#include <sys/utsname.h>
#endif
#ifdef __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#include "../Print/RawLogging/RawLogging.hpp"

namespace MF::Internal::Utils {

    namespace CoreUtilities {
        inline std::string Lowercase(const std::string& s) {
            std::string out = s;
            std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
            return out;
        }

        inline std::string Trim(const std::string& s) {
            auto start = std::find_if_not(s.begin(), s.end(), [](unsigned char c){ return std::isspace(c); });
            auto end   = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c){ return std::isspace(c); }).base();
            return (start < end) ? std::string(start, end) : std::string{};
        }

        inline std::string NormalizeString(std::string s, bool applySysInfo = false) {
            s = Trim(Lowercase(s));
            if (!applySysInfo) return s;

            if (s == "aarch64") s = "arm64";
            else if (s == "aarch32") s = "arm32";
            else if (s == "x86_64" || s == "amd64") s = "x64";
            else if (s == "i386" || s == "i686") s = "x86";
            else if (s == "macos" || s == "apple") s = "mac";
            return s;
        }
    }

    namespace System {

        namespace Rulebook {
            inline std::vector<std::string> ValidArchitectures = {"x64","x86","arm64","arm32"};
            inline std::vector<std::string> ValidPlatforms = {"linux","mac","windows"};
        }

        inline std::string GetArch() {
#ifdef __linux__
            struct utsname buf{};
            if (uname(&buf) == 0) return CoreUtilities::NormalizeString(buf.machine, true);
#elif defined(_WIN32)
            USHORT arch = 0, native = 0;
            if (IsWow64Process2(GetCurrentProcess(), &arch, &native)) {
                switch (arch) {
                    case IMAGE_FILE_MACHINE_AMD64: return "x64";
                    case IMAGE_FILE_MACHINE_I386: return "x86";
                    case IMAGE_FILE_MACHINE_ARM64: return "arm64";
                    case IMAGE_FILE_MACHINE_ARM: return "arm32";
                    default: break;
                }
            }
#elif defined(__APPLE__)
            char buf[256]; size_t size = sizeof(buf);
            if (sysctlbyname("hw.machine", buf, &size, nullptr, 0) == 0)
                return CoreUtilities::NormalizeString(buf, true);
#endif
            return "unknown";
        }

        inline std::string _GetPlatform() {
        #ifdef __linux__
            return "linux";
        #elif defined(_WIN32)
            return "windows";
        #elif defined(__APPLE__)
            return "mac";
        #else
            return "unknown";
        #endif
        }

        inline bool IsArchitectureValid(const std::string& what) {
            std::string n = CoreUtilities::NormalizeString(what, false);
            return std::find(Rulebook::ValidArchitectures.begin(), Rulebook::ValidArchitectures.end(), n)
                   != Rulebook::ValidArchitectures.end();
        }

        inline bool IsPlatformValid(const std::string& what) {
            std::string n = CoreUtilities::NormalizeString(what, false);
            return std::find(Rulebook::ValidPlatforms.begin(), Rulebook::ValidPlatforms.end(), n)
                   != Rulebook::ValidPlatforms.end();
        }

        inline std::string GetArchitectureSafe() {
            std::string arch = GetArch();
            if (System::IsArchitectureValid(arch)) return arch;

            Print::Internal::mf_cout << "Invalid architecture: \"" << arch << "\"\n";
            return "unknown";
        }

        inline std::string GetPlatformSafe() {
            std::string plat = _GetPlatform();
            if (IsPlatformValid(plat)) return plat;
            Print::Internal::mf_cout << "Invalid platform: \"" << plat << "\"\n";
            return "unknown";
        }

        inline bool IsWindows() { return _GetPlatform() == "windows"; }
        inline bool IsLinux()   { return _GetPlatform() == "linux"; }
        inline bool IsMac()     { return _GetPlatform() == "mac"; }

    } // namespace System

    namespace CoreUtilities {

        inline std::string Uppercase(const std::string& s) {
            std::string out = s;
            std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::toupper(c); });
            return out;
        }

        inline std::string Capitalize(const std::string& s) {
            if (s.empty()) return {};
            std::string out = Lowercase(s);
            out[0] = static_cast<char>(std::toupper(out[0]));
            return out;
        }

        inline std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter) {
            std::vector<std::string> output;
            size_t prev = 0, pos;
            while ((pos = str.find(delimiter, prev)) != std::string::npos) {
                if (pos > prev) output.push_back(str.substr(prev, pos - prev));
                prev = pos + delimiter.size();
            }
            if (prev < str.size()) output.push_back(str.substr(prev));
            return output;
        }

        inline std::string JoinStrings(const std::vector<std::string>& vec, const std::string& delimiter) {
            std::string out;
            for (size_t i = 0; i < vec.size(); ++i) {
                out += vec[i];
                if (i + 1 < vec.size()) out += delimiter;
            }
            return out;
        }

        inline bool StartsWith(const std::string& str, const std::string& prefix) {
            return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
        }

        inline bool EndsWith(const std::string& str, const std::string& suffix) {
            return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        inline bool Contains(const std::string& str, const std::string& sub) {
            return str.find(sub) != std::string::npos;
        }

        inline std::string GetArchitecture() {
            try {
                return System::GetArchitectureSafe();
            } catch (const std::exception& e) {
                Print::Internal::mf_cout << "Failed to get architecture: " << e.what() << "\n";
                return "unknown";
            }
        }

        inline std::string GetPlatform() {
            try {
                return System::GetPlatformSafe();
            } catch (const std::exception& e) {
                Print::Internal::mf_cout << "Failed to get platform: " << e.what() << "\n";
                return "unknown";
            }
        }

    } // namespace CoreUtilities

} // namespace MF::Internal::Utils
