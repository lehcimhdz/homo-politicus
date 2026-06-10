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

// Detecta si una línea inicia un item de lista: indentación X seguida de "- "
// Devuelve true e indentación del marker en `outIndent` si es item de lista.
static bool isListItemStart(const std::string& line, int& outIndent, std::string& restAfterDash) {
    int n = 0;
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == ' ') { n++; continue; }
        if (line[i] == '-' && i + 1 < line.size() && line[i + 1] == ' ') {
            outIndent = n;
            restAfterDash = line.substr(i + 2);
            return true;
        }
        return false;
    }
    return false;
}

bool loadList(const std::string& path, const std::string& rootKey,
              std::vector<KV>& items) {
    std::ifstream f(path);
    if (!f) return false;
    std::string line;
    bool inRoot = false;
    int rootIndent = -1;
    int listIndent = -1;
    KV current;
    bool haveCurrent = false;
    std::vector<std::pair<int, std::string>> stack;

    auto flushCurrent = [&]() {
        if (haveCurrent && !current.empty()) items.push_back(current);
        current.clear();
        haveCurrent = false;
        stack.clear();
    };

    auto absorbKeyValue = [&](int indent, const std::string& trimmed) {
        auto colon = trimmed.find(':');
        if (colon == std::string::npos) return;
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
            current[full] = stripQuotes(val);
        }
    };

    while (std::getline(f, line)) {
        // Remove inline comments only after first non-space
        size_t hash = line.find(" #");
        if (hash != std::string::npos) line = line.substr(0, hash);
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;

        int indent = indentOf(line);
        int dashIndent;
        std::string afterDash;
        bool startsItem = isListItemStart(line, dashIndent, afterDash);

        if (!inRoot) {
            auto colon = trimmed.find(':');
            if (colon == std::string::npos) continue;
            std::string key = trim(trimmed.substr(0, colon));
            std::string val = trim(trimmed.substr(colon + 1));
            if (key == rootKey && val.empty()) {
                inRoot = true;
                rootIndent = indent;
            }
            continue;
        }

        // Salida del bloque root
        if (!startsItem && indent <= rootIndent && listIndent != -1) {
            flushCurrent();
            inRoot = false;
            continue;
        }

        if (startsItem) {
            if (listIndent == -1) listIndent = dashIndent;
            flushCurrent();
            haveCurrent = true;
            // Contenido en la misma línea: "  - id: foo"
            std::string after = trim(afterDash);
            if (!after.empty()) {
                int contentIndent = dashIndent + 2;
                absorbKeyValue(contentIndent, after);
            }
        } else if (haveCurrent && indent > listIndent) {
            absorbKeyValue(indent, trimmed);
        }
    }
    flushCurrent();
    return true;
}

} // namespace MiniYaml
