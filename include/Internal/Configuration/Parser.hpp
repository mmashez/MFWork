#pragma once

#include <string>
#include <variant>
#include <vector>
#include <fstream>
#include <iostream>
#include <stack>
#include <cctype>
#include "../Files/FilesManager.hpp"

namespace MF::Configurations::Internal::Parser {
    struct HCValue;

    using HCMap = std::vector<std::pair<std::string, HCValue>>;
    using HCList = std::vector<HCValue>;

    struct HCValue {
        using ValueType = std::variant<std::monostate, bool, int, double, std::string, HCMap, HCList>;

        ValueType value;
        std::vector<std::string> CommentsBefore;
        std::string InlineComment;

        HCValue() = default;
        HCValue(bool b) : value(b) {}
        HCValue(int i) : value(i) {}
        HCValue(double d) : value(d) {}
        HCValue(std::string s) : value(std::move(s)) {}
        HCValue(HCMap m) : value(std::move(m)) {}
        HCValue(HCList l) : value(std::move(l)) {}

        bool isMap() const { return std::holds_alternative<HCMap>(value); }
        bool isList() const { return std::holds_alternative<HCList>(value); }

        HCMap& asMap() { return std::get<HCMap>(value); }
        const HCMap& asMap() const { return std::get<HCMap>(value); }

        HCList& asList() { return std::get<HCList>(value); }
        const HCList& asList() const { return std::get<HCList>(value); }

        std::string asString() const {
            if (auto pval = std::get_if<std::string>(&value)) return *pval;
            if (auto pbool = std::get_if<bool>(&value)) return *pbool ? "true" : "false";
            if (auto pint = std::get_if<int>(&value)) return std::to_string(*pint);
            if (auto pdbl = std::get_if<double>(&value)) return std::to_string(*pdbl);
            return "";
        }

        std::string getType() const {
            if (std::holds_alternative<std::string>(value)) return "string";
            if (std::holds_alternative<bool>(value)) return "bool";
            if (std::holds_alternative<int>(value)) return "int";
            if (std::holds_alternative<double>(value)) return "double";
            if (std::holds_alternative<HCMap>(value)) return "map";
            if (std::holds_alternative<HCList>(value)) return "list";
            return "unknown";
        }
    };

    inline std::string trim(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
        return s.substr(start, end - start);
    }

