#pragma once

#include "../../../Outer/Print/Print.hpp"
#include "../IntegrityIssue.hpp"

namespace MF::Integrity {
    inline bool TakeActionOnIntegrityIssue(IntegrityIssue* Issue) {
        try {
            std::string AppName = Issue->AppName;

            if (!Issue->Supported.OperatingSystem) {
                MF::Print::Out(MF::Print::LogLevel::Error, "App " + AppName + " is not supported on " + Issue->OperatingSystem + ".");
            }
            if (!Issue->Supported.Architecture) {
                MF::Print::Out(MF::Print::LogLevel::Error, "App " + AppName + " is not supported on " + Issue->Architecture + ".");
            }
            return true;
        } catch (std::exception& e) {
            MF::Print::Out(MF::Print::LogLevel::Error, "Error while taking action on integrity issue: " + std::string(e.what()));
            return false;
        }
    }
}