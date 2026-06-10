#include "ui/LeaderPortrait.hpp"
#include <cstdint>

unsigned LeaderPortrait::hashName(const std::string& s) {
    unsigned h = 2166136261u;
    for (char c : s) {
        h ^= (unsigned)(unsigned char)c;
        h *= 16777619u;
    }
    return h;
}

static sf::Color skinFor(unsigned h) {
    static const sf::Color tones[] = {
        {245, 215, 175}, {225, 190, 155}, {200, 160, 120},
        {170, 130, 95},  {130, 95, 70},   {95,  70, 55},
    };
    return tones[h % 6];
}

static sf::Color hairFor(unsigned h) {
    static const sf::Color tones[] = {
        {35, 30, 25}, {90, 60, 35}, {180, 130, 70},
        {220, 200, 160}, {200, 200, 210}, {130, 80, 50},
    };
    return tones[(h >> 4) % 6];
}

static sf::Color outfitFor(unsigned h) {
    static const sf::Color tones[] = {
        {40, 60, 110}, {70, 30, 30}, {30, 60, 50},
        {60, 40, 80}, {40, 40, 50}, {90, 70, 30},
    };
    return tones[(h >> 8) % 6];
}

void LeaderPortrait::draw(sf::RenderWindow& win, const sf::Font& font,
                          const std::string& name, const std::string& title,
                          float cx, float cy, float radius) {
    unsigned h = hashName(name);

    // Marco circular dorado.
    sf::CircleShape frame(radius);
    frame.setOrigin({radius, radius});
    frame.setPosition({cx, cy});
    frame.setFillColor(sf::Color(28, 32, 44));
    frame.setOutlineColor(sf::Color(220, 180, 80));
    frame.setOutlineThickness(2.5f);
    win.draw(frame);

    // Hombros / cuerpo (eclipse abajo).
    sf::CircleShape shoulders(radius * 0.78f);
    shoulders.setOrigin({radius * 0.78f, radius * 0.78f});
    shoulders.setPosition({cx, cy + radius * 0.85f});
    shoulders.setFillColor(outfitFor(h));
    win.draw(shoulders);

    // Cabeza.
    sf::CircleShape head(radius * 0.45f);
    head.setOrigin({radius * 0.45f, radius * 0.45f});
    head.setPosition({cx, cy - radius * 0.10f});
    head.setFillColor(skinFor(h));
    win.draw(head);

    // Pelo (semicirculo arriba de la cabeza).
    sf::CircleShape hair(radius * 0.48f);
    hair.setOrigin({radius * 0.48f, radius * 0.48f});
    hair.setPosition({cx, cy - radius * 0.18f});
    hair.setFillColor(hairFor(h));
    win.draw(hair);
    // Recubre con cara para ocultar el pelo en mitad inferior.
    sf::CircleShape headOver(radius * 0.42f);
    headOver.setOrigin({radius * 0.42f, radius * 0.42f});
    headOver.setPosition({cx, cy - radius * 0.06f});
    headOver.setFillColor(skinFor(h));
    win.draw(headOver);

    // Nombre y titulo.
    if (!name.empty()) {
        sf::Text t(font, name, 14);
        t.setStyle(sf::Text::Bold);
        t.setFillColor(sf::Color(225, 228, 240));
        auto lb = t.getLocalBounds();
        t.setOrigin({lb.position.x + lb.size.x / 2.f, 0.f});
        t.setPosition({cx, cy + radius + 4.f});
        win.draw(t);
    }
    if (!title.empty()) {
        sf::Text t(font, title, 11);
        t.setFillColor(sf::Color(170, 174, 188));
        auto lb = t.getLocalBounds();
        t.setOrigin({lb.position.x + lb.size.x / 2.f, 0.f});
        t.setPosition({cx, cy + radius + 22.f});
        win.draw(t);
    }
}

void LeaderPortrait::drawCompact(sf::RenderWindow& win, const sf::Font& font,
                                 const std::string& name, float cx, float cy, float radius) {
    (void)font;
    unsigned h = hashName(name);

    sf::CircleShape frame(radius);
    frame.setOrigin({radius, radius});
    frame.setPosition({cx, cy});
    frame.setFillColor(sf::Color(28, 32, 44));
    frame.setOutlineColor(sf::Color(190, 160, 80));
    frame.setOutlineThickness(1.5f);
    win.draw(frame);

    sf::CircleShape shoulders(radius * 0.85f);
    shoulders.setOrigin({radius * 0.85f, radius * 0.85f});
    shoulders.setPosition({cx, cy + radius * 0.9f});
    shoulders.setFillColor(outfitFor(h));
    win.draw(shoulders);

    sf::CircleShape head(radius * 0.55f);
    head.setOrigin({radius * 0.55f, radius * 0.55f});
    head.setPosition({cx, cy - radius * 0.05f});
    head.setFillColor(skinFor(h));
    win.draw(head);

    sf::CircleShape hair(radius * 0.55f);
    hair.setOrigin({radius * 0.55f, radius * 0.55f});
    hair.setPosition({cx, cy - radius * 0.18f});
    hair.setFillColor(hairFor(h));
    win.draw(hair);
    sf::CircleShape headOver(radius * 0.50f);
    headOver.setOrigin({radius * 0.50f, radius * 0.50f});
    headOver.setPosition({cx, cy});
    headOver.setFillColor(skinFor(h));
    win.draw(headOver);
}
