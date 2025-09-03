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
        inline std::string NormalizeString(std::string What, bool ImplementSysInfo = false) {
            // lowercase
            std::transform(What.begin(), What.end(), What.begin(),
                           [](unsigned char c){ return std::tolower(c); });

            // trim leading/trailing whitespace
            What.erase(What.begin(), std::find_if(What.begin(), What.end(),
                                                  [](unsigned char c){ return !std::isspace(c); }));
            What.erase(std::find_if(What.rbegin(), What.rend(),
                                    [](unsigned char c){ return !std::isspace(c); }).base(), What.end());

            // optional system-specific normalization
            if (ImplementSysInfo) {
                if (What == "aarch64") What = "arm64";
                else if (What == "aarch32") What = "arm32";
                else if (What == "x86_64" || What == "amd64") What = "x64";
                else if (What == "i386" || What == "i686") What = "x86";
                else if (What == "macos" || What == "apple") What = "mac";
            }

            return What;
        }
    }
    namespace System {
        namespace Rulebook {
            inline std::vector<std::string> ValidArchitectures = {
                "x64",
                "x86",
                "arm64",
                "arm32",
            };
            inline std::vector<std::string> ValidPlatforms = {
                "linux",
                "mac",
                "windows"
            };
        }
        inline bool IsArchitectureValid(std::string What) {
            What = CoreUtilities::NormalizeString(What, false);
            for (const auto& Architecture : Rulebook::ValidArchitectures) {
                if (What == Architecture) {
                    return true;
                    break;
                }
            }
            return false;
        }
        inline bool IsPlatformValid(std::string What) {
            What = CoreUtilities::NormalizeString(What);
            for (const auto& Platform : Rulebook::ValidPlatforms) {
                if (What == Platform) {
                    return true;
                    break;
                }
            }
            return false;
        }
        inline std::string GetArch() {
            // for linux platforms
            #ifdef __linux__
                // try uname first
                struct utsname buf;
                if (uname(&buf) == 0) {
                    std::string arch = buf.machine;
                    if (arch == "x86_64") return "x64";
                    if (arch == "i686" || arch == "i386") return "x86";
                    if (arch == "aarch64") return "arm64";
                    if (arch.find("arm") != std::string::npos) return "arm32";
                    return arch; // fallback, raw string
                }
                return "unknown";

            // for windows platforms
            #elif defined(_WIN32)
            USHORT arch = 0;
            USHORT nativeMachine = 0;
            if (IsWow64Process2(GetCurrentProcess(), &arch, nativeMachine) {
                switch (arch) {
                    case IMAGE_FILE_MACHINE_AMD64: return "x64";
                    case IMAGE_FILE_MACHINE_I386: return "x86";
                    case IMAGE_FILE_MACHINE_ARM64: return "arm64";
                    case IMAGE_FILE_MACHINE_ARM: return "arm32";
                    default: return "unknown";
                }
            }
            return "unknown";

            // for macOS platforms
            #elif defined(__APPLE__)
            char buf[256];
            size_t size = sizeof(buf);
            if (sysctlbyname("hw.machine", buf, &size, nullptr, 0) == 0) return std::string(buf);
            return "unknown";

            // fallback
            #elif __x86_64__
            return "x64";
            #elif defined(__i386__) || defined(__i686__)
            return "x86";
            #elif __aarch64__
            return "arm64";
            #elif __arm__
            return "arm32";
            #else
            return "unknown";
            #endif
        }

        // compile-time OS detection helpers
        inline bool IsWindows() {
            #ifdef _WIN32
            return true;
            #else
            return false;
            #endif
        }

        inline bool IsLinux() {
            #ifdef __linux__
            return true;
            #else
            return false;
            #endif
        }

        inline bool IsMac() {
            #ifdef __APPLE__
            return true;
            #else
            return false;
            #endif
        }

        inline std::string _GetPlatform() {
            #ifdef __linux__
                return "linux";
            #elif defined(__APPLE__)
                return "mac";
            #elif defined(_WIN32)
                return "windows";
            #else
                return "unknown";
            #endif
        }
    }

    namespace CoreUtilities {
        // String utilities:

        inline std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter) {
            std::vector<std::string> Output;
            size_t pos = 0, prev = 0;

            while ((pos = str.find(delimiter, prev)) != std::string::npos) {
                if (pos > prev) {
                    Output.push_back(str.substr(prev, pos - prev));
                }
                prev = pos + delimiter.size();
            }

            if (prev < str.size()) {
                Output.push_back(str.substr(prev));
            }

            return Output;
        }

        inline std::string JoinStrings(const std::vector<std::string>& vec, const std::string& delimiter) {
            std::string Output;
            for (size_t i = 0; i < vec.size(); ++i) {
                Output += vec[i];
                if (i + 1 < vec.size()) Output += delimiter;
            }
            return Output;
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

        inline std::string Capitalize(const std::string& str) {
            if (str.empty()) return str;
            std::string result = str;
            // first make everything lowercase
            std::transform(result.begin(), result.end(), result.begin(),
                           [](unsigned char c){ return std::tolower(c); });
            result[0] = static_cast<char>(std::toupper(result[0]));
            return result;
        }

        inline std::string Lowercase(const std::string& str) {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(),
                           [](unsigned char c){ return std::tolower(c); });
            return result;
        }

        inline std::string Uppercase(const std::string& str) {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(),
                           [](unsigned char c){ return std::toupper(c); });
            return result;
        }

        // System utilities/helpers

        inline std::string GetArchitecture() {
            bool IsResultValid = false;
            std::string Result = "";
            try {
                Result = NormalizeString(System::GetArch(), true);
                for (const std::string Arch : System::Rulebook::ValidArchitectures) {
                    if (Result == Arch) {
                        IsResultValid = true;
                        break;
                    }
                }
            } catch (std::exception &e) {
                Print::Internal::cout << "Failed to get valid architecture!: " << e.what() << "\n";
            }
            if (IsResultValid && !Result.empty()) {
                return Result;
            }
            Print::Internal::cout << "Failed to get valid architecture!: Invalid architecture (\"" << Result << "\")\n";
            return "unknown";
        }

        inline std::string GetPlatform() {
            bool IsResultValid = false;
            std::string Result = "";
            try {
                Result = System::_GetPlatform();
            } catch (std::exception &e) {
                Print::Internal::cout << "Failed to get valid platform!: " << e.what();
            }
            for (const auto& Platform : System::Rulebook::ValidPlatforms) {
                if (Result == Platform) {
                    IsResultValid = true;
                    break;
                }
            }
            if (IsResultValid && !Result.empty()) {
                return Result;
            }
            Print::Internal::cout << "Failed to get valid platform!: Invalid platform (\"" << Result << "\")\n";
            return "unknown";
        }
    }
}
