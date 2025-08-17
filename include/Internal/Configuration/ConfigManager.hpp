#pragma once

#include <string>
#include "Parser.hpp"

namespace MF::Configurations {
    class ConfigManager {
    public:
        MF::Configurations::Internal::Parser::HotConfig Configuration;
        std::string Filename;
        bool Loaded = false;

        bool Load(const std::string& filename, bool setFileName = true) {
            if (setFileName) Filename = filename;
            Loaded = Configuration.loadFromFile(Filename, true);
            return Loaded;
        }

        bool Get(const std::string& keyPath, std::string& OutValue) {
            auto val = Configuration.get(keyPath);
            if (!val) return false;
            OutValue = val->asString();
            return true;
        }

        bool Set(const std::string& keyPath, const std::string& value, bool reloadFile = true) {
            bool ok = Configuration.set(keyPath, Internal::Parser::HCValue(value));
            if (!ok) return false;
            if (reloadFile) return Save(Filename, true);
            return true;
        }

        bool Save(const std::string& filename = "", bool reloadAfter = false) {
            std::string fileToUse = filename.empty() ? Filename : filename;
            if (!Configuration.save(fileToUse)) return false;
            if (reloadAfter) return Load(fileToUse);
            return true;
        }
    };
}