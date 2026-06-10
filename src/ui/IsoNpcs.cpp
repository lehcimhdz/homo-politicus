#include "ui/IsoNpcs.hpp"
#include <cmath>
#include <algorithm>

float IsoNpcs::rand01() {
    rng_ = rng_ * 1664525u + 1013904223u;
    return (float)(rng_ & 0xFFFFFF) / (float)0xFFFFFF;
}

void IsoNpcs::pickTarget(Npc& n) {
    if (waypoints_.empty()) return;
    size_t idx = (size_t)(rand01() * waypoints_.size());
    if (idx >= waypoints_.size()) idx = waypoints_.size() - 1;
    // Variar el target dentro de un radio chico del waypoint para evitar que todos converjan al mismo punto.
    sf::Vector2f w = waypoints_[idx];
    n.target = { w.x + (rand01() - 0.5f) * 1.4f, w.y + (rand01() - 0.5f) * 1.4f };
    n.speed = 0.6f + rand01() * 0.9f;
}

void IsoNpcs::configure(const IsoBuilding& buildings, const Country& c) {
    waypoints_.clear();
    for (const auto& b : buildings.buildings()) {
        waypoints_.push_back({b.gx, b.gy});
    }
    int npcCount = 30 + (int)(c.welfare.urban_population_ratio * 50.f);
    if (npcCount > 90) npcCount = 90;
    npcs_.clear();
    npcs_.reserve(npcCount);
    for (int i = 0; i < npcCount; ++i) {
        Npc n;
        if (waypoints_.empty()) {
            n.pos = {0.f, 0.f};
        } else {
            sf::Vector2f w = waypoints_[(size_t)(i % waypoints_.size())];
            n.pos = { w.x + (rand01() - 0.5f) * 2.f, w.y + (rand01() - 0.5f) * 2.f };
        }
        // Color: cuerpos heterogeneos (paleta sutil).
        float h = rand01();
        if (h < 0.30f)      n.color = sf::Color(200, 140, 100);
        else if (h < 0.60f) n.color = sf::Color(120, 130, 180);
        else if (h < 0.85f) n.color = sf::Color(140, 170, 140);
        else                n.color = sf::Color(180, 160, 100);
        pickTarget(n);
        npcs_.push_back(n);
    }
}

void IsoNpcs::update(float dt) {
    for (auto& n : npcs_) {
        sf::Vector2f delta = n.target - n.pos;
        float d2 = delta.x * delta.x + delta.y * delta.y;
        if (d2 < 0.05f * 0.05f) {
            pickTarget(n);
            continue;
        }
        float d = std::sqrt(d2);
        float step = n.speed * dt;
        if (step > d) step = d;
        n.pos.x += (delta.x / d) * step;
        n.pos.y += (delta.y / d) * step;
    }
}

void IsoNpcs::draw(sf::RenderWindow& win, const IsoCamera& cam, float nightAmount) const {
    float z = cam.zoom();
    for (const auto& n : npcs_) {
        sf::Vector2f s = cam.worldToScreen(n.pos.x, n.pos.y, 0.f);
        // Sombra (elipse pequeña abajo).
        sf::CircleShape shadow(3.f * z);
        shadow.setOrigin({3.f * z, 1.5f * z});
        shadow.setScale({1.f, 0.4f});
        shadow.setPosition({s.x, s.y + 2.f * z});
        shadow.setFillColor(sf::Color(0, 0, 0, 90));
        win.draw(shadow);
        // Cuerpo: triangulo apuntando hacia el target (orientacion implicita por dir).
        sf::ConvexShape body(3);
        body.setPoint(0, {s.x,            s.y - 4.f * z});
        body.setPoint(1, {s.x - 2.5f * z, s.y + 2.f * z});
        body.setPoint(2, {s.x + 2.5f * z, s.y + 2.f * z});
        sf::Color c = n.color;
        c.r = (uint8_t)(c.r * (1.f - 0.3f * nightAmount));
        c.g = (uint8_t)(c.g * (1.f - 0.3f * nightAmount));
        c.b = (uint8_t)(c.b * (1.f - 0.2f * nightAmount));
        body.setFillColor(c);
        body.setOutlineColor(sf::Color(0, 0, 0, 110));
        body.setOutlineThickness(0.5f);
        win.draw(body);
    }
}
