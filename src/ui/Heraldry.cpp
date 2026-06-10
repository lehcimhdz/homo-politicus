#include "ui/Heraldry.hpp"
#include <cmath>

unsigned Heraldry::seedFromString(const std::string& s) {
    unsigned h = 2166136261u;
    for (char c : s) {
        h ^= (unsigned)(unsigned char)c;
        h *= 16777619u;
    }
    return h;
}

static sf::Color colorFromHash(unsigned h) {
    static const sf::Color palette[] = {
        {180,  30,  35},  // rojo
        { 30,  60, 160},  // azul
        { 30, 130,  60},  // verde
        {200, 170,  30},  // amarillo
        { 80,  35, 130},  // purpura
        {200,  90,  20},  // naranja
        { 35,  35,  40},  // negro
        {220, 220, 215},  // blanco
    };
    return palette[h % 8];
}

static void drawShield(sf::RenderWindow& win, float cx, float cy, float r,
                       sf::Color a, sf::Color b) {
    // Escudo: forma de 5-7 lados estilo coat of arms.
    sf::ConvexShape shield(6);
    shield.setPoint(0, {cx - r * 0.85f, cy - r * 0.85f});
    shield.setPoint(1, {cx + r * 0.85f, cy - r * 0.85f});
    shield.setPoint(2, {cx + r * 0.95f, cy + r * 0.10f});
    shield.setPoint(3, {cx + r * 0.55f, cy + r * 0.85f});
    shield.setPoint(4, {cx - r * 0.55f, cy + r * 0.85f});
    shield.setPoint(5, {cx - r * 0.95f, cy + r * 0.10f});
    shield.setFillColor(a);
    shield.setOutlineColor(sf::Color(40, 30, 20));
    shield.setOutlineThickness(1.5f);
    win.draw(shield);
    // Division: mitad inferior de color b.
    sf::ConvexShape divB(4);
    divB.setPoint(0, {cx - r * 0.90f, cy + r * 0.10f});
    divB.setPoint(1, {cx + r * 0.90f, cy + r * 0.10f});
    divB.setPoint(2, {cx + r * 0.55f, cy + r * 0.83f});
    divB.setPoint(3, {cx - r * 0.55f, cy + r * 0.83f});
    divB.setFillColor(b);
    win.draw(divB);
}

static void drawCircle(sf::RenderWindow& win, float cx, float cy, float r,
                       sf::Color a, sf::Color b) {
    sf::CircleShape c(r);
    c.setOrigin({r, r});
    c.setPosition({cx, cy});
    c.setFillColor(a);
    c.setOutlineColor(sf::Color(40, 30, 20));
    c.setOutlineThickness(1.5f);
    win.draw(c);
    sf::ConvexShape divB(4);
    divB.setPoint(0, {cx - r, cy});
    divB.setPoint(1, {cx + r, cy});
    divB.setPoint(2, {cx + r * 0.7f, cy + r * 0.7f});
    divB.setPoint(3, {cx - r * 0.7f, cy + r * 0.7f});
    divB.setFillColor(b);
    win.draw(divB);
}

static void drawTriangle(sf::RenderWindow& win, float cx, float cy, float r,
                         sf::Color a, sf::Color b) {
    sf::ConvexShape tri(3);
    tri.setPoint(0, {cx,            cy - r});
    tri.setPoint(1, {cx + r * 0.92f, cy + r * 0.6f});
    tri.setPoint(2, {cx - r * 0.92f, cy + r * 0.6f});
    tri.setFillColor(a);
    tri.setOutlineColor(sf::Color(40, 30, 20));
    tri.setOutlineThickness(1.5f);
    win.draw(tri);
    sf::ConvexShape divB(3);
    divB.setPoint(0, {cx,            cy + r * 0.1f});
    divB.setPoint(1, {cx + r * 0.65f, cy + r * 0.58f});
    divB.setPoint(2, {cx - r * 0.65f, cy + r * 0.58f});
    divB.setFillColor(b);
    win.draw(divB);
}

