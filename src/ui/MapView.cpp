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

MapView::MapView() {
    loadSilhouette("argentina");
    isoCam_.setCenter(0.f, 0.f);
}

bool MapView::loadSilhouette(const std::string& name) {
    const std::string candidates[] = {
        "assets/silhouettes/" + name + ".svg",
        "/Users/michelcano/Documents/Repositorios/homo-politicus/assets/silhouettes/" + name + ".svg",
    };
    for (const auto& p : candidates) {
        if (homeSilhouette_.loadFromFile(p)) {
            popDots_.clear();
            return true;
        }
    }
    return false;
}

void MapView::ensurePopDots(int count) const {
    if ((int)popDots_.size() == count) return;
    popDots_.clear();
    if (!homeSilhouette_.loaded() || count <= 0) return;
    // Generar puntos rechazo: sampleados en bbox, aceptados si caen en el poligono.
    unsigned rng = 0xBEEFCAFE;
    auto rand01 = [&]() {
        rng = rng * 1664525u + 1013904223u;
        return (float)(rng & 0xFFFFFF) / (float)0xFFFFFF;
    };
    auto bbox = homeSilhouette_.screenBBox(homePos_.x, homePos_.y, homeRadius_);
    int attempts = 0;
    while ((int)popDots_.size() < count && attempts < count * 30) {
        ++attempts;
        float x = bbox.position.x + rand01() * bbox.size.x;
        float y = bbox.position.y + rand01() * bbox.size.y;
        if (!homeSilhouette_.containsScreen({x, y}, homePos_.x, homePos_.y, homeRadius_)) continue;
        popDots_.push_back({{x, y}, rand01() * 6.28318f});
    }
}

