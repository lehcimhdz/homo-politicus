#include "ui/IsoBuilding.hpp"
#include <cmath>
#include <algorithm>

void IsoBuilding::configure(const IsoWorld& world, const CountrySilhouette& sil,
                            sf::Vector2f homePos, float homeRadius,
                            const Country& country) {
    (void)sil; (void)homePos; (void)homeRadius;
    buildings_.clear();
    if (world.tiles().empty()) return;

    // Encontrar tiles urban dentro de la silueta.
    std::vector<size_t> urbanIdx;
    int centerGx = 0, centerGy = 0;
    int countInside = 0;
    for (size_t i = 0; i < world.tiles().size(); ++i) {
        const auto& t = world.tiles()[i];
        if (!t.inside) continue;
        ++countInside;
        centerGx += t.gx;
        centerGy += t.gy;
        if (t.biome == IsoWorld::Biome::Urban) urbanIdx.push_back(i);
    }
    if (countInside == 0) return;
    float avgGx = (float)centerGx / countInside;
    float avgGy = (float)centerGy / countInside;

    // Capital: tile urban mas cercano al centro (o cualquier inside si no hay urban).
    size_t capitalIdx = (size_t)-1;
    float bestD = 1e9f;
    auto consider = [&](size_t i) {
        const auto& t = world.tiles()[i];
        float dx = (float)t.gx - avgGx;
        float dy = (float)t.gy - avgGy;
        float d2 = dx*dx + dy*dy;
        if (d2 < bestD) { bestD = d2; capitalIdx = i; }
    };
    if (!urbanIdx.empty()) for (size_t i : urbanIdx) consider(i);
    else for (size_t i = 0; i < world.tiles().size(); ++i) if (world.tiles()[i].inside) consider(i);
    if (capitalIdx == (size_t)-1) return;

    // GDP determina escala global de altura (log10).
    float gdpFactor = (float)std::log10(std::max(1e6, country.economy.gdp)) - 6.f; // ~0..4
    if (gdpFactor < 0.5f) gdpFactor = 0.5f;
    if (gdpFactor > 4.0f) gdpFactor = 4.0f;

    Building cap;
    cap.gx = (float)world.tiles()[capitalIdx].gx + 0.5f;
    cap.gy = (float)world.tiles()[capitalIdx].gy + 0.5f;
    cap.w = 1.6f; cap.d = 1.6f;
    cap.h = 1.6f + gdpFactor * 0.6f;
    cap.kind = Kind::Capital;
    cap.baseColor = sf::Color(210, 175, 80);
    buildings_.push_back(cap);

    // Ciudades secundarias: hasta 9 urban tiles esparcidos.
    std::vector<size_t> remainingUrban;
    for (size_t i : urbanIdx) if (i != capitalIdx) remainingUrban.push_back(i);
    int targetCities = std::min<int>(9, (int)remainingUrban.size());
    // Tomar evenly espaciados.
    if (targetCities > 0) {
        float step = (float)remainingUrban.size() / (float)targetCities;
        for (int k = 0; k < targetCities; ++k) {
            size_t idx = remainingUrban[(size_t)(k * step)];
            const auto& t = world.tiles()[idx];
            Building b;
            b.gx = (float)t.gx + 0.5f;
            b.gy = (float)t.gy + 0.5f;
            b.w = 0.9f; b.d = 0.9f;
            unsigned hash = (unsigned)(t.gx * 73856093 ^ t.gy * 19349663);
            float hFactor = 0.4f + ((hash >> 8) & 0xFF) / 255.f * 0.7f;
            b.h = hFactor + gdpFactor * 0.25f;
            b.kind = Kind::City;
            uint8_t cr = 90  + (hash & 0x3F);
            uint8_t cg = 95  + ((hash >> 6) & 0x3F);
            uint8_t cb = 130 + ((hash >> 12) & 0x3F);
            b.baseColor = sf::Color(cr, cg, cb);
            buildings_.push_back(b);
        }
    }
}

void IsoBuilding::updateForCountry(const Country& country) {
    // Re-escalar alturas con GDP actual (sin reposicionar).
    float gdpFactor = (float)std::log10(std::max(1e6, country.economy.gdp)) - 6.f;
    if (gdpFactor < 0.5f) gdpFactor = 0.5f;
    if (gdpFactor > 4.0f) gdpFactor = 4.0f;
    for (auto& b : buildings_) {
        if (b.kind == Kind::Capital) {
            b.h = 1.6f + gdpFactor * 0.6f;
        } else {
            unsigned hash = (unsigned)((int)(b.gx * 100) * 73856093 ^ (int)(b.gy * 100) * 19349663);
            float hFactor = 0.4f + ((hash >> 8) & 0xFF) / 255.f * 0.7f;
            b.h = hFactor + gdpFactor * 0.25f;
        }
    }
}

sf::Vector2f IsoBuilding::screenAt(size_t idx, const IsoCamera& cam) const {
    if (idx >= buildings_.size()) return {0.f, 0.f};
    const auto& b = buildings_[idx];
    return cam.worldToScreen(b.gx, b.gy, 0.f);
}

