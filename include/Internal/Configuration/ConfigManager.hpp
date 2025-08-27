#pragma once

#include <string>
#include <variant>
#include "Parser.hpp"

using ValType = std::variant<
    std::monostate,
    bool,
    int,
    double,
    std::string,
    MF::Configurations::Internal::Parser::HCMap,
    MF::Configurations::Internal::Parser::HCList
>;

namespace MF::Configurations {
    class ConfigManager {
    public:
        MF::Configurations::Internal::Parser::HotConfig Configuration;
        std::string Filename;
        bool Loaded = false;

        bool Has(const std::string& keyPath) {
            auto val = Configuration.get(keyPath);
            return val != nullptr;
        }

        bool Load(const std::string& filename, bool setFileName = true) {
            Loaded = Configuration.loadFromFile(filename, setFileName);
            if (Loaded && setFileName) Filename = filename;
            return Loaded;
        }

        bool Get(const std::string& keyPath, ValType& OutValue) {
            if (auto val = Configuration.get(keyPath)) {
                OutValue = val->value;
                return true;
            }
            return false;
        }

        bool Set(const std::string& keyPath, const std::string& value, bool reloadFile = true) {
            bool ok = Configuration.set(keyPath, Internal::Parser::HCValue(value));
            if (!ok) return false;
            if (reloadFile) return Save(Filename, true);
            return true;
        }

        bool Set(const std::string& keyPath, bool value, bool reloadFile = true) {
            bool ok = Configuration.set(keyPath, Internal::Parser::HCValue(value));
            if (!ok) return false;
            if (reloadFile) return Save(Filename, true);
            return true;
        }

        bool Set(const std::string& keyPath, int value, bool reloadFile = true) {
            bool ok = Configuration.set(keyPath, Internal::Parser::HCValue(value));
            if (!ok) return false;
            if (reloadFile) return Save(Filename, true);
            return true;
        }

        bool Set(const std::string& keyPath, double value, bool reloadFile = true) {
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

        // legacy wrappers
        bool has(const std::string& keyPath) { return Has(keyPath); }
        bool get(const std::string& keyPath, ValType& OutValue) { return Get(keyPath, OutValue); }
        bool set(const std::string& keyPath, const std::string& value, bool reloadFile = true) {
            return Set(keyPath, value, reloadFile);
        }
        bool save(const std::string& filename = "", bool reloadAfter = false) {
            return Save(filename, reloadAfter);
        }

        // typed getters
        bool TryGetBool(const std::string& keyPath, bool& out) {
            if (auto val = Configuration.get(keyPath)) {
                if (auto p = std::get_if<bool>(&val->value)) {
                    out = *p;
                    return true;
                } else if (auto s = std::get_if<std::string>(&val->value)) {
                    std::string v = Internal::Parser::trim(*s);
                    if (Internal::Parser::iequals(v, "true")) { out = true; return true; }
                    if (Internal::Parser::iequals(v, "false")) { out = false; return true; }
                }
            }
            return false;
        }

        bool GetBool(const std::string& keyPath, bool defaultVal = false) {
            bool out = defaultVal;
            TryGetBool(keyPath, out);
            return out;
        }

        bool TryGetInt(const std::string& keyPath, int& out) {
            if (auto val = Configuration.get(keyPath)) {
                if (auto p = std::get_if<int>(&val->value)) {
                    out = *p;
                    return true;
                } else if (auto s = std::get_if<std::string>(&val->value)) {
                    std::string v = Internal::Parser::trim(*s);
                    try {
                        size_t pos;
                        out = std::stoi(v, &pos);
                        if (pos == v.size()) return true;
                    } catch (...) {}
                }
            }
            return false;
        }

        int GetInt(const std::string& keyPath, int defaultVal = 0) {
            int out = defaultVal;
            TryGetInt(keyPath, out);
            return out;
        }

        bool TryGetDouble(const std::string& keyPath, double& out) {
            if (auto val = Configuration.get(keyPath)) {
                if (auto p = std::get_if<double>(&val->value)) {
                    out = *p;
                    return true;
                } else if (auto s = std::get_if<std::string>(&val->value)) {
                    std::string v = Internal::Parser::trim(*s);
                    try {
                        size_t pos;
                        out = std::stod(v, &pos);
                        if (pos == v.size()) return true;
                    } catch (...) {}
                }
            }
            return false;
        }

        double GetDouble(const std::string& keyPath, double defaultVal = 0.0) {
            double out = defaultVal;
            TryGetDouble(keyPath, out);
            return out;
        }

        bool TryGetString(const std::string& keyPath, std::string& out) {
            if (auto val = Configuration.get(keyPath)) {
                out = val->asString();
                return true;
            }
            return false;
        }

        std::string GetString(const std::string& keyPath, const std::string& defaultVal = "") {
            std::string out = defaultVal;
            TryGetString(keyPath, out);
            return out;
        }

        bool TryGetList(const std::string& keyPath, const Internal::Parser::HCList*& out) {
            if (auto val = Configuration.get(keyPath)) {
                if (val->isList()) {
                    out = &val->asList();
                    return true;
                }
            }
            return false;
        }

        bool TryGetMap(const std::string& keyPath, const Internal::Parser::HCMap*& out) {
            if (auto val = Configuration.get(keyPath)) {
                if (val->isMap()) {
                    out = &val->asMap();
                    return true;
                }
            }
            return false;
        }
    };
}