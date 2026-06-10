#include "ui/IsoVehicles.hpp"
#include <cmath>
#include <algorithm>

float IsoVehicles::rand01() {
    rng_ = rng_ * 1664525u + 1013904223u;
    return (float)(rng_ & 0xFFFFFF) / (float)0xFFFFFF;
}

void IsoVehicles::configure(const IsoBuilding& buildings, const Country& c) {
    roads_.clear();
    vehicles_.clear();
    if (buildings.buildings().size() < 2) return;

    // Capital es siempre el primer building (insertado primero por IsoBuilding).
    const auto& cap = buildings.buildings()[0];
    sf::Vector2f capPos{cap.gx, cap.gy};

    // Roads: capital a cada ciudad.
    for (size_t i = 1; i < buildings.buildings().size(); ++i) {
        const auto& b = buildings.buildings()[i];
        roads_.push_back({capPos, {b.gx, b.gy}});
    }
    // Adicionalmente, conectar pares de ciudades consecutivas (anillo).
    for (size_t i = 1; i + 1 < buildings.buildings().size(); ++i) {
        const auto& a = buildings.buildings()[i];
        const auto& bb = buildings.buildings()[i + 1];
        roads_.push_back({{a.gx, a.gy}, {bb.gx, bb.gy}});
    }

    int targetVehicles = 5 + (int)((float)std::log10(std::max(1e6, c.economy.gdp)) - 6.f) * 3;
    targetVehicles = std::clamp(targetVehicles, 3, 18);
    if (roads_.empty()) return;
    for (int i = 0; i < targetVehicles; ++i) {
        Vehicle v;
        v.roadIdx = (int)(rand01() * roads_.size());
        if (v.roadIdx >= (int)roads_.size()) v.roadIdx = (int)roads_.size() - 1;
        v.t = rand01();
        v.speed = (0.18f + rand01() * 0.18f) * (rand01() < 0.5f ? -1.f : 1.f);
        float h = rand01();
        if (h < 0.33f)      v.color = sf::Color(160, 110,  60);
        else if (h < 0.66f) v.color = sf::Color( 70,  90, 130);
        else                v.color = sf::Color(120, 130, 140);
        vehicles_.push_back(v);
    }
}

void IsoVehicles::update(float dt) {
    for (auto& v : vehicles_) {
        v.t += v.speed * dt;
        if (v.t > 1.f) { v.t = 1.f; v.speed = -std::abs(v.speed); }
        if (v.t < 0.f) { v.t = 0.f; v.speed = std::abs(v.speed); }
    }
}

void IsoVehicles::drawRoads(sf::RenderWindow& win, const IsoCamera& cam) const {
    for (const auto& r : roads_) {
        sf::Vector2f a = cam.worldToScreen(r.a.x, r.a.y, 0.f);
        sf::Vector2f b = cam.worldToScreen(r.b.x, r.b.y, 0.f);
        sf::Vertex line[2] = {
            sf::Vertex{a, sf::Color(120, 100, 70, 120), {}},
            sf::Vertex{b, sf::Color(120, 100, 70, 120), {}},
        };
        win.draw(line, 2, sf::PrimitiveType::Lines);
    }
}

void IsoVehicles::drawVehicles(sf::RenderWindow& win, const IsoCamera& cam, float nightAmount) const {
    float z = cam.zoom();
    for (const auto& v : vehicles_) {
        const auto& r = roads_[v.roadIdx];
        sf::Vector2f wpos {
            r.a.x + (r.b.x - r.a.x) * v.t,
            r.a.y + (r.b.y - r.a.y) * v.t,
        };
        sf::Vector2f s = cam.worldToScreen(wpos.x, wpos.y, 0.f);
        sf::Color c = v.color;
        c.r = (uint8_t)(c.r * (1.f - 0.3f * nightAmount));
        c.g = (uint8_t)(c.g * (1.f - 0.3f * nightAmount));
        c.b = (uint8_t)(c.b * (1.f - 0.2f * nightAmount));
        sf::RectangleShape body({8.f * z, 4.f * z});
        body.setOrigin({4.f * z, 2.f * z});
        body.setPosition(s);
        body.setFillColor(c);
        body.setOutlineColor(sf::Color(0, 0, 0, 130));
        body.setOutlineThickness(0.5f);
        win.draw(body);
        // Ruedas (dos puntos negros).
        sf::CircleShape wheel(0.9f * z);
        wheel.setOrigin({0.9f * z, 0.9f * z});
        wheel.setFillColor(sf::Color(20, 20, 25));
        wheel.setPosition({s.x - 3.f * z, s.y + 2.f * z});
        win.draw(wheel);
        wheel.setPosition({s.x + 3.f * z, s.y + 2.f * z});
        win.draw(wheel);
        // Linterna en la noche.
        if (nightAmount > 0.4f) {
            sf::CircleShape lamp(1.5f * z);
            lamp.setOrigin({1.5f * z, 1.5f * z});
            lamp.setPosition({s.x + (v.speed > 0 ? 5.f : -5.f) * z, s.y - 1.f * z});
            lamp.setFillColor(sf::Color(255, 230, 130, (uint8_t)(180 * (nightAmount - 0.4f) / 0.6f)));
            win.draw(lamp);
        }
    }
}
