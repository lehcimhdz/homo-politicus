#include "ui/MandateTimeline.hpp"
#include <algorithm>

void MandateTimeline::addEvent(int turn, const std::string& label, sf::Color color) {
    events_.push_back({turn, label, color});
}

void MandateTimeline::draw(sf::RenderWindow& win, const sf::Font& font,
                           float x, float y, float w, float h,
                           int currentTurn, int totalTurns) const {
    // Si el turno actual supera el rango, expandir dinamicamente con margen.
    if (currentTurn + 8 > totalTurns) {
        totalTurns = currentTurn + 8;
    }
    // Linea base.
    float lineY = y + h * 0.55f;
    sf::RectangleShape base({w, 2.f});
    base.setPosition({x, lineY});
    base.setFillColor(sf::Color(80, 88, 110));
    win.draw(base);

    // Marcadores de elecciones (cada 4 turnos).
    auto toX = [&](int turn) {
        float t = (float)turn / (float)totalTurns;
        if (t < 0.f) t = 0.f; if (t > 1.f) t = 1.f;
        return x + t * w;
    };
    for (int t = 0; t <= totalTurns; t += 4) {
        float px = toX(t);
        sf::RectangleShape tick({1.5f, 10.f});
        tick.setPosition({px, lineY - 5.f});
        tick.setFillColor(sf::Color(140, 150, 170));
        win.draw(tick);
    }

    // Marker de turno actual.
    float curX = toX(currentTurn);
    sf::CircleShape now(5.f);
    now.setOrigin({5.f, 5.f});
    now.setPosition({curX, lineY});
    now.setFillColor(sf::Color(240, 200, 80));
    now.setOutlineColor(sf::Color(255, 240, 180));
    now.setOutlineThickness(1.5f);
    win.draw(now);

    // Eventos (sobre la linea).
    for (const auto& ev : events_) {
        if (ev.turn < 0 || ev.turn > totalTurns) continue;
        float ex = toX(ev.turn);
        sf::CircleShape dot(3.5f);
        dot.setOrigin({3.5f, 3.5f});
        dot.setPosition({ex, lineY - 14.f});
        dot.setFillColor(ev.color);
        win.draw(dot);
        // Linea fina al evento.
        sf::RectangleShape link({1.f, 14.f});
        link.setPosition({ex - 0.5f, lineY - 14.f});
        link.setFillColor(sf::Color(ev.color.r, ev.color.g, ev.color.b, 120));
        win.draw(link);
    }

    // Labels: solo los ultimos 4 eventos para no saturar. Clamp dentro del rect.
    int shown = 0;
    for (auto it = events_.rbegin(); it != events_.rend() && shown < 4; ++it, ++shown) {
        float ex = toX(it->turn);
        sf::Text t(font, it->label, 10);
        t.setFillColor(sf::Color(it->color.r, it->color.g, it->color.b, 220));
        auto lb = t.getLocalBounds();
        t.setOrigin({lb.position.x + lb.size.x / 2.f, 0.f});
        // Clamp horizontal: que el texto no sobresalga del rect.
        float labelHalfW = lb.size.x / 2.f;
        float minX = x + labelHalfW + 4.f;
        float maxX = x + w - labelHalfW - 4.f;
        if (ex < minX) ex = minX;
        if (ex > maxX) ex = maxX;
        t.setPosition({ex, lineY - 32.f - shown * 12.f});
        win.draw(t);
    }

    // Etiqueta turno actual.
    sf::Text label(font, "Turno " + std::to_string(currentTurn) + " / " + std::to_string(totalTurns),
                   11);
    label.setFillColor(sf::Color(170, 174, 188));
    label.setPosition({x, lineY + 10.f});
    win.draw(label);
}
