#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>

namespace MF::Runtime::Arguments {
    class Parser {
    public:
        void Parse(int argc, char* argv[]) {
            for (int i = 1; i < argc; ++i) {
                std::string arg = argv[i];

                if (arg.rfind("--", 0) == 0) {
                    // --key=value or --key
                    auto pos = arg.find('=');
                    if (pos != std::string::npos) {
                        std::string key = arg.substr(2, pos - 2);
                        std::string value = arg.substr(pos + 1);
                        normalize(key);
                        trimQuotes(value);
                        insertKey(key, value);
                    } else {
                        std::string key = arg.substr(2);
                        normalize(key);
                        insertKey(key, "true");
                    }
                } else if (arg.find('=') != std::string::npos) {
                    // key=value (no leading --)
                    auto pos = arg.find('=');
                    std::string key = arg.substr(0, pos);
                    std::string value = arg.substr(pos + 1);
                    normalize(key);
                    trimQuotes(value);
                    insertKey(key, value);
                } else {
                    // pure positional arg
                    positional_.push_back(arg);
                }
            }
        }

        bool Has(const std::string& key) const {
            std::string norm = key;
            normalize(norm);
            return args_.find(norm) != args_.end();
        }

        std::string Get(const std::string& key, const std::string& def = "") const {
            std::string norm = key;
            normalize(norm);
            auto it = args_.find(norm);
            if (it != args_.end()) return it->second;
            return def;
        }

        const std::vector<std::string>& Positional() const {
            return positional_;
        }

        const std::unordered_map<std::string, std::string>& Dump() const {
            return args_;
        }

    private:
        static void normalize(std::string& s) {
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        }

        static void trimQuotes(std::string& s) {
            if (s.size() >= 2 &&
               ((s.front() == '"' && s.back() == '"') ||
                (s.front() == '\'' && s.back() == '\''))) {
                s = s.substr(1, s.size() - 2);
            }
        }

        void insertKey(const std::string& key, const std::string& value) {
            args_[key] = value;
        }

        std::unordered_map<std::string, std::string> args_;
        std::vector<std::string> positional_;
    };
}
