#include "ui/ActionPanel.hpp"
#include <sstream>

static const sf::Color kPanel  = sf::Color(32, 36, 48);
static const sf::Color kPanel2 = sf::Color(46, 50, 64);
static const sf::Color kBorder = sf::Color(60, 65, 80);
static const sf::Color kText   = sf::Color(220, 222, 232);
static const sf::Color kMuted  = sf::Color(150, 154, 168);
static const sf::Color kAccent = sf::Color(80, 160, 240);

// Coordenadas (relativas a la ventana 1280x800), main panel area: 200..1030, 100..680.
static constexpr float kPanelX = 218.f;
static constexpr float kPanelY = 110.f;
static constexpr float kPanelW = 810.f;
static constexpr float kPanelH = 580.f;
static constexpr float kTabH = 36.f;
static constexpr float kBtnW = 240.f;
static constexpr float kBtnH = 48.f;
static constexpr float kBtnGap = 12.f;

ActionPanel::ActionPanel() {
    families_ = {
        { "Fiscal", {
            { "tax+",         "Subir impuestos",          "+10% recaudacion, -5% popularidad, +1pt inflacion" },
            { "tax-",         "Bajar impuestos",          "-10% recaudacion, +3% popularidad" },
            { "wage-",        "Recortar salario minimo",  "+competitividad, -6% popularidad" },
            { "retire+",      "Subir edad jubilacion",    "+sustentabilidad, -5% popularidad, riesgo huelga" },
            { "retire-",      "Bajar edad jubilacion",    "+3% popularidad, -sustentabilidad" },
        }},
        { "Monetaria", {
            { "interest+",    "Subir tasa interes",       "-0.3pt inflacion esperada, contractivo" },
            { "interest-",    "Bajar tasa interes",       "+0.2pt inflacion esperada, expansivo" },
            { "autonomy+",    "Subir autonomia BC",       "+credibilidad, anti-emision" },
            { "autonomy-",    "Bajar autonomia BC",       "+capacidad emision, riesgo fuga" },
            { "print+",       "Emitir moneda",            "curva exponencial de inflacion, downgrade rating" },
        }},
        { "Social", {
            { "worship+",     "Libertad religiosa +",     "-influencia clerical, +tension religiosa" },
            { "worship-",     "Libertad religiosa -",     "+influencia clerical, +radicalismo" },
            { "minority+",    "Proteccion minorias +",    "+UN score, +polarizacion conservadora" },
            { "minority-",    "Proteccion minorias -",    "-UN score, +tension social" },
            { "press+",       "Libertad de prensa +",     "+innovacion, riesgo de escandalo" },
            { "press-",       "Libertad de prensa -",     "+control narrativo, -innovacion" },
        }},
        { "Escandalos", {
            { "cover_up",          "Cubrir escandalo",     "Reduce severidad si exito, sube presion judicial si falla" },
            { "scapegoat",         "Chivo expiatorio",     "Cierra 1 escandalo, -5% popularidad" },
            { "apologize",         "Disculpas publicas",   "-5% popularidad, -20% severidades, cooldown 3 turnos" },
            { "counter_narrative", "Contra-narrativa",     "Reduce exposicion mediatica" },
        }},
        { "Diplomacia", {
            { "diplomacy+", "Inversion diplomatica", "+5pt prestigio, costo fiscal" },
            { "diplomacy-", "Recorte diplomatico",   "-5pt prestigio, ahorro" },
        }},
    };
}

static sf::RectangleShape makeRect(float x, float y, float w, float h, sf::Color fill, sf::Color outline) {
    sf::RectangleShape r({w, h});
    r.setPosition({x, y});
    r.setFillColor(fill);
    r.setOutlineColor(outline);
    r.setOutlineThickness(1.f);
    return r;
}

static sf::Text mkText(const sf::Font& font, const std::string& s, unsigned sz, sf::Color c, float x, float y) {
    sf::Text t(font, s, sz);
    t.setFillColor(c);
    t.setPosition({x, y});
    return t;
}

static bool inside(sf::Vector2f p, float x, float y, float w, float h) {
    return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
}

void ActionPanel::onClick(sf::Vector2f mouse) {
    // Tabs (familias)
    float tabW = kPanelW / (float)families_.size();
    for (size_t i = 0; i < families_.size(); ++i) {
        float tx = kPanelX + (float)i * tabW;
        if (inside(mouse, tx, kPanelY, tabW, kTabH)) {
            currentFamily_ = (int)i;
            return;
        }
    }
    // Botones de acciones
    const auto& actions = families_[currentFamily_].actions;
    float startY = kPanelY + kTabH + 20.f;
    for (size_t i = 0; i < actions.size(); ++i) {
        int col = (int)i % 3;
        int row = (int)i / 3;
        float x = kPanelX + 12 + col * (kBtnW + kBtnGap);
        float y = startY + row * (kBtnH + kBtnGap);
        if (inside(mouse, x, y, kBtnW, kBtnH)) {
            if (cb_) cb_(actions[i].id);
            return;
        }
    }
}

void ActionPanel::onMouseMove(sf::Vector2f mouse) {
    hoveredActionId_.clear();
    hoveredTooltip_.clear();
    const auto& actions = families_[currentFamily_].actions;
    float startY = kPanelY + kTabH + 20.f;
    for (size_t i = 0; i < actions.size(); ++i) {
        int col = (int)i % 3;
        int row = (int)i / 3;
        float x = kPanelX + 12 + col * (kBtnW + kBtnGap);
        float y = startY + row * (kBtnH + kBtnGap);
        if (inside(mouse, x, y, kBtnW, kBtnH)) {
            hoveredActionId_ = actions[i].id;
            hoveredTooltip_ = actions[i].tooltip;
            return;
        }
    }
}

void ActionPanel::draw(sf::RenderWindow& win, const sf::Font& font) const {
    // Tabs
    float tabW = kPanelW / (float)families_.size();
    for (size_t i = 0; i < families_.size(); ++i) {
        float tx = kPanelX + (float)i * tabW;
        sf::Color fill = ((int)i == currentFamily_) ? kAccent : kPanel2;
        sf::Color textCol = ((int)i == currentFamily_) ? sf::Color(255, 255, 255) : kText;
        win.draw(makeRect(tx, kPanelY, tabW, kTabH, fill, kBorder));
        win.draw(mkText(font, families_[i].name, 14, textCol, tx + 12, kPanelY + 9));
    }

    // Botones
    const auto& actions = families_[currentFamily_].actions;
    float startY = kPanelY + kTabH + 20.f;
    for (size_t i = 0; i < actions.size(); ++i) {
        int col = (int)i % 3;
        int row = (int)i / 3;
        float x = kPanelX + 12 + col * (kBtnW + kBtnGap);
        float y = startY + row * (kBtnH + kBtnGap);
        sf::Color fill = (actions[i].id == hoveredActionId_) ? sf::Color(56, 80, 110) : kPanel2;
        win.draw(makeRect(x, y, kBtnW, kBtnH, fill, kBorder));
        win.draw(mkText(font, actions[i].label, 14, kText, x + 12, y + 14));
    }

    // Tooltip al hover
    if (!hoveredTooltip_.empty()) {
        float tx = kPanelX + 12;
        float ty = kPanelY + kPanelH - 60.f;
        win.draw(makeRect(tx, ty, kPanelW - 24, 48, sf::Color(20, 22, 30, 230), kBorder));
        win.draw(mkText(font, hoveredActionId_, 12, kAccent, tx + 10, ty + 6));
        win.draw(mkText(font, hoveredTooltip_, 13, kText, tx + 10, ty + 24));
    }
}
