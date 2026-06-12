#include "ui/TutorialOverlay.hpp"

static const sf::Color kOverlay = sf::Color(0, 0, 0, 130);
static const sf::Color kPanel   = sf::Color(34, 38, 52);
static const sf::Color kBorder  = sf::Color(240, 180, 60);
static const sf::Color kText    = sf::Color(230, 232, 240);
static const sf::Color kMuted   = sf::Color(150, 154, 168);
static const sf::Color kAccent  = sf::Color(240, 180, 60);
static const sf::Color kBtn     = sf::Color(70, 75, 95);
static const sf::Color kBtnH    = sf::Color(100, 110, 140);

static constexpr float kPanelW = 720.f;
static constexpr float kPanelH = 280.f;
static constexpr float kPanelX = (1280.f - kPanelW) / 2.f;
static constexpr float kPanelY = 800.f - kPanelH - 30.f;

void TutorialOverlay::start() {
    visible_ = true;
    completed_ = false;
    mission_ = 1;
}

void TutorialOverlay::advance() {
    if (!visible_) return;
    mission_++;
    if (mission_ > 5) {
        visible_ = false;
        completed_ = true;
    }
}

void TutorialOverlay::skip() {
    visible_ = false;
    completed_ = true;
    if (skipCb_) skipCb_();
}

TutorialOverlay::MissionText TutorialOverlay::textFor(int m) {
    switch (m) {
        case 1: return {
            "Mision 1/5 — Tu primer turno",
            "Pasa 3 turnos manteniendo tu popularidad sobre 55%.",
            "Tecla N para pasar turno. Mira como cambian las metricas del Dashboard."
        };
        case 2: return {
            "Mision 2/5 — Manejo fiscal",
            "Inflacion alta? Probá Tab 'Accion' para usar tax+ o interest+.",
            "Cada accion tiene trade-off. Ve los efectos al hover del boton."
        };
        case 3: return {
            "Mision 3/5 — Diplomacia",
            "Mejora la relacion con un vecino hostil. Tab 'Mapa' lo muestra.",
            "Click en un vecino abre detalles bilaterales."
        };
        case 4: return {
            "Mision 4/5 — Escandalo",
            "Te van a salir escandalos. Tenes 4 estrategias: cover_up, scapegoat, deny, apologize.",
            "Cada una tiene su costo. apologize es la mas honesta pero quema popularidad."
        };
        case 5: return {
            "Mision 5/5 — Decision reactiva",
            "Cuando aparece una DECISION (tecla D para demo), elegi con cuidado.",
            "Algunas opciones son trampas. cede_power = game over inmediato."
        };
        default: return {"", "", ""};
    }
}

static sf::RectangleShape mkRect(float x, float y, float w, float h, sf::Color fill, sf::Color outline, float thick = 1.f) {
    sf::RectangleShape r({w, h});
    r.setPosition({x, y});
    r.setFillColor(fill);
    r.setOutlineColor(outline);
    r.setOutlineThickness(thick);
    return r;
}

static sf::Text mkText(const sf::Font& font, const std::string& s, unsigned sz, sf::Color c, float x, float y) {
    sf::Text t(font, s, sz);
    t.setFillColor(c);
    t.setPosition({x, y});
    return t;
}

void TutorialOverlay::onClick(sf::Vector2f mouse) {
    if (!visible_) return;
    auto inside = [](sf::Vector2f p, float x, float y, float w, float h) {
        return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
    };
    float btnW = 180.f, btnH = 40.f;
    float bx1 = kPanelX + kPanelW - 200.f, by = kPanelY + kPanelH - 60.f;
    float bx2 = kPanelX + 20.f;
    if (inside(mouse, bx1, by, btnW, btnH)) advance();
    else if (inside(mouse, bx2, by, btnW, btnH)) skip();
}

void TutorialOverlay::draw(sf::RenderWindow& win, const sf::Font& font) const {
    if (!visible_) return;
    win.draw(mkRect(0, 0, 1280, 800, kOverlay, sf::Color(0, 0, 0, 0), 0.f));
    // Panel con gradient.
    {
        sf::Color top(50, 56, 80);
        sf::Color bot(28, 32, 48);
        sf::VertexArray g(sf::PrimitiveType::TriangleStrip, 4);
        g[0] = sf::Vertex{{kPanelX,           kPanelY          }, top, {}};
        g[1] = sf::Vertex{{kPanelX + kPanelW, kPanelY          }, top, {}};
        g[2] = sf::Vertex{{kPanelX,           kPanelY + kPanelH}, bot, {}};
        g[3] = sf::Vertex{{kPanelX + kPanelW, kPanelY + kPanelH}, bot, {}};
        win.draw(g);
        // Highlight superior.
        sf::VertexArray hl(sf::PrimitiveType::TriangleStrip, 4);
        sf::Color hlOut(255, 255, 255, 35); sf::Color hlIn(255, 255, 255, 0);
        hl[0] = sf::Vertex{{kPanelX,           kPanelY      }, hlOut, {}};
        hl[1] = sf::Vertex{{kPanelX + kPanelW, kPanelY      }, hlOut, {}};
        hl[2] = sf::Vertex{{kPanelX,           kPanelY + 6.f}, hlIn,  {}};
        hl[3] = sf::Vertex{{kPanelX + kPanelW, kPanelY + 6.f}, hlIn,  {}};
        win.draw(hl);
        (void)kPanel;
    }
    win.draw(mkRect(kPanelX, kPanelY, kPanelW, kPanelH, sf::Color(0,0,0,0), kBorder, 2.f));
    win.draw(mkRect(kPanelX + 3, kPanelY + 3, kPanelW - 6, kPanelH - 6, sf::Color(0,0,0,0),
                    sf::Color(255, 255, 255, 35), 0.5f));

    auto mt = textFor(mission_);
    win.draw(mkText(font, "TUTORIAL", 12, kAccent, kPanelX + 20, kPanelY + 16));
    win.draw(mkText(font, mt.title, 22, kText, kPanelX + 20, kPanelY + 36));
    win.draw(mkText(font, mt.body, 14, kText, kPanelX + 20, kPanelY + 90));
    win.draw(mkText(font, "Tip: " + mt.tip, 13, kMuted, kPanelX + 20, kPanelY + 130));

    // Botones
    float btnW = 180.f, btnH = 40.f;
    float bx1 = kPanelX + kPanelW - 200.f, by = kPanelY + kPanelH - 60.f;
    float bx2 = kPanelX + 20.f;
    win.draw(mkRect(bx1, by, btnW, btnH, kBtn, kBorder, 1.f));
    win.draw(mkText(font, "Siguiente (SPACE)", 14, kText, bx1 + 16, by + 12));
    win.draw(mkRect(bx2, by, btnW, btnH, kBtnH, sf::Color(160, 80, 80), 1.f));
    win.draw(mkText(font, "Saltar tutorial", 14, kText, bx2 + 30, by + 12));
}