    inline bool iequals(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(a[i])) !=
                std::tolower(static_cast<unsigned char>(b[i]))) return false;
        }
        return true;
    }

    inline bool startsWith(const std::string& s, const std::string& prefix) {
        return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
    }

    // fix: check for quotes first, then check for the character outside quotes
    inline size_t findUnquoted(const std::string& s, char ch, size_t start_pos = 0) {
        size_t i = start_pos;
        while (i < s.size()) {
            if (s[i] == '"' || s[i] == '\'') {
                char quote = s[i];
                ++i;
                while (i < s.size() && s[i] != quote) ++i;
                if (i < s.size()) ++i;
            } else {
                if (s[i] == ch) return i;
                ++i;
            }
        }
        return std::string::npos;
    }

    inline HCValue parseValue(const std::string& val) {
        std::string v = trim(val);
        if (v.empty()) return HCValue(std::string(""));
        char first = v[0];
        if ((first == '"' || first == '\'') && v.back() == first && v.size() >= 2) {
            return HCValue(v.substr(1, v.size() - 2));
        }
        if (iequals(v, "true")) return HCValue(true);
        if (iequals(v, "false")) return HCValue(false);
        try {
            size_t pos;
            int i = std::stoi(v, &pos);
            if (pos == v.size()) return HCValue(i);
        } catch (...) {}
        try {
            size_t pos;
            double d = std::stod(v, &pos);
            if (pos == v.size()) return HCValue(d);
        } catch (...) {}
        return HCValue(v);
    }

    inline HCMap::iterator findCaseInsensitive(HCMap& map, const std::string& key) {
        for (auto it = map.begin(); it != map.end(); ++it) {
            if (iequals(it->first, key)) return it;
        }
        return map.end();
    }

    inline HCMap::const_iterator findCaseInsensitive(const HCMap& map, const std::string& key) {
        for (auto it = map.cbegin(); it != map.cend(); ++it) {
            if (iequals(it->first, key)) return it;
        }
        return map.cend();
    }

    struct HotConfig {
        HCMap root;
        std::string filename = "";

        bool loadFromFile(const std::string& _filename, bool setFilename = true) {
            if (!FilesManager::Exists(_filename)) return false;
            std::ifstream file(_filename);
            if (!file.is_open()) return false;
            if (setFilename) filename = _filename;
            return parseLines(file);
        }

        bool parseLines(std::istream& stream) {
            root.clear();
            struct Context { int indent; HCMap* map; std::string lastKey; };
            std::stack<Context> context;
            context.push({-1, &root, ""});

            std::vector<std::string> pendingComments;
            std::string rawLine;

            while (std::getline(stream, rawLine)) {
                if (rawLine.empty()) continue;

                size_t indent = 0;
                while (indent < rawLine.size() && std::isspace(static_cast<unsigned char>(rawLine[indent]))) ++indent;

                std::string content = rawLine.substr(indent);

                size_t commentPos = findUnquoted(content, '#');
                std::string inlineComment;
                if (commentPos != std::string::npos) {
                    inlineComment = trim(content.substr(commentPos + 1));
                    content = content.substr(0, commentPos);
                }

                std::string trimmedContent = trim(content);
                if (trimmedContent.empty()) {
                    if (!inlineComment.empty()) pendingComments.push_back(std::string("# ") + inlineComment);
                    continue;
                }

                if (startsWith(trimmedContent, "#")) {
                    pendingComments.push_back(trimmedContent);
                    continue;
                }

                while (context.size() > 1 && indent <= context.top().indent) context.pop();
                if (context.empty()) return false;
                HCMap* currentMap = context.top().map;

                if (startsWith(content, "-")) {
                    size_t valStart = content.find_first_not_of(" \t", 1);
                    bool isContainerStart = false;
                    std::string valStr;
                    if (valStart == std::string::npos) {
                        isContainerStart = true;
                    } else if (content[valStart] == ':') {
                        isContainerStart = true;
                    } else {
                        valStr = content.substr(valStart);
                        isContainerStart = false;
                    }

                    HCValue val;
                    if (isContainerStart) {
                        val = HCValue(HCMap{});
                    } else {
                        val = parseValue(valStr);
                    }
                    val.CommentsBefore = std::move(pendingComments);
                    val.InlineComment = std::move(inlineComment);
                    pendingComments.clear();

                    std::stack<Context> tmp = context;
                    HCMap* parentMap = nullptr;
                    std::string parentKey;
                    while (!tmp.empty()) {
                        const Context &c = tmp.top();
                        if (!c.lastKey.empty()) {
                            if (findCaseInsensitive(*c.map, c.lastKey) != c.map->end()) {
                                parentMap = c.map;
                                parentKey = c.lastKey;
                                break;
                            }
                        }
                        tmp.pop();
                    }
                    if (!parentMap) return false;

                    auto it = findCaseInsensitive(*parentMap, parentKey);
                    if (it == parentMap->end()) {
                        HCList list;
                        list.push_back(std::move(val));
                        it = parentMap->emplace(parentMap->end(), parentKey, HCValue(std::move(list)));
                    } else {
                        auto &target = it->second;
                        if (target.isList()) {
                            if (isContainerStart) {
                                target.asList().push_back(std::move(val));
                                HCMap* newMap = &target.asList().back().asMap();
                                context.push({static_cast<int>(indent), newMap, ""});
                            } else {
                                target.asList().push_back(std::move(val));
                            }
                        } else if (target.isMap()) {
                            if (!target.asMap().empty()) return false;
                            if (!context.empty() && context.top().map == &target.asMap()) context.pop();
                            HCList list;
                            target = HCValue(std::move(list));
                            if (isContainerStart) {
                                target.asList().push_back(std::move(val));
                                HCMap* newMap = &target.asList().back().asMap();
                                context.push({static_cast<int>(indent), newMap, ""});
                            } else {
                                target.asList().push_back(std::move(val));
                            }
                        } else {
                            HCValue old = std::move(target);
                            HCList list;
                            list.push_back(std::move(old));
                            target = HCValue(std::move(list));
                            if (isContainerStart) {
                                target.asList().push_back(std::move(val));
                                HCMap* newMap = &target.asList().back().asMap();
                                context.push({static_cast<int>(indent), newMap, ""});
                            } else {
                                target.asList().push_back(std::move(val));
                            }
                        }
                    }
                } else {
                    size_t colonPos = findUnquoted(content, ':');
                    if (colonPos == std::string::npos) return false;

                    std::string key = trim(content.substr(0, colonPos));
                    std::string valStr = content.substr(colonPos + 1);

                    HCValue val;
                    val.CommentsBefore = std::move(pendingComments);
                    val.InlineComment = std::move(inlineComment);
                    pendingComments.clear();

                    bool isContainer = trim(valStr).empty();
                    if (isContainer) {
                        val = HCValue(HCMap{});
                    } else {
                        val = parseValue(valStr);
                    }

                    auto it = findCaseInsensitive(*currentMap, key);
                    if (it != currentMap->end()) {
                        it->second = std::move(val);
                    } else {
                        currentMap->emplace_back(key, std::move(val));
                    }

                    context.top().lastKey = key;

                    if (isContainer) {
                        auto lastIt = --currentMap->end();
                        HCMap* childMap = &lastIt->second.asMap();
                        context.push({static_cast<int>(indent), childMap, ""});
                    }
                }
            }

            return true;
        }

        HCValue* get(const std::string& keyPath) {
            HCMap* map = &root;
            size_t pos = 0, dotPos;

            while ((dotPos = keyPath.find('.', pos)) != std::string::npos) {
                std::string key = keyPath.substr(pos, dotPos - pos);
                auto it = findCaseInsensitive(*map, key);
                if (it == map->end() || !it->second.isMap()) return nullptr;
                map = &(it->second.asMap());
                pos = dotPos + 1;
            }

            std::string lastKey = keyPath.substr(pos);
            auto it = findCaseInsensitive(*map, lastKey);
            if (it == map->end()) return nullptr;
            return &(it->second);
        }

        bool set(const std::string& keyPath, HCValue newValue) {
            HCMap* map = &root;
            size_t pos = 0, dotPos;
            while ((dotPos = keyPath.find('.', pos)) != std::string::npos) {
                std::string key = keyPath.substr(pos, dotPos - pos);
                auto it = findCaseInsensitive(*map, key);
                if (it == map->end()) {
                    map->emplace_back(key, HCValue(HCMap{}));
                    it = --map->end();
                } else if (!it->second.isMap()) {
                    it->second = HCValue(HCMap{});
                }
                map = &(it->second.asMap());
                pos = dotPos + 1;
            }
            std::string lastKey = keyPath.substr(pos);
            auto it = findCaseInsensitive(*map, lastKey);
            if (it != map->end()) {
                it->second = std::move(newValue);
            } else {
                map->emplace_back(lastKey, std::move(newValue));
            }
            return true;
        }

        void writeMap(std::ostream& os, const HCMap& map, int indent = 0) const {
            std::string indentStr(indent, ' ');
            for (const auto& p : map) {
                const auto& [key, val] = p;
                for (const auto& c : val.CommentsBefore) os << indentStr << c << "\n";
                if (val.isMap()) {
                    os << indentStr << key << ":\n";
                    writeMap(os, val.asMap(), indent + 2);
                } else if (val.isList()) {
                    os << indentStr << key << ":\n";
                    for (const auto& item : val.asList()) {
                        for (const auto& c : item.CommentsBefore) os << indentStr << "  " << c << "\n";
                        os << indentStr << "  - " << item.asString();
                        if (!item.InlineComment.empty()) os << " # " << item.InlineComment;
                        os << "\n";
                    }
                } else {
                    os << indentStr << key << ": " << val.asString();
                    if (!val.InlineComment.empty()) os << " # " << val.InlineComment;
                    os << "\n";
                }
            }
        }

        bool save(const std::string& _filename = "") const {
            std::string outFile = _filename.empty() ? filename : _filename;
            std::ofstream file(outFile);
            if (!file.is_open()) return false;
            writeMap(file, root);
            return true;
        }

        bool has(const std::string& keyPath) {
            return get(keyPath) != nullptr;
        }
    };
}
