#pragma once

#include <stdexcept>
#include <string>
#include "IntSettingsStack.hpp"
#include "HCHelper.hpp"

namespace MF::InternalSettings {
    inline SettingsStack GlobalSettings;

    inline void Setup(SettingsStack *settings) {
        try {
            GlobalSettings = *settings;
        } catch (std::exception &e) {
            std::runtime_error("Failed to setup SettingsStack: " + std::string(e.what()));
        }
        GlobalSettings.Usable = true;
    }
    inline void SetupHC(const std::string& filename) {
        Internal::HCHelper helper;
        if (!helper.Load(filename)) {
            throw std::runtime_error("Failed to load settings file: " + filename);
        }
        if (!helper.Map(&GlobalSettings)) {
            throw std::runtime_error("Failed to map settings from file: " + filename);
        }
        GlobalSettings.Usable = true;
    }
}