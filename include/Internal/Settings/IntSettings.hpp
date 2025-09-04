#pragma once

#include <stdexcept>
#include <string>
#include "IntSettingsStack.hpp"
#include "HCHelper.hpp"
#include "../Global/GlobalDefinitions.hpp"

namespace MF::InternalSettings {

    inline void Setup(SettingsStack *settings) {
        try {
            MF::Global::GlobalSettings = *settings;
        } catch (std::exception &e) {
            throw std::runtime_error("Failed to setup SettingsStack: " + std::string(e.what()));
        }
        MF::Global::GlobalSettings.Usable = true;
    }

    inline void SetupHC(const std::string& filename) {
        Internal::HCHelper helper;

        if (!helper.Load(filename)) {
            throw std::runtime_error("Failed to load settings file: " + filename);
        }

        if (!helper.Map(&MF::Global::GlobalSettings)) {
            throw std::runtime_error("Failed to map settings from file: " + filename);
        }

        MF::Global::GlobalSettings.Usable = true;
    }

}
