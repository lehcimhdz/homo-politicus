#include "ui/CourtNetwork.hpp"
#include <cmath>
#include <sstream>

void CourtNetwork::configure(const Country& c) {
    nodes_.clear();
    // Lider en el centro.
    Node leader;
    leader.name = "Presidente";
    leader.role = "Jefe de Estado";
    leader.basePos = {0.5f, 0.5f};
    leader.relation = 1.f;
    leader.color = sf::Color(220, 180, 80);
    leader.radius = 32.f;
    nodes_.push_back(leader);

    // 5 asesores leales (relacion 0.3..0.9).
    const struct { const char* name; const char* role; float rel; } advisors[] = {
        {"Min. Hacienda",      "Economia",            0.7f},
        {"Min. Defensa",       "Seguridad",           0.6f},
        {"Jefe de Gabinete",   "Coordinacion",        0.8f},
        {"Canciller",          "Diplomacia",          0.5f},
        {"Procurador",         "Justicia",            0.4f},
    };
    int N = sizeof(advisors) / sizeof(advisors[0]);
    for (int i = 0; i < N; ++i) {
        float ang = -1.57f + (float)i * (2.f * 3.14159f / (float)(N + 3));
        Node n;
        n.name = advisors[i].name;
        n.role = advisors[i].role;
        n.basePos = {0.5f + std::cos(ang) * 0.18f, 0.5f + std::sin(ang) * 0.20f};
        n.relation = advisors[i].rel;
        n.color = sf::Color(120, 180, 220);
        n.radius = 22.f;
        nodes_.push_back(n);
    }

    // 4 opositores en la periferia (relaciones negativas).
    const struct { const char* name; const char* role; } opps[] = {
        {"Oposicion",           "Congreso"},
        {"Lider Sindical",      "Calles"},
        {"Magnate Empresarial", "Capital"},
        {"Cupula Militar",      "Cuartel"},
    };
    int M = sizeof(opps) / sizeof(opps[0]);
    float baseRel = -(float)c.politics.popular_pressure * 0.5f - 0.2f;
    for (int i = 0; i < M; ++i) {
        float ang = (float)i * (2.f * 3.14159f / (float)M);
        Node n;
        n.name = opps[i].name;
        n.role = opps[i].role;
        n.basePos = {0.5f + std::cos(ang) * 0.42f, 0.5f + std::sin(ang) * 0.42f};
        n.relation = baseRel - (float)i * 0.08f;
        if (n.relation < -1.f) n.relation = -1.f;
        n.color = sf::Color(200, 90, 80);
        n.radius = 20.f;
        nodes_.push_back(n);
    }
}

void CourtNetwork::update(float dt) {
    t_ += dt;
}

void CourtNetwork::onMouseMove(sf::Vector2f mouse) {
    hovered_ = -1;
    for (size_t i = 0; i < nodes_.size(); ++i) {
        sf::Vector2f d = mouse - nodes_[i].screenPos;
        float r = nodes_[i].radius + 2.f;
        if (d.x * d.x + d.y * d.y <= r * r) {
            hovered_ = (int)i;
            return;
        }
    }
}

std::string CourtNetwork::hoveredDetail() const {
    if (hovered_ < 0 || hovered_ >= (int)nodes_.size()) return "";
    const auto& n = nodes_[hovered_];
    std::ostringstream oss;
    oss << n.name << " - " << n.role << " | relacion: ";
    oss.precision(2);
    oss.setf(std::ios::fixed);
    oss << n.relation;
    return oss.str();
}

void CourtNetwork::draw(sf::RenderWindow& win, const sf::Font& font,
                        float x, float y, float w, float h) const {
    // Computar posiciones de pantalla.
    for (size_t i = 0; i < nodes_.size(); ++i) {
        const_cast<Node&>(nodes_[i]).screenPos = {
            x + nodes_[i].basePos.x * w,
            y + nodes_[i].basePos.y * h
        };
    }
    // Aristas del lider a cada otro nodo.
    if (!nodes_.empty()) {
        sf::Vector2f cpos = nodes_[0].screenPos;
        for (size_t i = 1; i < nodes_.size(); ++i) {
            sf::Vector2f e = nodes_[i].screenPos;
            float rel = nodes_[i].relation;
            sf::Color col = rel >= 0
                ? sf::Color((uint8_t)(80 + (1-rel)*100), (uint8_t)(180 + rel*60), 100, 200)
                : sf::Color(220, (uint8_t)(100 + (1+rel)*80), 80, 220);
            float pulse = 0.5f + 0.5f * std::sin(t_ * 3.f + (float)i);
            float th = 1.5f + std::abs(rel) * 3.f + pulse * 0.6f;
            // Linea gruesa: dibujar como rectangle rotado.
            sf::Vector2f delta = e - cpos;
            float len = std::sqrt(delta.x*delta.x + delta.y*delta.y);
            if (len < 1.f) continue;
            float ang = std::atan2(delta.y, delta.x) * 180.f / 3.14159f;
            sf::RectangleShape edge({len, th});
            edge.setOrigin({0.f, th * 0.5f});
            edge.setPosition(cpos);
            edge.setRotation(sf::degrees(ang));
            edge.setFillColor(col);
            win.draw(edge);
        }
    }

    // Nodos.
    for (size_t i = 0; i < nodes_.size(); ++i) {
        const auto& n = nodes_[i];
        // Aura de pulsacion.
        float pulse = 0.5f + 0.5f * std::sin(t_ * 2.f + (float)i * 0.7f);
        sf::CircleShape aura(n.radius + 6.f);
        aura.setOrigin({n.radius + 6.f, n.radius + 6.f});
        aura.setPosition(n.screenPos);
        sf::Color auraColor = n.color;
        auraColor.a = (uint8_t)(40 + 30 * pulse);
        aura.setFillColor(auraColor);
        win.draw(aura);
        // Nodo principal.
        sf::CircleShape node(n.radius);
        node.setOrigin({n.radius, n.radius});
        node.setPosition(n.screenPos);
        node.setFillColor(n.color);
        node.setOutlineColor(((int)i == hovered_) ? sf::Color(255, 240, 180) : sf::Color(20, 22, 30));
        node.setOutlineThickness(((int)i == hovered_) ? 2.5f : 1.5f);
        win.draw(node);
        // Etiqueta debajo.
        sf::Text lbl(font, n.name, 11);
        lbl.setStyle(sf::Text::Bold);
        lbl.setFillColor(sf::Color(225, 228, 240));
        auto bb = lbl.getLocalBounds();
        lbl.setOrigin({bb.position.x + bb.size.x / 2.f, 0.f});
        lbl.setPosition({n.screenPos.x, n.screenPos.y + n.radius + 4.f});
        win.draw(lbl);
        // Rol bajo el nombre.
        sf::Text rl(font, n.role, 9);
        rl.setFillColor(sf::Color(170, 174, 188));
        auto bb2 = rl.getLocalBounds();
        rl.setOrigin({bb2.position.x + bb2.size.x / 2.f, 0.f});
        rl.setPosition({n.screenPos.x, n.screenPos.y + n.radius + 18.f});
        win.draw(rl);
    }
}
