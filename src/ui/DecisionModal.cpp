#include "ui/DecisionModal.hpp"
#include "ui/AssetManager.hpp"

static const sf::Color kOverlay = sf::Color(0, 0, 0, 180);
// Paleta sepia papiro (modal con textura de pergamino).
static const sf::Color kInk     = sf::Color(60, 35, 18);
static const sf::Color kInkSoft = sf::Color(95, 60, 32);
static const sf::Color kAccent  = sf::Color(130, 30, 30);   // rojo sello
static const sf::Color kButton  = sf::Color(165, 135, 90, 200);
static const sf::Color kButtonH = sf::Color(220, 195, 140, 220);
static const sf::Color kBorder  = sf::Color(110, 80, 40);
static const sf::Color kSkip    = sf::Color(140, 50, 50, 180);
static const sf::Color kSkipH   = sf::Color(180, 70, 70, 220);

static constexpr float kModalW = 720.f;
static constexpr float kModalH = 460.f;
static constexpr float kModalX = (1280.f - kModalW) / 2.f;
static constexpr float kModalY = (800.f - kModalH) / 2.f;

DecisionModal::DecisionModal() {}

void DecisionModal::show(const PendingDecision& d) {
    visible_ = true;
    current_ = d;
    hoveredOption_.clear();
}

void DecisionModal::hide() {
    visible_ = false;
    current_ = PendingDecision();
}

static bool inside(sf::Vector2f p, float x, float y, float w, float h) {
    return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
}

void DecisionModal::onClick(sf::Vector2f mouse) {
    if (!visible_) return;
    // Botones de opciones
    float startY = kModalY + 180.f;
    float btnH = 48.f;
    float btnW = kModalW - 80.f;
    for (size_t i = 0; i < current_.options.size(); ++i) {
        float y = startY + (float)i * (btnH + 12.f);
        if (inside(mouse, kModalX + 40, y, btnW, btnH)) {
            if (choiceCb_) choiceCb_(current_.options[i]);
            return;
        }
    }
    // Skip
    float skipY = kModalY + kModalH - 60.f;
    if (inside(mouse, kModalX + 40, skipY, btnW, 40.f)) {
        if (skipCb_) skipCb_();
        return;
    }
}

void DecisionModal::onMouseMove(sf::Vector2f mouse) {
    hoveredOption_.clear();
    if (!visible_) return;
    float startY = kModalY + 180.f;
    float btnH = 48.f;
    float btnW = kModalW - 80.f;
    for (size_t i = 0; i < current_.options.size(); ++i) {
        float y = startY + (float)i * (btnH + 12.f);
        if (inside(mouse, kModalX + 40, y, btnW, btnH)) {
            hoveredOption_ = current_.options[i];
            return;
        }
    }
    float skipY = kModalY + kModalH - 60.f;
    if (inside(mouse, kModalX + 40, skipY, btnW, 40.f)) {
        hoveredOption_ = "__skip__";
    }
}

static sf::RectangleShape makeRect(float x, float y, float w, float h, sf::Color fill, sf::Color outline = sf::Color(0, 0, 0, 0), float thick = 0.f) {
    sf::RectangleShape r({w, h});
    r.setPosition({x, y});
    r.setFillColor(fill);
    if (outline.a > 0) {
        r.setOutlineColor(outline);
        r.setOutlineThickness(thick);
    }
    return r;
}

static sf::Text mkText(const sf::Font& font, const std::string& s, unsigned sz, sf::Color c, float x, float y) {
    sf::Text t(font, s, sz);
    t.setFillColor(c);
    t.setPosition({x, y});
    return t;
}

void DecisionModal::draw(sf::RenderWindow& win, const sf::Font& font) const {
    if (!visible_) return;

    // Overlay opaco
    win.draw(makeRect(0, 0, 1280, 800, kOverlay));

    // Modal: textura de Magna Carta como pergamino.
    const sf::Texture* parch = AssetManager::instance().getTexture("bg_magna_carta");
    if (parch) {
        sf::Sprite sprite(*parch);
        auto sz = parch->getSize();
        float sx = kModalW / sz.x;
        float sy = kModalH / sz.y;
        sprite.setScale({sx, sy});
        sprite.setPosition({kModalX, kModalY});
        sprite.setColor(sf::Color(255, 230, 195));  // tinte sepia
        win.draw(sprite);
        // Veil de papel envejecido (warm tint) para uniformar.
        win.draw(makeRect(kModalX, kModalY, kModalW, kModalH,
                          sf::Color(245, 220, 175, 110)));
    } else {
        win.draw(makeRect(kModalX, kModalY, kModalW, kModalH,
                          sf::Color(232, 210, 168, 245)));
    }
    // Borde sello con doble linea.
    win.draw(makeRect(kModalX, kModalY, kModalW, kModalH,
                      sf::Color(0,0,0,0), kBorder, 3.f));
    win.draw(makeRect(kModalX + 6, kModalY + 6, kModalW - 12, kModalH - 12,
                      sf::Color(0,0,0,0), sf::Color(170, 130, 70), 1.f));

    // Titulo (tinta roja para "DECISION REQUERIDA" como sello).
    win.draw(mkText(font, "DECISION REQUERIDA", 14, kAccent, kModalX + 40, kModalY + 24));
    sf::Text idText(font, current_.id, 22);
    idText.setStyle(sf::Text::Bold);
    idText.setFillColor(kInk);
    idText.setPosition({kModalX + 40, kModalY + 48});
    win.draw(idText);

    // Prompt (envuelve si es largo, naive split por 80 chars)
    std::string p = current_.prompt;
    float py = kModalY + 100.f;
    size_t start = 0;
    while (start < p.size()) {
        size_t end = std::min(start + 80, p.size());
        if (end < p.size()) {
            size_t spc = p.rfind(' ', end);
            if (spc != std::string::npos && spc > start) end = spc;
        }
        win.draw(mkText(font, p.substr(start, end - start), 14, kInk, kModalX + 40, py));
        py += 20.f;
        start = (end == p.size()) ? end : end + 1;
        if (py > kModalY + 160) break;
    }

    // Botones de opciones
    float startY = kModalY + 180.f;
    float btnH = 48.f;
    float btnW = kModalW - 80.f;
    for (size_t i = 0; i < current_.options.size(); ++i) {
        float y = startY + (float)i * (btnH + 12.f);
        sf::Color fill = (current_.options[i] == hoveredOption_) ? kButtonH : kButton;
        win.draw(makeRect(kModalX + 40, y, btnW, btnH, fill, kBorder, 1.f));
        win.draw(mkText(font, current_.options[i], 16, kInk, kModalX + 56, y + 14));
    }

    // Skip
    float skipY = kModalY + kModalH - 60.f;
    sf::Color skipFill = (hoveredOption_ == "__skip__") ? kSkipH : kSkip;
    win.draw(makeRect(kModalX + 40, skipY, btnW, 40.f, skipFill, kBorder, 1.f));
    win.draw(mkText(font, "Saltar (-credibilidad)", 14, sf::Color(245, 230, 200), kModalX + 56, skipY + 10));
    (void)kInkSoft;
}
