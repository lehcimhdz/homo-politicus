#include "ui/AssetManager.hpp"

AssetManager& AssetManager::instance() {
    static AssetManager m;
    return m;
}

bool AssetManager::loadTexture(const std::string& key, const std::string& path) {
    auto tex = std::make_unique<sf::Texture>();
    if (!tex->loadFromFile(path)) return false;
    tex->setSmooth(true);
    textures_[key] = std::move(tex);
    return true;
}

bool AssetManager::loadTexture(const std::string& key, const std::vector<std::string>& paths) {
    for (const auto& p : paths) {
        if (loadTexture(key, p)) return true;
    }
    return false;
}

const sf::Texture* AssetManager::getTexture(const std::string& key) const {
    auto it = textures_.find(key);
    if (it == textures_.end()) return nullptr;
    return it->second.get();
}

bool AssetManager::hasTexture(const std::string& key) const {
    return textures_.find(key) != textures_.end();
}

static std::vector<std::string> pathsFor(const std::string& rel) {
    return {
        rel,
        std::string("/Users/michelcano/Documents/Repositorios/homo-politicus/") + rel,
    };
}

void AssetManager::preloadDefaults() {
    // Retratos historicos (dominio publico, Wikimedia Commons).
    const char* portraits[] = {
        "bolivar", "san_martin", "belgrano", "juarez", "bismarck",
        "garibaldi", "washington", "lincoln", "napoleon",
    };
    for (const char* slug : portraits) {
        std::string key = std::string("portrait_") + slug;
        loadTexture(key, pathsFor(std::string("assets/portraits/") + slug + ".jpg"));
    }
    // Backgrounds.
    const char* backgrounds[] = {
        "declaration", "napoleon_coronation", "bastille", "revolution",
    };
    for (const char* slug : backgrounds) {
        std::string key = std::string("bg_") + slug;
        loadTexture(key, pathsFor(std::string("assets/backgrounds/") + slug + ".jpg"));
    }
}
