#pragma once

namespace MF::SystemInfo::OS {
    enum class OS {
        Windows,
        Linux,
        Unknown
    };

    inline OS getOS() {
        #if defined(_WIN64) || defined(_WIN32)
            return OS::Windows;
        #elif defined(__linux__) || defined(linux)
            return OS::Linux;
        #else
            return OS::Unknown;
        #endif
    }
}