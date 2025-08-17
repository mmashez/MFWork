#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <fstream>
#include <iostream>
#include <stack>
#include <cctype>
#include "../Files/FilesManager.hpp"

namespace MF::Configurations::Internal::Parser {
    struct HCValue;

    using HCMap = std::unordered_map<std::string, HCValue>;
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
    };

    inline std::string trim(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
        return s.substr(start, end - start);
    }

    inline bool startsWith(const std::string& s, const std::string& prefix) {
        return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
    }

    inline HCValue parseValue(const std::string& val) {
        std::string v = trim(val);
        if (v == "true") return HCValue(true);
        if (v == "false") return HCValue(false);
        try { size_t pos; int i = std::stoi(v, &pos); if (pos == v.size()) return HCValue(i); } catch (...) {}
        try { size_t pos; double d = std::stod(v, &pos); if (pos == v.size()) return HCValue(d); } catch (...) {}
        return HCValue(v);
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
                // skip totally empty lines
                if (rawLine.empty()) continue;

                std::string trimmedLine = trim(rawLine);
                if (trimmedLine.empty()) continue;

                // full-line comment
                if (startsWith(trimmedLine, "#")) {
                    pendingComments.push_back(trimmedLine);
                    continue;
                }

                // indentation count (spaces only)
                int indent = 0;
                while (indent < rawLine.size() && rawLine[indent] == ' ') ++indent;

                // content without leading spaces
                std::string content = rawLine.substr(indent);

                // inline comment
                std::string inlineComment;
                size_t commentPos = content.find('#');
                if (commentPos != std::string::npos) {
                    inlineComment = trim(content.substr(commentPos + 1));
                    content = trim(content.substr(0, commentPos));
                } else {
                    content = trim(content);
                }

                if (content.empty()) {
                    // line had only a comment after indentation
                    if (!inlineComment.empty()) pendingComments.push_back(std::string("# ") + inlineComment);
                    continue;
                }

                // pop contexts until we find parent with smaller indent
                while (!context.empty() && indent <= context.top().indent) context.pop();
                if (context.empty()) return false;
                HCMap* currentMap = context.top().map;

                // list item
                if (startsWith(content, "- ")) {
                    std::string valStr = trim(content.substr(2));
                    HCValue val = parseValue(valStr);
                    val.CommentsBefore = std::move(pendingComments);
                    val.InlineComment = std::move(inlineComment);
                    pendingComments.clear();

                    // find parent context that has a lastKey set and its map contains that key
                    std::stack<Context> tmp = context;
                    HCMap* parentMap = nullptr;
                    std::string parentKey;
                    while (!tmp.empty()) {
                        const Context &c = tmp.top();
                        if (!c.lastKey.empty()) {
                            if (c.map->find(c.lastKey) != c.map->end()) {
                                parentMap = c.map;
                                parentKey = c.lastKey;
                                break;
                            }
                        }
                        tmp.pop();
                    }
                    if (!parentMap) return false;

                    auto it = parentMap->find(parentKey);
                    if (it == parentMap->end()) {
                        // shouldn't happen, but create a list just in case
                        HCList list; list.push_back(std::move(val));
                        (*parentMap)[parentKey] = HCValue(std::move(list));
                    } else {
                        auto &target = it->second;
                        if (target.isList()) {
                            target.asList().push_back(std::move(val));
                        } else if (target.isMap()) {
                            // if map is empty, convert to list (no garbage element)
                            if (target.asMap().empty()) {
                                HCList list;
                                list.push_back(std::move(val));
                                target = HCValue(std::move(list));
                            } else {
                                // it's a non-empty map -> can't convert safely
                                return false;
                            }
                        } else {
                            // scalar -> convert existing scalar into first element of list
                            HCList list;
                            list.push_back(std::move(target));
                            list.push_back(std::move(val));
                            target = HCValue(std::move(list));
                        }
                    }
                } else {
                    // key [: value] case
                    size_t colonPos = content.find(':');
                    if (colonPos == std::string::npos) return false;

                    std::string key = trim(content.substr(0, colonPos));
                    std::string valStr = trim(content.substr(colonPos + 1));

                    // prepare value holder with comments
                    HCValue val;
                    val.CommentsBefore = std::move(pendingComments);
                    val.InlineComment = std::move(inlineComment);
                    pendingComments.clear();

                    if (valStr.empty()) {
                        // placeholder for nested structure (could become map or list)
                        // create an empty map for now (child map), but importantly
                        // tell the parent context that its lastKey is `key` so lists can attach.
                        (*currentMap)[key] = HCValue(HCMap{});
                        // mark parent's lastKey so "- " lines can attach to this key
                        context.top().lastKey = key;
                        // push child context (child map receives potential subkeys)
                        HCMap* childMap = &((*currentMap)[key].asMap());
                        context.push({indent, childMap, ""});
                    } else {
                        // scalar value
                        val = parseValue(valStr);
                        (*currentMap)[key] = std::move(val);
                        // set lastKey in current context so immediate "- " lines (deeper indent)
                        // know which key they may attach to
                        context.top().lastKey = key;
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
                auto it = map->find(key);
                if (it == map->end() || !it->second.isMap()) return nullptr;
                map = &(it->second.asMap());
                pos = dotPos + 1;
            }
            std::string lastKey = keyPath.substr(pos);
            auto it = map->find(lastKey);
            if (it == map->end()) return nullptr;
            return &(it->second);
        }

        bool set(const std::string& keyPath, HCValue newValue) {
            HCMap* map = &root;
            size_t pos = 0, dotPos;
            while ((dotPos = keyPath.find('.', pos)) != std::string::npos) {
                std::string key = keyPath.substr(pos, dotPos - pos);
                auto it = map->find(key);
                if (it == map->end()) { (*map)[key] = HCValue(HCMap{}); it = map->find(key); }
                else if (!it->second.isMap()) it->second = HCValue(HCMap{});
                map = &(it->second.asMap());
                pos = dotPos + 1;
            }
            std::string lastKey = keyPath.substr(pos);
            (*map)[lastKey] = std::move(newValue);
            return true;
        }

        void writeMap(std::ostream& os, const HCMap& map, int indent = 0) const {
            std::string indentStr(indent, ' ');
            for (const auto& [key, val] : map) {
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
    };
}