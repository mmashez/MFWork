#pragma once

#include <string>
#include <fstream>
#include <unordered_map>
#include <sys/utsname.h>
#include "OS.hpp"

struct OSInfo {
    std::string os;             // "linux", "windows", "Unknown"
    std::string distro;         // distribution name
    std::string major_release;  // major version number
    std::string version;        // full version string
    std::string kernel_version; // kernel version string
};

std::string OSToString(OS os) {
    switch (os) {
    case OS::Windows:
        return "Windows";
    case OS::Linux:
        return "Linux";
    default:
        return "Unknown";
    }
}

inline OSInfo GetOSInfo() {
    OSInfo info;

    struct utsname buf;
    if (uname(&buf) == 0) {
        info.kernel_version = buf.release;
        info.os = buf.sysname;

        std::ifstream file("/etc/os-release");
        if (file) {
            std::unordered_map<std::string, std::string> kv;
            std::string line;
            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;
                auto eqPos = line.find('=');
                if (eqPos != std::string::npos) {
                    std::string key = line.substr(0, eqPos);
                    std::string value = line.substr(eqPos + 1);
                    if (!value.empty() && value.front() == '"' && value.back() == '"') {
                        value = value.substr(1, value.size() - 2);
                    }
                    kv[key] = value;
                }
            }
            info.os = OSToString(getOS());
            
            if (kv.count("ID")) info.distro = kv["ID"];
            else info.distro = "undetermined-linux";

            if (kv.count("VERSION_ID")) info.major_release = kv["VERSION_ID"];
            else info.major_release = "unknown";

            if (kv.count("PRETTY_NAME")) info.version = kv["PRETTY_NAME"];
            else info.version = info.os + " " + info.major_release;
        } else {
            info.major_release = "unknown";
            info.version = info.os;
        }
    } else {
        info.os = "unknown";
        info.major_release = "unknown";
        info.version = "unknown";
        info.kernel_version = "unknown";
    }

    return info;
}

inline std::string GetOSName(bool DetermineByDefines = false) {
    if (!DetermineByDefines) return GetOSInfo().os;
    return OSToString(getOS());
}