#include "ui/ProvinceMap.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>

static const char* kProvinceNames[] = {
    "Capital",       "Norte",      "Sur",        "Este",
    "Oeste",         "Cordillera", "Llanura",    "Costa",
    "Pampa",         "Selva",      "Patagonia",  "Altiplano",
};
static const int kProvinceNameCount = sizeof(kProvinceNames) / sizeof(kProvinceNames[0]);

void ProvinceMap::configure(const IsoWorld& world, const Country& c, int numProvinces) {
    provinces_.clear();
    tileProvince_.clear();
    if (world.tiles().empty()) return;

    // Indices de tiles inside.
    std::vector<size_t> insideIdx;
    for (size_t i = 0; i < world.tiles().size(); ++i) {
        if (world.tiles()[i].inside) insideIdx.push_back(i);
    }
    if (insideIdx.empty()) return;

    if (numProvinces > kProvinceNameCount) numProvinces = kProvinceNameCount;
    if (numProvinces > (int)insideIdx.size()) numProvinces = (int)insideIdx.size();
    if (numProvinces < 1) numProvinces = 1;

    // Seeds evenly distribuidos en el array de inside tiles.
    float step = (float)insideIdx.size() / (float)numProvinces;
    for (int p = 0; p < numProvinces; ++p) {
        Province prov;
        size_t pickIdx = insideIdx[(size_t)((p + 0.5f) * step)];
        prov.seedTileIdx = (int)pickIdx;
        prov.name = kProvinceNames[p % kProvinceNameCount];
        // Satisfaction: popularidad + hash-based offset por provincia.
        unsigned h = (unsigned)(pickIdx * 73856093u + p * 19349663u);
        float offset = ((h & 0xFF) / 255.f - 0.5f) * 0.4f;
        prov.satisfaction = std::clamp((float)c.politics.popularity + offset, 0.f, 1.f);
        // Color tinte deterministic.
        uint8_t r = (uint8_t)(80 + ((h >> 0) & 0x7F));
        uint8_t g = (uint8_t)(80 + ((h >> 7) & 0x7F));
        uint8_t b = (uint8_t)(80 + ((h >> 14) & 0x7F));
        prov.tint = sf::Color(r, g, b, 70);
        provinces_.push_back(prov);
    }

    // Asignar cada tile inside al seed mas cercano.
    tileProvince_.assign(world.tiles().size(), -1);
    for (size_t i = 0; i < world.tiles().size(); ++i) {
        const auto& t = world.tiles()[i];
        if (!t.inside) continue;
        int bestP = 0;
        float bestD = 1e9f;
        for (int p = 0; p < (int)provinces_.size(); ++p) {
            const auto& seedT = world.tiles()[provinces_[p].seedTileIdx];
            float dx = (float)(t.gx - seedT.gx);
            float dy = (float)(t.gy - seedT.gy);
            float d2 = dx * dx + dy * dy;
            if (d2 < bestD) { bestD = d2; bestP = p; }
        }
        tileProvince_[i] = bestP;
    }
}

void ProvinceMap::update(float dt, const Country& c) {
    t_ += dt;
    // Refrescar satisfaction segun popularidad actual.
    for (size_t i = 0; i < provinces_.size(); ++i) {
        unsigned h = (unsigned)(provinces_[i].seedTileIdx * 73856093u + i * 19349663u);
        float offset = ((h & 0xFF) / 255.f - 0.5f) * 0.4f;
        provinces_[i].satisfaction = std::clamp((float)c.politics.popularity + offset, 0.f, 1.f);
    }
}

void ProvinceMap::onMouseMove(sf::Vector2f mouse, const IsoCamera& cam) {
    (void)cam;
    // Hover detectado al draw (donde sabemos screen coords).
    // Por ahora guardo el mouse y lo evaluamos al draw.
    hovered_ = -2; // marker para que draw lo compute
    (void)mouse;
    hoverMouse_ = mouse;
}

std::string ProvinceMap::hoveredTooltip() const {
    if (hovered_ < 0 || hovered_ >= (int)provinces_.size()) return "";
    const auto& p = provinces_[hovered_];
    std::ostringstream oss;
    oss << p.name << " - Satisfaccion: ";
    oss.precision(0);
    oss.setf(std::ios::fixed);
    oss << (p.satisfaction * 100) << "%";
    return oss.str();
}

