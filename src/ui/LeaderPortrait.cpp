#include "ui/LeaderPortrait.hpp"
#include <cstdint>
#include <cmath>
#include <algorithm>

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

void LeaderPortrait::drawDetailed(sf::RenderWindow& win, const sf::Font& font,
                                  const std::string& name, const std::string& title,
                                  float cx, float cy, float radius,
                                  Expression expr, sf::Color regimeAccent,
                                  float legitimacy) {
    unsigned h = hashName(name);

    // Capa 1: marco grabado dorado.
    sf::CircleShape frame(radius);
    frame.setOrigin({radius, radius});
    frame.setPosition({cx, cy});
    frame.setFillColor(sf::Color(28, 32, 44));
    frame.setOutlineColor(sf::Color(220, 180, 80));
    frame.setOutlineThickness(2.5f);
    win.draw(frame);
    // Grabado: 8 rayos decorativos.
    for (int i = 0; i < 8; ++i) {
        float ang = (float)i * (6.28318f / 8.f);
        float rIn  = radius + 3.f;
        float rOut = radius + 6.f;
        sf::Vertex line[2] = {
            sf::Vertex{{cx + std::cos(ang) * rIn,  cy + std::sin(ang) * rIn},
                       sf::Color(190, 150, 70), {}},
            sf::Vertex{{cx + std::cos(ang) * rOut, cy + std::sin(ang) * rOut},
                       sf::Color(190, 150, 70), {}},
        };
        win.draw(line, 2, sf::PrimitiveType::Lines);
    }

    // Capa 2: hombros con uniforme color regime.
    sf::CircleShape shoulders(radius * 0.85f);
    shoulders.setOrigin({radius * 0.85f, radius * 0.85f});
    shoulders.setPosition({cx, cy + radius * 0.95f});
    shoulders.setFillColor(regimeAccent);
    win.draw(shoulders);
    // Cuello/V-neck.
    sf::ConvexShape vneck(3);
    vneck.setPoint(0, {cx,                   cy + radius * 0.25f});
    vneck.setPoint(1, {cx - radius * 0.12f, cy + radius * 0.55f});
    vneck.setPoint(2, {cx + radius * 0.12f, cy + radius * 0.55f});
    vneck.setFillColor(sf::Color(20, 22, 30));
    win.draw(vneck);
    // Condecoraciones segun legitimacy (1..4 medallas).
    int meds = std::clamp((int)(legitimacy * 4.f + 0.5f), 0, 4);
    for (int m = 0; m < meds; ++m) {
        sf::CircleShape med(radius * 0.06f);
        med.setOrigin({radius * 0.06f, radius * 0.06f});
        med.setPosition({cx - radius * 0.25f + (float)m * radius * 0.13f,
                         cy + radius * 0.45f});
        med.setFillColor(sf::Color(230, 200, 90));
        med.setOutlineColor(sf::Color(150, 110, 30));
        med.setOutlineThickness(0.5f);
        win.draw(med);
    }

    // Capa 3: cabeza.
    sf::Color skin = ([h]() {
        const sf::Color tones[] = {
            {245, 215, 175}, {225, 190, 155}, {200, 160, 120},
            {170, 130, 95},  {130, 95, 70},   {95,  70, 55},
        };
        return tones[h % 6];
    })();
    sf::CircleShape head(radius * 0.48f);
    head.setOrigin({radius * 0.48f, radius * 0.48f});
    head.setPosition({cx, cy - radius * 0.10f});
    head.setFillColor(skin);
    win.draw(head);

    // Capa 4: pelo.
    sf::Color hair = ([h]() {
        const sf::Color tones[] = {
            {35, 30, 25}, {90, 60, 35}, {180, 130, 70},
            {220, 200, 160}, {200, 200, 210}, {130, 80, 50},
        };
        return tones[(h >> 4) % 6];
    })();
    sf::CircleShape hairTop(radius * 0.50f);
    hairTop.setOrigin({radius * 0.50f, radius * 0.50f});
    hairTop.setPosition({cx, cy - radius * 0.22f});
    hairTop.setFillColor(hair);
    win.draw(hairTop);
    // Recubre con cara para ocultar pelo en mitad inferior.
    sf::CircleShape headOver(radius * 0.44f);
    headOver.setOrigin({radius * 0.44f, radius * 0.44f});
    headOver.setPosition({cx, cy - radius * 0.06f});
    headOver.setFillColor(skin);
    win.draw(headOver);

    // Capa 5: ojos + boca segun expresion.
    float eyeY = cy - radius * 0.16f;
    float eyeDX = radius * 0.16f;
    auto drawEye = [&](float ex) {
        sf::CircleShape eye(radius * 0.04f);
        eye.setOrigin({radius * 0.04f, radius * 0.04f});
        eye.setPosition({ex, eyeY});
        eye.setFillColor(sf::Color(30, 30, 40));
        win.draw(eye);
    };
    drawEye(cx - eyeDX);
    drawEye(cx + eyeDX);
    // Cejas y boca segun expresion.
    auto brow = [&](float x0, float y0, float x1, float y1) {
        sf::Vertex line[2] = {
            sf::Vertex{{x0, y0}, sf::Color(30, 30, 40), {}},
            sf::Vertex{{x1, y1}, sf::Color(30, 30, 40), {}},
        };
        win.draw(line, 2, sf::PrimitiveType::Lines);
    };
    float browY = eyeY - radius * 0.10f;
    float mouthY = cy + radius * 0.06f;
    float mouthHalfW = radius * 0.10f;
    switch (expr) {
        case Expression::Happy:
            brow(cx - eyeDX - radius*0.05f, browY + radius*0.02f, cx - eyeDX + radius*0.05f, browY);
            brow(cx + eyeDX - radius*0.05f, browY,                cx + eyeDX + radius*0.05f, browY + radius*0.02f);
            // Boca sonrisa: arco simulado con 2 lineas.
            brow(cx - mouthHalfW, mouthY, cx,             mouthY + radius*0.05f);
            brow(cx,              mouthY + radius*0.05f, cx + mouthHalfW, mouthY);
            break;
        case Expression::Neutral:
            brow(cx - eyeDX - radius*0.05f, browY, cx - eyeDX + radius*0.05f, browY);
            brow(cx + eyeDX - radius*0.05f, browY, cx + eyeDX + radius*0.05f, browY);
            brow(cx - mouthHalfW, mouthY, cx + mouthHalfW, mouthY);
            break;
        case Expression::Worried:
            brow(cx - eyeDX - radius*0.06f, browY - radius*0.03f, cx - eyeDX + radius*0.04f, browY);
            brow(cx + eyeDX - radius*0.04f, browY,                cx + eyeDX + radius*0.06f, browY - radius*0.03f);
            brow(cx - mouthHalfW, mouthY + radius*0.02f, cx, mouthY - radius*0.01f);
            brow(cx, mouthY - radius*0.01f, cx + mouthHalfW, mouthY + radius*0.02f);
            break;
        case Expression::Angry:
            brow(cx - eyeDX - radius*0.05f, browY,                cx - eyeDX + radius*0.05f, browY - radius*0.05f);
            brow(cx + eyeDX - radius*0.05f, browY - radius*0.05f, cx + eyeDX + radius*0.05f, browY);
            brow(cx - mouthHalfW, mouthY + radius*0.04f, cx + mouthHalfW, mouthY + radius*0.04f);
            break;
    }

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
