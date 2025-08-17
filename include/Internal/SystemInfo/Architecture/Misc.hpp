#pragma once

#include "Misc.hpp"
#include <string>

namespace MF::SystemInfo::Arch {
    inline std::string GetStrArchitecture() {
        #ifdef __x86_64__
            return "x86_64";
        #elif __aarch64__
            return "arm64";
        #else
            return "unknown";
        #endif
    }
}