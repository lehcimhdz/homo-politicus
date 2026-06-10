#include "ui/IsoWorld.hpp"
#include <cmath>

static unsigned tileHash(int gx, int gy) {
    unsigned h = 2166136261u;
    h ^= (unsigned)(gx * 73856093);
    h *= 16777619u;
    h ^= (unsigned)(gy * 19349663);
    h *= 16777619u;
    return h;
}

sf::Color IsoWorld::biomeColor(Biome b) {
    switch (b) {
        case Biome::Urban:    return sf::Color(120, 120, 130);
        case Biome::Rural:    return sf::Color(110, 150,  80);
        case Biome::Mountain: return sf::Color(140, 120, 100);
        case Biome::Coast:    return sf::Color( 80, 130, 170);
    }
    return sf::Color(110, 150, 80);
}

void IsoWorld::configure(const CountrySilhouette& sil, const IsoCamera& cam,
                         sf::Vector2f homeScreenPos, float homeRadius, int gridSize) {
    (void)homeScreenPos; (void)homeRadius;
    gridSize_ = gridSize;
    tiles_.clear();
    if (!sil.loaded()) return;

    int half = gridSize_ / 2;
    for (int gy = -half; gy < half; ++gy) {
        for (int gx = -half; gx < half; ++gx) {
            Tile t;
            t.gx = gx; t.gy = gy;
            t.screenCenter = cam.worldToScreen((float)gx + 0.5f, (float)gy + 0.5f, 0.f);
            t.inside = sil.containsScreen(t.screenCenter, homeScreenPos.x, homeScreenPos.y, homeRadius);
            unsigned h = tileHash(gx, gy);
            float hf01 = (float)(h & 0xFFFF) / 65535.f;
            // Asignacion de bioma: borde de silueta -> coast, interior con hash -> rural/urban/mountain.
            if (!t.inside) {
                // Probar vecinos para deteccion costa.
                auto neighborCenter = cam.worldToScreen((float)gx + 1.5f, (float)gy + 0.5f, 0.f);
                bool n_in = sil.containsScreen(neighborCenter, homeScreenPos.x, homeScreenPos.y, homeRadius);
                if (n_in) { t.biome = Biome::Coast; t.inside = true; }
                else { t.biome = Biome::Coast; }
            } else if (hf01 < 0.20f) {
                t.biome = Biome::Urban;
            } else if (hf01 < 0.32f) {
                t.biome = Biome::Mountain;
            } else {
                t.biome = Biome::Rural;
            }
            t.satisfaction = 0.5f; // default, refrescada en draw().
            tiles_.push_back(t);
        }
    }
}

static void drawIsoDiamond(sf::RenderWindow& win, sf::Vector2f center, float tileW, float tileH,
                           sf::Color fill, sf::Color outline) {
    sf::ConvexShape diamond(4);
    diamond.setPoint(0, {center.x,            center.y - tileH * 0.5f});
    diamond.setPoint(1, {center.x + tileW*0.5f, center.y              });
    diamond.setPoint(2, {center.x,            center.y + tileH * 0.5f});
    diamond.setPoint(3, {center.x - tileW*0.5f, center.y              });
    diamond.setFillColor(fill);
    diamond.setOutlineColor(outline);
    diamond.setOutlineThickness(0.5f);
    win.draw(diamond);
}

void IsoWorld::draw(sf::RenderWindow& win, const IsoCamera& cam,
                    const Country& country, float nightAmount) const {
    float tw = cam.tileW();
    float th = cam.tileH();
    for (const auto& t : tiles_) {
        if (!t.inside) continue;
        sf::Color base = biomeColor(t.biome);
        // Satisfaction tints (combina popularidad global con hash).
        unsigned h = tileHash(t.gx, t.gy);
        float regOffset = ((h & 0xFF) / 255.f - 0.5f) * 0.30f;
        float sat = (float)country.politics.popularity + regOffset;
        if (sat < 0.f) sat = 0.f; if (sat > 1.f) sat = 1.f;
        // Si satisfaction baja, sombrear hacia rojo; si alta, hacia verde.
        if (sat < 0.4f) {
            base.r = (uint8_t)std::min(255, base.r + 60);
            base.g = (uint8_t)std::max(0, base.g - 30);
        } else if (sat > 0.6f) {
            base.g = (uint8_t)std::min(255, base.g + 30);
        }
        // Atenuacion nocturna.
        base.r = (uint8_t)(base.r * (1.f - 0.35f * nightAmount));
        base.g = (uint8_t)(base.g * (1.f - 0.35f * nightAmount));
        base.b = (uint8_t)(base.b * (1.f - 0.25f * nightAmount));
        sf::Color outline(0, 0, 0, (uint8_t)(50 + 60 * (1.f - nightAmount)));
        drawIsoDiamond(win, t.screenCenter, tw, th, base, outline);
    }
}
