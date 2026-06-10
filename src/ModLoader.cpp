#include "ModLoader.hpp"
#include "MiniYaml.hpp"
#include <filesystem>
#include <unordered_set>

namespace ModLoader {

namespace fs = std::filesystem;

std::vector<ModMetadata> scan(const std::string& modsDir) {
    std::vector<ModMetadata> out;
    if (!fs::exists(modsDir) || !fs::is_directory(modsDir)) return out;
    for (const auto& entry : fs::directory_iterator(modsDir)) {
        if (!entry.is_directory()) continue;
        std::string modYaml = entry.path().string() + "/mod.yaml";
        if (!fs::exists(modYaml)) continue;
        MiniYaml::KV kv;
        if (!MiniYaml::loadFile(modYaml, kv)) continue;
        ModMetadata m;
        auto getS = [&](const std::string& k, const std::string& d = "") {
            auto it = kv.find(k); return it == kv.end() ? d : it->second;
        };
        m.id = getS("id");
        if (m.id.empty()) continue;
        m.name = getS("name", m.id);
        m.author = getS("author", "Anonymous");
        m.version = getS("version", "0.1");
        m.description = getS("description", "");
        // dependencies: ignorado por ahora, solo string única
        std::string dep = getS("dependencies");
        if (!dep.empty()) m.dependencies.push_back(dep);
        out.push_back(m);
    }
    return out;
}

bool validateNoConflicts(const std::vector<ModMetadata>& mods) {
    std::unordered_set<std::string> seen;
    for (const auto& m : mods) {
        if (!seen.insert(m.id).second) return false;
    }
    return true;
}

std::vector<std::string> missingDependencies(const std::vector<ModMetadata>& mods) {
    std::unordered_set<std::string> available;
    for (const auto& m : mods) available.insert(m.id);
    std::vector<std::string> missing;
    for (const auto& m : mods) {
        for (const auto& d : m.dependencies) {
            if (!available.count(d)) missing.push_back(d);
        }
    }
    return missing;
}

}
