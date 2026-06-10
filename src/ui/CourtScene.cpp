#include "ui/CourtScene.hpp"
#include "ui/LeaderPortrait.hpp"
#include <cmath>

void CourtScene::update(float dt) {
    t_ += dt;
}

static void drawColumn(sf::RenderWindow& win, float cx, float topY, float botY, float w) {
    // Capital (top piece).
    sf::RectangleShape cap({w * 1.6f, 8.f});
    cap.setOrigin({w * 0.8f, 4.f});
    cap.setPosition({cx, topY});
    cap.setFillColor(sf::Color(210, 200, 175));
    win.draw(cap);
    // Shaft.
    sf::RectangleShape shaft({w, botY - topY - 12.f});
    shaft.setOrigin({w * 0.5f, 0.f});
    shaft.setPosition({cx, topY + 4.f});
    shaft.setFillColor(sf::Color(220, 210, 185));
    shaft.setOutlineColor(sf::Color(150, 140, 110));
    shaft.setOutlineThickness(0.5f);
    win.draw(shaft);
    // Base.
    sf::RectangleShape base({w * 1.6f, 8.f});
    base.setOrigin({w * 0.8f, 0.f});
    base.setPosition({cx, botY - 8.f});
    base.setFillColor(sf::Color(180, 165, 135));
    win.draw(base);
}

void CourtScene::draw(sf::RenderWindow& win, const sf::Font& font,
                      float x, float y, float w, float h, const Country& c) const {
    // Fondo del salon.
    sf::RectangleShape bg({w, h});
    bg.setPosition({x, y});
    bg.setFillColor(sf::Color(46, 36, 50));
    bg.setOutlineColor(sf::Color(80, 65, 90));
    bg.setOutlineThickness(1.f);
    win.draw(bg);
    // Piso.
    sf::RectangleShape floor({w, h * 0.20f});
    floor.setPosition({x, y + h * 0.78f});
    floor.setFillColor(sf::Color(60, 45, 38));
    win.draw(floor);
    // Cortinas a los lados.
    for (int side = 0; side < 2; ++side) {
        sf::ConvexShape curtain(4);
        float cw = w * 0.10f;
        float cx = side == 0 ? x + cw * 0.5f : x + w - cw * 0.5f;
        curtain.setPoint(0, {cx - cw * 0.5f, y});
        curtain.setPoint(1, {cx + cw * 0.5f, y});
        curtain.setPoint(2, {cx + cw * 0.4f, y + h * 0.78f});
        curtain.setPoint(3, {cx - cw * 0.4f, y + h * 0.78f});
        curtain.setFillColor(sf::Color(100, 30, 30));
        curtain.setOutlineColor(sf::Color(60, 18, 18));
        curtain.setOutlineThickness(0.8f);
        win.draw(curtain);
    }
    // 4 columnas detras.
    int cols = 4;
    for (int k = 0; k < cols; ++k) {
        float t = (k + 1.f) / (cols + 1.f);
        float cx = x + t * w;
        drawColumn(win, cx, y + h * 0.08f, y + h * 0.78f, w * 0.025f);
    }

    // Lider en podio (centro).
    float lx = x + w * 0.5f;
    float ly = y + h * 0.55f;
    float scale = 1.f + 0.012f * std::sin(t_ * 1.8f); // idle breathing
    // Podio.
    sf::RectangleShape podium({w * 0.12f, h * 0.10f});
    podium.setOrigin({w * 0.06f, 0.f});
    podium.setPosition({lx, y + h * 0.70f});
    podium.setFillColor(sf::Color(100, 80, 60));
    podium.setOutlineColor(sf::Color(70, 55, 40));
    podium.setOutlineThickness(0.8f);
    win.draw(podium);
    // Lider (portrait detallado).
    LeaderPortrait::Expression expr = LeaderPortrait::Expression::Neutral;
    if (c.politics.popularity > 0.65) expr = LeaderPortrait::Expression::Happy;
    else if (c.politics.popularity < 0.30) expr = LeaderPortrait::Expression::Angry;
    else if (c.politics.popular_pressure > 0.6) expr = LeaderPortrait::Expression::Worried;
    sf::Color regimeAccent(40, 60, 110);
    if (c.politics.auth_dem_axis > 0.6) regimeAccent = sf::Color(80, 30, 30);
    else if (c.politics.civilian_military_control < 0.5) regimeAccent = sf::Color(60, 80, 50);
    float lr = 38.f * scale;
    LeaderPortrait::drawDetailed(win, font, "Presidente", "",
                                 lx, ly, lr, expr, regimeAccent,
                                 (float)c.politics.regime_legitimacy);

    // 3 asesores alrededor.
    const struct { const char* name; const char* role; float offset; } ads[] = {
        {"Economia", "Min. Hacienda", -0.30f},
        {"Defensa",  "Min. Seguridad", 0.22f},
        {"Gabinete", "Jefe de Gab.",   0.30f},
    };
    for (int i = 0; i < 3; ++i) {
        float ax = x + w * (0.5f + ads[i].offset);
        float ay = y + h * 0.68f;
        float ascale = 1.f + 0.012f * std::sin(t_ * 1.6f + i * 0.7f);
        float ar = 22.f * ascale;
        LeaderPortrait::drawCompact(win, font, ads[i].name, ax, ay, ar);
        sf::Text nm(font, ads[i].name, 10);
        nm.setFillColor(sf::Color(225, 228, 240));
        auto bb = nm.getLocalBounds();
        nm.setOrigin({bb.position.x + bb.size.x / 2.f, 0.f});
        nm.setPosition({ax, ay + ar + 4.f});
        win.draw(nm);
    }

    // Burbuja de dialogo si hay.
    if (!dialog_.empty()) {
        sf::RectangleShape bubble({w * 0.55f, 50.f});
        bubble.setPosition({x + w * 0.225f, y + h * 0.18f});
        bubble.setFillColor(sf::Color(245, 240, 220));
        bubble.setOutlineColor(sf::Color(120, 100, 60));
        bubble.setOutlineThickness(1.5f);
        win.draw(bubble);
        // Punta hacia el lider.
        sf::ConvexShape tail(3);
        tail.setPoint(0, {lx - 8.f, y + h * 0.18f + 50.f});
        tail.setPoint(1, {lx + 8.f, y + h * 0.18f + 50.f});
        tail.setPoint(2, {lx,       y + h * 0.18f + 64.f});
        tail.setFillColor(sf::Color(245, 240, 220));
        tail.setOutlineColor(sf::Color(120, 100, 60));
        tail.setOutlineThickness(1.5f);
        win.draw(tail);
        sf::Text txt(font, dialog_, 12);
        txt.setFillColor(sf::Color(40, 30, 20));
        txt.setPosition({x + w * 0.24f, y + h * 0.18f + 12.f});
        win.draw(txt);
    }
}
