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

static const char* kPortraitSlugs[] = {
    "bolivar", "belgrano", "juarez", "bismarck",
    "garibaldi", "washington", "lincoln", "napoleon",
};
static const int kPortraitCount = 8;

static const char* kPortraitNames[] = {
    "Bolivar",  "Belgrano",  "Juarez",     "Bismarck",
    "Garibaldi", "Washington", "Lincoln",  "Napoleon",
};

const sf::Texture* AssetManager::pickPortrait(int gameSeed, int role) const {
    // Asigna a cada rol un slug distinto rotando con la seed.
    // Asegura que los 4 roles consecutivos no colisionen.
    int idx = ((gameSeed * 7 + role * 11 + 13) % kPortraitCount + kPortraitCount) % kPortraitCount;
    std::string key = std::string("portrait_") + kPortraitSlugs[idx];
    return getTexture(key);
}

const char* AssetManager::pickPortraitName(int gameSeed, int role) const {
    int idx = ((gameSeed * 7 + role * 11 + 13) % kPortraitCount + kPortraitCount) % kPortraitCount;
    return kPortraitNames[idx];
}

void AssetManager::generateProcedural() {
    // Paper noise 256x256 gris con variacion sutil.
    sf::Image img({256u, 256u}, sf::Color(120, 120, 120));
    auto hashXY = [](unsigned x, unsigned y) -> unsigned {
        unsigned h = x * 73856093u ^ y * 19349663u;
        h ^= h >> 13;
        h *= 1274126177u;
        h ^= h >> 16;
        return h;
    };
    for (unsigned y = 0; y < 256; ++y) {
        for (unsigned x = 0; x < 256; ++x) {
            unsigned h = hashXY(x, y);
            uint8_t base = 110 + (uint8_t)(h & 0x3F);  // 110..173
            img.setPixel({x, y}, sf::Color(base, base, base * 95 / 100, 255));
        }
    }
    auto tex = std::make_unique<sf::Texture>(img);
    tex->setRepeated(true);
    tex->setSmooth(true);
    textures_["texture_paper_noise"] = std::move(tex);
}

void AssetManager::preloadDefaults() {
    generateProcedural();
    // Retratos historicos (dominio publico, Wikimedia Commons).
    for (const char* slug : kPortraitSlugs) {
        std::string key = std::string("portrait_") + slug;
        loadTexture(key, pathsFor(std::string("assets/portraits/") + slug + ".jpg"));
    }
    // Backgrounds.
    const char* backgrounds[] = {
        "declaration", "napoleon_coronation", "bastille", "revolution",
        "magna_carta", "declaration_orig",
    };
    for (const char* slug : backgrounds) {
        std::string key = std::string("bg_") + slug;
        loadTexture(key, pathsFor(std::string("assets/backgrounds/") + slug + ".jpg"));
    }
}