void MapView::update(float dt) {
    t_ += dt;
    isoNpcs_.update(dt);
    isoVehicles_.update(dt);
}

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
    if (homeSilhouette_.loaded()) {
        // Ciclo dia/noche: smooth a partir de t_ con periodo de 24s.
        float nightAmount = (1.f - std::cos(t_ * 0.262f)) * 0.5f;
        if (nightAmount < 0.f) nightAmount = 0.f; if (nightAmount > 1.f) nightAmount = 1.f;
        // Base oscura de la silueta (mas oscura por la noche).
        sf::Color baseFill(
            (uint8_t)(38 - 18 * nightAmount),
            (uint8_t)(42 - 22 * nightAmount),
            (uint8_t)(56 - 16 * nightAmount));
        sf::Color outline(220, 222, 232, 200);
        homeSilhouette_.draw(win, homePos_.x, homePos_.y, homeRadius_, baseFill, outline);
        // Vista isometrica: tiles encima de la silueta base.
        if (!isoConfigured_) {
            // Configurar camara: viewport = centro del bbox, zoom para cubrir bbox con N tiles.
            auto bbox0 = homeSilhouette_.screenBBox(homePos_.x, homePos_.y, homeRadius_);
            float bboxCx = bbox0.position.x + bbox0.size.x * 0.5f;
            float bboxCy = bbox0.position.y + bbox0.size.y * 0.5f;
            isoCam_.setViewport(bboxCx, bboxCy);
            // Para que un grid 22x22 isometric cubra el bbox:
            // ancho iso del grid = N * tileW (en mitad de tile entre filas).
            // Aprox: zoom = bbox.size.x / (gridSize * tileW * 0.6)
            float targetZoom = bbox0.size.x / (22.f * IsoCamera::kTileW * 0.55f);
            isoCam_.setZoom(targetZoom);
            isoWorld_.configure(homeSilhouette_, isoCam_, homePos_, homeRadius_, 22);
            isoBuildings_.configure(isoWorld_, homeSilhouette_, homePos_, homeRadius_, c);
            isoNpcs_.configure(isoBuildings_, c);
            isoVehicles_.configure(isoBuildings_, c);
            isoConfigured_ = true;
        }
        isoBuildings_.updateForCountry(c);
        isoWorld_.draw(win, isoCam_, c, nightAmount);
        isoVehicles_.drawRoads(win, isoCam_);
        isoNpcs_.draw(win, isoCam_, nightAmount);
        isoVehicles_.drawVehicles(win, isoCam_, nightAmount);
        isoBuildings_.draw(win, isoCam_, c, nightAmount);
        // === Eventos contextuales animados ===
        auto capPos = isoBuildings_.buildings().empty() ? sf::Vector2f{0.f, 0.f}
                       : sf::Vector2f{isoBuildings_.buildings()[0].gx, isoBuildings_.buildings()[0].gy};

        // Parade militar: 12 NPCs marchando en formacion 3x4 frente a la capital.
        if (c.security.war_active || c.politics.military_pressure > 0.7) {
            sf::Vector2f marchOrigin{capPos.x - 4.f, capPos.y - 0.5f};
            float marchPhase = std::fmod(t_ * 0.6f, 6.f);
            for (int row = 0; row < 4; ++row) {
                for (int col = 0; col < 3; ++col) {
                    float gx = marchOrigin.x + (float)col * 0.6f + marchPhase;
                    float gy = marchOrigin.y + (float)row * 0.4f;
                    auto s = isoCam_.worldToScreen(gx, gy, 0.f);
                    sf::ConvexShape body(3);
                    body.setPoint(0, {s.x,           s.y - 5.f * isoCam_.zoom()});
                    body.setPoint(1, {s.x - 2.5f * isoCam_.zoom(), s.y + 2.f * isoCam_.zoom()});
                    body.setPoint(2, {s.x + 2.5f * isoCam_.zoom(), s.y + 2.f * isoCam_.zoom()});
                    body.setFillColor(sf::Color(80, 100, 60));
                    body.setOutlineColor(sf::Color(0, 0, 0, 130));
                    body.setOutlineThickness(0.5f);
                    win.draw(body);
                }
            }
            // Etiqueta.
            auto lbl = isoCam_.worldToScreen(marchOrigin.x, marchOrigin.y - 1.5f, 0.f);
            win.draw(makeText(font, "DESFILE MILITAR", 11, sf::Color(150, 200, 110), lbl.x - 50.f, lbl.y));
        }

        // Protesta: cluster en circulo girando frente al capitolio.
        if (c.politics.popular_pressure > 0.6) {
            int marchers = 12 + (int)((c.politics.popular_pressure - 0.6) * 30);
            for (int m = 0; m < marchers; ++m) {
                float ang = t_ * 1.2f + (float)m * (6.28318f / (float)marchers);
                float r = 1.8f + 0.4f * std::sin(t_ * 2.f + m);
                float gx = capPos.x + std::cos(ang) * r;
                float gy = capPos.y + std::sin(ang) * r * 0.6f;
                auto s = isoCam_.worldToScreen(gx, gy, 0.f);
                sf::CircleShape head(1.5f * isoCam_.zoom());
                head.setOrigin({1.5f * isoCam_.zoom(), 1.5f * isoCam_.zoom()});
                head.setPosition(s);
                head.setFillColor(sf::Color(220, 80, 70));
                win.draw(head);
            }
        }

        // Eleccion: cuando turn % 4 == 0 y turno > 0, NPCs convergen a la capital.
        if (turn_ > 0 && turn_ % 4 == 0) {
            int queues = 8;
            for (int q = 0; q < queues; ++q) {
                float ang = (float)q * (6.28318f / (float)queues);
                for (int i = 0; i < 5; ++i) {
                    float r = 0.8f + (float)i * 0.3f;
                    float gx = capPos.x + std::cos(ang) * r;
                    float gy = capPos.y + std::sin(ang) * r * 0.6f;
                    auto s = isoCam_.worldToScreen(gx, gy, 0.f);
                    sf::CircleShape head(1.2f * isoCam_.zoom());
                    head.setOrigin({1.2f * isoCam_.zoom(), 1.2f * isoCam_.zoom()});
                    head.setPosition(s);
                    head.setFillColor(sf::Color(100, 160, 220, 200));
                    win.draw(head);
                }
            }
            auto lbl = isoCam_.worldToScreen(capPos.x - 3.f, capPos.y - 3.5f, 0.f);
            win.draw(makeText(font, "ELECCION EN CURSO", 11, sf::Color(160, 200, 240), lbl.x, lbl.y));
        }
        auto bbox = homeSilhouette_.screenBBox(homePos_.x, homePos_.y, homeRadius_);
        // Population dots: cantidad escalada por urbanizacion, micro-movimiento.
        int dotCount = 60 + (int)(c.welfare.urban_population_ratio * 180);
        ensurePopDots(dotCount);
        // Por la noche, las urbes brillan como luces; de dia se ven mas tenues.
        sf::Color dayColor(245, 240, 220, 200);
        sf::Color nightColor(255, 230, 120, 240);
        sf::Color dotColor(
            (uint8_t)(dayColor.r + (nightColor.r - dayColor.r) * nightAmount),
            (uint8_t)(dayColor.g + (nightColor.g - dayColor.g) * nightAmount),
            (uint8_t)(dayColor.b + (nightColor.b - dayColor.b) * nightAmount),
            (uint8_t)(dayColor.a + (nightColor.a - dayColor.a) * nightAmount));
        float dotR = 1.6f + nightAmount * 0.6f;
        for (const auto& d : popDots_) {
            float ox = std::sin(t_ * 1.3f + d.phase) * 1.2f;
            float oy = std::cos(t_ * 1.1f + d.phase * 1.7f) * 1.2f;
            sf::CircleShape dot(dotR);
            dot.setOrigin({dotR, dotR});
            dot.setPosition({d.base.x + ox, d.base.y + oy});
            dot.setFillColor(dotColor);
            win.draw(dot);
        }
        // Marcha de protesta: columna animada cuando popular_pressure es alto.
        if (c.politics.popular_pressure > 0.5) {
            float pressure = (float)c.politics.popular_pressure;
            int marchers = 6 + (int)((pressure - 0.5f) * 30.f);
            float marchY = bbox.position.y + bbox.size.y * 0.45f;
            float marchSpeed = 50.f + pressure * 30.f;
            for (int m = 0; m < marchers; ++m) {
                float phase = (float)m * 12.f;
                float traveled = std::fmod(t_ * marchSpeed + phase, bbox.size.x + 40.f);
                float mx = bbox.position.x - 20.f + traveled;
                float my = marchY + std::sin(t_ * 8.f + m * 0.9f) * 2.f;
                if (!homeSilhouette_.containsScreen({mx, my}, homePos_.x, homePos_.y, homeRadius_)) continue;
                sf::RectangleShape body({3.f, 5.f});
                body.setOrigin({1.5f, 2.5f});
                body.setPosition({mx, my});
                body.setFillColor(sf::Color(230, 70, 70, 230));
                win.draw(body);
                // Cabeza chica.
                sf::CircleShape head(1.5f);
                head.setOrigin({1.5f, 1.5f});
                head.setPosition({mx, my - 3.5f});
                head.setFillColor(sf::Color(245, 215, 175));
                win.draw(head);
            }
            // Etiqueta sobre el bbox.
            sf::Text warn(font, "PROTESTAS ACTIVAS", 11);
            warn.setFillColor(sf::Color(240, 100, 80));
            warn.setStyle(sf::Text::Bold);
            warn.setPosition({bbox.position.x, bbox.position.y - 18.f});
            win.draw(warn);
        }
    } else {
        win.draw(circleShape(homePos_, homeRadius_, homeFill, kBorder));
    }
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
    // Indicador dia/noche.
    if (homeSilhouette_.loaded()) {
        float nightAmount = (1.f - std::cos(t_ * 0.262f)) * 0.5f;
        const char* phase = nightAmount > 0.66f ? "NOCHE"
                          : nightAmount > 0.33f ? "CREPUSCULO"
                          : "DIA";
        sf::Text dt(font, phase, 12);
        dt.setFillColor(sf::Color(170, 174, 188));
        dt.setPosition({230.f, 678.f});
        win.draw(dt);
    }
}

int MapView::neighborAt(sf::Vector2f mouse) const {
    for (int i = 0; i < 3; ++i) {
        sf::Vector2f d = mouse - neighbor_[i];
        if (d.x * d.x + d.y * d.y <= neighRadius_ * neighRadius_) return i;
    }
    return -1;
}