static void drawSymbolEagle(sf::RenderWindow& win, float cx, float cy, float r, sf::Color c) {
    // V-shape con alas.
    sf::ConvexShape wingL(3);
    wingL.setPoint(0, {cx, cy + r * 0.10f});
    wingL.setPoint(1, {cx - r * 0.55f, cy - r * 0.30f});
    wingL.setPoint(2, {cx - r * 0.30f, cy - r * 0.05f});
    wingL.setFillColor(c);
    win.draw(wingL);
    sf::ConvexShape wingR(3);
    wingR.setPoint(0, {cx, cy + r * 0.10f});
    wingR.setPoint(1, {cx + r * 0.55f, cy - r * 0.30f});
    wingR.setPoint(2, {cx + r * 0.30f, cy - r * 0.05f});
    wingR.setFillColor(c);
    win.draw(wingR);
    // Cuerpo central.
    sf::CircleShape head(r * 0.10f);
    head.setOrigin({r * 0.10f, r * 0.10f});
    head.setPosition({cx, cy - r * 0.05f});
    head.setFillColor(c);
    win.draw(head);
}

static void drawSymbolStar(sf::RenderWindow& win, float cx, float cy, float r, sf::Color c) {
    sf::ConvexShape star(10);
    for (int i = 0; i < 10; ++i) {
        float ang = -1.5708f + i * (3.14159f / 5.f);
        float rad = (i % 2 == 0) ? r * 0.55f : r * 0.22f;
        star.setPoint(i, {cx + std::cos(ang) * rad, cy + std::sin(ang) * rad});
    }
    star.setFillColor(c);
    win.draw(star);
}

static void drawSymbolSword(sf::RenderWindow& win, float cx, float cy, float r, sf::Color c) {
    // Lama vertical.
    sf::RectangleShape blade({r * 0.10f, r * 0.85f});
    blade.setOrigin({r * 0.05f, r * 0.65f});
    blade.setPosition({cx, cy + r * 0.05f});
    blade.setFillColor(c);
    win.draw(blade);
    // Guarda horizontal.
    sf::RectangleShape guard({r * 0.50f, r * 0.08f});
    guard.setOrigin({r * 0.25f, r * 0.04f});
    guard.setPosition({cx, cy - r * 0.18f});
    guard.setFillColor(c);
    win.draw(guard);
    // Pomo redondo.
    sf::CircleShape pommel(r * 0.07f);
    pommel.setOrigin({r * 0.07f, r * 0.07f});
    pommel.setPosition({cx, cy - r * 0.30f});
    pommel.setFillColor(c);
    win.draw(pommel);
}

static void drawSymbolSun(sf::RenderWindow& win, float cx, float cy, float r, sf::Color c) {
    sf::CircleShape body(r * 0.22f);
    body.setOrigin({r * 0.22f, r * 0.22f});
    body.setPosition({cx, cy - r * 0.05f});
    body.setFillColor(c);
    win.draw(body);
    for (int i = 0; i < 8; ++i) {
        float ang = i * (3.14159f / 4.f);
        sf::RectangleShape ray({r * 0.30f, r * 0.05f});
        ray.setOrigin({r * 0.30f * 0.5f, r * 0.025f});
        ray.setPosition({cx + std::cos(ang) * r * 0.36f, cy - r * 0.05f + std::sin(ang) * r * 0.36f});
        ray.setRotation(sf::radians(ang));
        ray.setFillColor(c);
        win.draw(ray);
    }
}

void Heraldry::draw(sf::RenderWindow& win, float cx, float cy, float radius, unsigned seed) {
    Shape shape = (Shape)((seed >> 0) % 3);
    sf::Color colA = colorFromHash(seed >> 4);
    sf::Color colB = colorFromHash(seed >> 12);
    // Asegurar contraste minimo.
    if (colA == colB) colB = colorFromHash((seed >> 12) + 1);
    Symbol sym = (Symbol)((seed >> 20) % 4);
    sf::Color symColor = colorFromHash((seed >> 24) + 3);
    if (symColor == colA && symColor == colB) symColor = sf::Color(220, 220, 215);

    switch (shape) {
        case Shape::Shield:   drawShield(win, cx, cy, radius, colA, colB);   break;
        case Shape::Circle:   drawCircle(win, cx, cy, radius, colA, colB);   break;
        case Shape::Triangle: drawTriangle(win, cx, cy, radius, colA, colB); break;
    }
    switch (sym) {
        case Symbol::Eagle: drawSymbolEagle(win, cx, cy, radius, symColor); break;
        case Symbol::Star:  drawSymbolStar (win, cx, cy, radius, symColor); break;
        case Symbol::Sword: drawSymbolSword(win, cx, cy, radius, symColor); break;
        case Symbol::Sun:   drawSymbolSun  (win, cx, cy, radius, symColor); break;
    }
}
