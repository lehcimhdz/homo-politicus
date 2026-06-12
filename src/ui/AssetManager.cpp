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

static unsigned hashXY(unsigned x, unsigned y) {
    unsigned h = x * 73856093u ^ y * 19349663u;
    h ^= h >> 13;
    h *= 1274126177u;
    h ^= h >> 16;
    return h;
}

static std::unique_ptr<sf::Texture> mkTex(sf::Image& img, bool repeat = true) {
    auto tex = std::make_unique<sf::Texture>(img);
    tex->setRepeated(repeat);
    tex->setSmooth(true);
    return tex;
}

void AssetManager::generateProcedural() {
    // === paper noise 256x256 gris ===
    {
        sf::Image img({256u, 256u}, sf::Color(120, 120, 120));
        for (unsigned y = 0; y < 256; ++y) for (unsigned x = 0; x < 256; ++x) {
            unsigned h = hashXY(x, y);
            uint8_t base = 110 + (uint8_t)(h & 0x3F);
            img.setPixel({x, y}, sf::Color(base, base, base * 95 / 100, 255));
        }
        textures_["texture_paper_noise"] = mkTex(img);
    }
    // === brick: 128x64 con filas de ladrillos rojos ===
    {
        sf::Image img({128u, 64u}, sf::Color(140, 70, 50));
        for (unsigned y = 0; y < 64; ++y) for (unsigned x = 0; x < 128; ++x) {
            // Linea horizontal cada 16px (mortar).
            bool mortarH = (y % 16) == 0 || (y % 16) == 15;
            // Linea vertical cada 32px con shift por fila.
            int row = y / 16;
            int shift = (row % 2) * 16;
            int colX = (x + shift) % 32;
            bool mortarV = (colX == 0 || colX == 31);
            unsigned hh = hashXY(x, y);
            uint8_t r = 130 + (hh & 0x1F);
            uint8_t g =  60 + ((hh >> 4) & 0x0F);
            uint8_t b =  45 + ((hh >> 8) & 0x0F);
            if (mortarH || mortarV) {
                r = 90; g = 70; b = 60;
            }
            img.setPixel({x, y}, sf::Color(r, g, b));
        }
        textures_["texture_brick"] = mkTex(img);
    }
    // === marble: blanco con vetas grises ===
    {
        sf::Image img({128u, 128u}, sf::Color(225, 220, 210));
        for (unsigned y = 0; y < 128; ++y) for (unsigned x = 0; x < 128; ++x) {
            unsigned h = hashXY(x / 4, y / 4);
            float noise = (float)(h & 0xFF) / 255.f;
            // Vetas: sin grande de baja frecuencia.
            float vein = std::sin((float)x * 0.05f + (float)y * 0.08f + noise * 6.28f);
            uint8_t base = 215 + (uint8_t)(noise * 25);
            if (vein > 0.85f) {
                base = (uint8_t)(base * 0.75f);
            }
            img.setPixel({x, y}, sf::Color(base, (uint8_t)(base - 5), (uint8_t)(base - 12)));
        }
        textures_["texture_marble"] = mkTex(img);
    }
    // === roof: tejas verticales gris-marron ===
    {
        sf::Image img({64u, 64u}, sf::Color(90, 60, 50));
        for (unsigned y = 0; y < 64; ++y) for (unsigned x = 0; x < 64; ++x) {
            int colTile = x / 8;
            int innerX = x % 8;
            unsigned h = hashXY(colTile, y);
            uint8_t r = 100 + ((h >> 0) & 0x1F);
            uint8_t g =  65 + ((h >> 4) & 0x1F);
            uint8_t b =  50 + ((h >> 8) & 0x1F);
            // Sombra al borde de cada teja.
            if (innerX == 0 || innerX == 7) {
                r = (uint8_t)(r * 0.6f);
                g = (uint8_t)(g * 0.6f);
                b = (uint8_t)(b * 0.6f);
            }
            img.setPixel({x, y}, sf::Color(r, g, b));
        }
        textures_["texture_roof"] = mkTex(img);
    }
    // === gold: brushed dorado para cupula ===
    {
        sf::Image img({128u, 128u}, sf::Color(220, 175, 70));
        for (unsigned y = 0; y < 128; ++y) for (unsigned x = 0; x < 128; ++x) {
            unsigned h = hashXY(x, y / 4);  // streaks horizontales (brushed)
            uint8_t shimmer = (uint8_t)(h & 0x2F);
            uint8_t r = 210 + shimmer / 2;
            uint8_t g = 160 + shimmer / 3;
            uint8_t b =  60 + shimmer / 4;
            img.setPixel({x, y}, sf::Color(r, g, b));
        }
        textures_["texture_gold"] = mkTex(img);
    }
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