void ProvinceMap::draw(sf::RenderWindow& win, const sf::Font& font,
                       const IsoCamera& cam, const IsoWorld& world) const {
    float tw = cam.tileW();
    float th = cam.tileH();
    auto drawDiamond = [&](sf::Vector2f c, sf::Color fill, sf::Color outline, float thick) {
        sf::ConvexShape d(4);
        d.setPoint(0, {c.x,            c.y - th * 0.5f});
        d.setPoint(1, {c.x + tw*0.5f, c.y});
        d.setPoint(2, {c.x,            c.y + th * 0.5f});
        d.setPoint(3, {c.x - tw*0.5f, c.y});
        d.setFillColor(fill);
        d.setOutlineColor(outline);
        d.setOutlineThickness(thick);
        win.draw(d);
    };

    // Hover detection: encontrar tile cuyo centro esta mas cercano al mouse.
    int hov = -1;
    {
        float bestD = 14.f * 14.f;
        for (size_t i = 0; i < world.tiles().size(); ++i) {
            if (!world.tiles()[i].inside) continue;
            sf::Vector2f c = world.tiles()[i].screenCenter;
            float dx = hoverMouse_.x - c.x;
            float dy = hoverMouse_.y - c.y;
            float d2 = dx * dx + dy * dy;
            if (d2 < bestD) {
                bestD = d2;
                hov = tileProvince_[i];
            }
        }
    }
    const_cast<int&>(hovered_) = hov;

    // Pintar tinte de provincia sobre cada tile.
    for (size_t i = 0; i < world.tiles().size(); ++i) {
        const auto& t = world.tiles()[i];
        if (!t.inside) continue;
        int p = tileProvince_[i];
        if (p < 0 || p >= (int)provinces_.size()) continue;
        sf::Color fill = provinces_[p].tint;
        if (p == hov) {
            fill.a = (uint8_t)std::min(220, fill.a + 80);
            fill.r = (uint8_t)std::min(255, fill.r + 30);
            fill.g = (uint8_t)std::min(255, fill.g + 30);
            fill.b = (uint8_t)std::min(255, fill.b + 30);
        }
        drawDiamond(t.screenCenter, fill, sf::Color::Transparent, 0.f);
    }
    // Bordes negros entre tiles de provincias distintas.
    int gs = world.gridSize();
    int half = gs / 2;
    auto idxFor = [&](int gx, int gy) -> int {
        int row = gy + half;
        int col = gx + half;
        if (row < 0 || row >= gs || col < 0 || col >= gs) return -1;
        return row * gs + col;
    };
    for (size_t i = 0; i < world.tiles().size(); ++i) {
        const auto& t = world.tiles()[i];
        if (!t.inside) continue;
        int myP = tileProvince_[i];
        // Comprobar 4 vecinos (E, W, N, S).
        const int dgx[4] = {1, -1, 0, 0};
        const int dgy[4] = {0, 0, 1, -1};
        for (int k = 0; k < 4; ++k) {
            int nIdx = idxFor(t.gx + dgx[k], t.gy + dgy[k]);
            if (nIdx < 0 || nIdx >= (int)tileProvince_.size()) continue;
            int nP = tileProvince_[nIdx];
            if (nP < 0) continue;
            if (nP != myP) {
                // Linea entre los dos centroides.
                sf::Vector2f a = t.screenCenter;
                sf::Vector2f b = world.tiles()[nIdx].screenCenter;
                sf::Vector2f mid = {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
                sf::CircleShape dot(1.f * cam.zoom());
                dot.setOrigin({1.f * cam.zoom(), 1.f * cam.zoom()});
                dot.setPosition(mid);
                dot.setFillColor(sf::Color(0, 0, 0, 220));
                win.draw(dot);
            }
        }
    }
    // Nombre del provincia en el seed.
    for (const auto& p : provinces_) {
        const auto& seedT = world.tiles()[p.seedTileIdx];
        sf::Text lbl(font, p.name, 11);
        lbl.setStyle(sf::Text::Bold);
        lbl.setFillColor(sf::Color(245, 240, 220, 220));
        lbl.setOutlineColor(sf::Color(0, 0, 0, 200));
        lbl.setOutlineThickness(1.5f);
        auto bb = lbl.getLocalBounds();
        lbl.setOrigin({bb.position.x + bb.size.x / 2.f, bb.position.y + bb.size.y / 2.f});
        lbl.setPosition(seedT.screenCenter);
        win.draw(lbl);
    }
}
