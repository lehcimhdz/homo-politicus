#include "MiniYaml.hpp"
#include <fstream>
#include <vector>
#include <sstream>

namespace MiniYaml {

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static int indentOf(const std::string& line) {
    int n = 0;
    for (char c : line) {
        if (c == ' ') n++;
        else if (c == '\t') n += 2;
        else break;
    }
    return n;
}

static std::string stripQuotes(const std::string& s) {
    if (s.size() >= 2 && (s.front() == '"' || s.front() == '\'') && s.back() == s.front())
        return s.substr(1, s.size() - 2);
    return s;
}

bool loadFile(const std::string& path, KV& out) {
    std::ifstream f(path);
    if (!f) return false;
    std::vector<std::pair<int, std::string>> stack; // (indent, key)
    std::string line;
    while (std::getline(f, line)) {
        size_t hash = line.find('#');
        if (hash != std::string::npos) line = line.substr(0, hash);
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;
        if (trimmed[0] == '-') continue; // lists not supported
        int indent = indentOf(line);
        auto colon = trimmed.find(':');
        if (colon == std::string::npos) continue;
        std::string key = trim(trimmed.substr(0, colon));
        std::string val = trim(trimmed.substr(colon + 1));
        while (!stack.empty() && stack.back().first >= indent) stack.pop_back();
        std::string full;
        for (const auto& p : stack) {
            if (!full.empty()) full += ".";
            full += p.second;
        }
        if (!full.empty()) full += ".";
        full += key;
        if (val.empty()) {
            stack.push_back({indent, key});
        } else {
            out[full] = stripQuotes(val);
        }
    }
    return true;
}

} // namespace MiniYaml
