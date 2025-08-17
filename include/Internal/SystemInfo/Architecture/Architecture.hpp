#pragma once

namespace MF::SystemInfo::Arch {
    enum class Architecture {
        x86_64,
        arm64,
        Unknown
    };

    inline Architecture GetArchitecture() {
        #ifdef __x86_64__
            return Architecture::x86_64;
        #elif __aarch64__
            return Architecture::arm64;
        #else
            return Architecture::Unknown;
        #endif
    }
}