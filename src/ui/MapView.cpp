#include "ui/MapView.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>

static const sf::Color kText   = sf::Color(220, 222, 232);
static const sf::Color kMuted  = sf::Color(150, 154, 168);
static const sf::Color kBorder = sf::Color(60, 65, 80);
static const sf::Color kHome   = sf::Color(80, 160, 240);
static const sf::Color kGood   = sf::Color(80, 200, 120);
static const sf::Color kBad    = sf::Color(220, 80, 80);
static const sf::Color kWarn   = sf::Color(240, 180, 60);

MapView::MapView() {}

void MapView::update(float dt) { t_ += dt; }

static sf::Color relationColor(double rel, bool atWar) {
    if (atWar) return kBad;
    if (rel > 0.5)  return kGood;
    if (rel > 0.0)  return sf::Color(110, 180, 110);
    if (rel > -0.3) return kWarn;
    return kBad;
}

static sf::CircleShape circleShape(sf::Vector2f pos, float r, sf::Color fill, sf::Color outline) {
    sf::CircleShape c(r);
    c.setOrigin({r, r});
    c.setPosition(pos);
    c.setFillColor(fill);
    c.setOutlineColor(outline);
    c.setOutlineThickness(2.f);
    return c;
}

static sf::Text makeText(const sf::Font& font, const std::string& s, unsigned size, sf::Color color, float x, float y) {
    sf::Text t(font, s, size);
    t.setFillColor(color);
    t.setPosition({x, y});
    return t;
}

void MapView::draw(sf::RenderWindow& win, const sf::Font& font, const Country& c) const {
    // === Lineas de relacion ===
    for (size_t i = 0; i < c.neighbors.size() && i < 3; ++i) {
        const auto& n = c.neighbors[i];
        sf::Color col = relationColor(n.diplomatic_relations, n.at_war);
        sf::Vertex line[2];
        line[0].position = homePos_;         line[0].color = col;
        line[1].position = neighbor_[i];     line[1].color = col;
        win.draw(line, 2, sf::PrimitiveType::Lines);

        // Puntos viajando (comercio)
        if (n.trade_volume > 1e6 && !n.at_war) {
            float phase = std::fmod(t_ * 0.5f + (float)i * 0.33f, 1.0f);
            for (int k = 0; k < 3; ++k) {
                float p = std::fmod(phase + (float)k * 0.33f, 1.0f);
                sf::Vector2f pt = homePos_ + (neighbor_[i] - homePos_) * p;
                sf::CircleShape dot(3.f);
                dot.setOrigin({3.f, 3.f});
                dot.setPosition(pt);
                dot.setFillColor(sf::Color(255, 220, 100));
                win.draw(dot);
            }
        }
    }

    // === Pais central ===
    sf::Color homeFill = kHome;
    // Color segun estabilidad agregada
    double stability = (c.politics.popularity + c.politics.regime_legitimacy) / 2.0;
    if (stability < 0.30) homeFill = kBad;
    else if (stability < 0.50) homeFill = kWarn;
    win.draw(circleShape(homePos_, homeRadius_, homeFill, kBorder));
    win.draw(makeText(font, "PAIS", 20, kText, homePos_.x - 26, homePos_.y - 14));

    std::ostringstream popStr;
    popStr << std::fixed << std::setprecision(0) << (c.politics.popularity * 100) << "%";
    win.draw(makeText(font, popStr.str(), 16, kText, homePos_.x - 14, homePos_.y + 12));

    // === Vecinos ===
    for (size_t i = 0; i < c.neighbors.size() && i < 3; ++i) {
        const auto& n = c.neighbors[i];
        sf::Color col = relationColor(n.diplomatic_relations, n.at_war);
        win.draw(circleShape(neighbor_[i], neighRadius_, col, kBorder));
        win.draw(makeText(font, n.name, 14, kText, neighbor_[i].x - 35, neighbor_[i].y - 10));

        std::ostringstream relStr;
        relStr << std::fixed << std::setprecision(1) << n.diplomatic_relations;
        win.draw(makeText(font, relStr.str(), 12, kText, neighbor_[i].x - 12, neighbor_[i].y + 10));

        if (n.at_war)               win.draw(makeText(font, "GUERRA", 11, kBad, neighbor_[i].x - 25, neighbor_[i].y + 28));
        else if (n.has_territorial_claim) win.draw(makeText(font, "CLAIM", 11, kWarn, neighbor_[i].x - 20, neighbor_[i].y + 28));
        else if (n.sanctions_against_us)  win.draw(makeText(font, "SANC", 11, kWarn, neighbor_[i].x - 18, neighbor_[i].y + 28));
    }

    // === Leyenda ===
    win.draw(makeText(font, "MAPA REGIONAL  -  click en un vecino: detalle bilateral", 12, kMuted, 230, 660));
}

int MapView::neighborAt(sf::Vector2f mouse) const {
    for (int i = 0; i < 3; ++i) {
        sf::Vector2f d = mouse - neighbor_[i];
        if (d.x * d.x + d.y * d.y <= neighRadius_ * neighRadius_) return i;
    }
    return -1;
}
