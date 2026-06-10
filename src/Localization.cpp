#include "Localization.hpp"
#include "MiniYaml.hpp"

namespace Localization {

static MiniYaml::KV g_strings;
static std::string g_lang;

bool load(const std::string& locales_dir, const std::string& lang) {
    MiniYaml::KV kv;
    std::string path = locales_dir + "/" + lang + ".yaml";
    if (!MiniYaml::loadFile(path, kv)) return false;
    g_strings = kv;
    g_lang = lang;
    return true;
}

std::string tr(const std::string& key) {
    auto it = g_strings.find(key);
    if (it == g_strings.end()) return "[missing: " + key + "]";
    return it->second;
}

const std::string& currentLanguage() { return g_lang; }

} // namespace Localization