void IsoBuilding::drawPrism(sf::RenderWindow& win, const IsoCamera& cam,
                            float gx, float gy, float w, float d, float h,
                            sf::Color base, float nightAmount) {
    // Esquinas del cubo (base z=0, top z=h).
    sf::Vector2f baseBL = cam.worldToScreen(gx - w*0.5f, gy + d*0.5f, 0.f); // bottom-left
    sf::Vector2f baseBR = cam.worldToScreen(gx + w*0.5f, gy + d*0.5f, 0.f); // bottom-right
    sf::Vector2f baseTL = cam.worldToScreen(gx - w*0.5f, gy - d*0.5f, 0.f); // top-left
    sf::Vector2f topBL  = cam.worldToScreen(gx - w*0.5f, gy + d*0.5f, h);
    sf::Vector2f topBR  = cam.worldToScreen(gx + w*0.5f, gy + d*0.5f, h);
    sf::Vector2f topTL  = cam.worldToScreen(gx - w*0.5f, gy - d*0.5f, h);
    sf::Vector2f topTR  = cam.worldToScreen(gx + w*0.5f, gy - d*0.5f, h);

    auto shade = [&](sf::Color c, float k) {
        c.r = (uint8_t)std::clamp((int)(c.r * k), 0, 255);
        c.g = (uint8_t)std::clamp((int)(c.g * k), 0, 255);
        c.b = (uint8_t)std::clamp((int)(c.b * k), 0, 255);
        c.r = (uint8_t)(c.r * (1.f - 0.35f * nightAmount));
        c.g = (uint8_t)(c.g * (1.f - 0.35f * nightAmount));
        c.b = (uint8_t)(c.b * (1.f - 0.25f * nightAmount));
        return c;
    };

    // Cara izquierda (front-left): base BL, base TL, top TL, top BL.
    sf::Color leftColor  = shade(base, 0.65f);
    sf::ConvexShape left(4);
    left.setPoint(0, baseBL);
    left.setPoint(1, baseTL);
    left.setPoint(2, topTL);
    left.setPoint(3, topBL);
    left.setFillColor(leftColor);
    left.setOutlineColor(sf::Color(0,0,0,80));
    left.setOutlineThickness(0.6f);
    win.draw(left);

    // Cara derecha (front-right): base BR, base BL, top BL, top BR.
    sf::Color rightColor = shade(base, 0.85f);
    sf::ConvexShape right(4);
    right.setPoint(0, baseBR);
    right.setPoint(1, baseBL);
    right.setPoint(2, topBL);
    right.setPoint(3, topBR);
    right.setFillColor(rightColor);
    right.setOutlineColor(sf::Color(0,0,0,80));
    right.setOutlineThickness(0.6f);
    win.draw(right);

    // Cara superior: top TL, top TR, top BR, top BL.
    sf::Color topColor   = shade(base, 1.05f);
    sf::ConvexShape top(4);
    top.setPoint(0, topTL);
    top.setPoint(1, topTR);
    top.setPoint(2, topBR);
    top.setPoint(3, topBL);
    top.setFillColor(topColor);
    top.setOutlineColor(sf::Color(0,0,0,90));
    top.setOutlineThickness(0.8f);
    win.draw(top);
}

void IsoBuilding::draw(sf::RenderWindow& win, const IsoCamera& cam,
                       const Country& country, float nightAmount) const {
    (void)country;
    // Ordenar por (gy, gx) para que los traseros se rendericen primero (painter's algo).
    std::vector<size_t> order(buildings_.size());
    for (size_t i = 0; i < buildings_.size(); ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        const auto& A = buildings_[a];
        const auto& B = buildings_[b];
        float ka = A.gx + A.gy;
        float kb = B.gx + B.gy;
        return ka < kb;
    });
    for (size_t i : order) {
        const auto& b = buildings_[i];
        drawPrism(win, cam, b.gx, b.gy, b.w, b.d, b.h, b.baseColor, nightAmount);
        // Banderita dorada en la capital.
        if (b.kind == Kind::Capital) {
            sf::Vector2f flagBase = cam.worldToScreen(b.gx, b.gy, b.h);
            sf::Vector2f flagTip  = cam.worldToScreen(b.gx, b.gy, b.h + 0.7f);
            sf::Vertex pole[2] = {
                sf::Vertex{flagBase, sf::Color(180, 180, 180), {}},
                sf::Vertex{flagTip,  sf::Color(180, 180, 180), {}},
            };
            win.draw(pole, 2, sf::PrimitiveType::Lines);
            sf::ConvexShape flag(3);
            flag.setPoint(0, flagTip);
            flag.setPoint(1, {flagTip.x + 14.f * cam.zoom(), flagTip.y + 4.f * cam.zoom()});
            flag.setPoint(2, {flagTip.x,                     flagTip.y + 8.f * cam.zoom()});
            flag.setFillColor(sf::Color(220, 180, 80));
            win.draw(flag);
        }
    }
}
