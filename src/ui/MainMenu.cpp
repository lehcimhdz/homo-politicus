#include "ui/MainMenu.hpp"
#include "ui/Heraldry.hpp"
#include "ui/AssetManager.hpp"

static const sf::Color kBg     = sf::Color(15, 17, 26);
static const sf::Color kBorder = sf::Color(80, 160, 240);
static const sf::Color kText   = sf::Color(230, 232, 240);
static const sf::Color kTitle  = sf::Color(80, 160, 240);
static const sf::Color kMuted  = sf::Color(140, 144, 158);

static constexpr float kBtnW = 360.f;
static constexpr float kBtnH_ = 56.f;
static constexpr float kBtnGap = 14.f;
static constexpr float kBtnX = (1280.f - kBtnW) / 2.f;
static constexpr float kBtnY0 = 340.f;

MainMenu::MainMenu() {
    buttons_ = {
        { Action::NewGame,      "Nueva partida" },
        { Action::Continue,     "Continuar (cargar slot 1)" },
        { Action::Achievements, "Logros" },
        { Action::Settings,     "Configuracion" },
        { Action::Quit,         "Salir" },
    };
}

static bool inside(sf::Vector2f p, float x, float y, float w, float h) {
    return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
}

int MainMenu::indexAt(sf::Vector2f mouse) const {
    for (size_t i = 0; i < buttons_.size(); ++i) {
        float y = kBtnY0 + (float)i * (kBtnH_ + kBtnGap);
        if (inside(mouse, kBtnX, y, kBtnW, kBtnH_)) return (int)i;
    }
    return -1;
}

void MainMenu::onClick(sf::Vector2f mouse) {
    int idx = indexAt(mouse);
    if (idx < 0) return;
    if (cb_) cb_(buttons_[idx].action);
}

void MainMenu::onMouseMove(sf::Vector2f mouse) {
    hoveredIndex_ = indexAt(mouse);
}

static sf::RectangleShape mkRect(float x, float y, float w, float h, sf::Color fill, sf::Color outline, float thick = 1.f) {
    sf::RectangleShape r({w, h});
    r.setPosition({x, y});
    r.setFillColor(fill);
    r.setOutlineColor(outline);
    r.setOutlineThickness(thick);
    return r;
}


void MainMenu::draw(sf::RenderWindow& win, const sf::Font& font) const {
    win.draw(mkRect(0, 0, 1280, 800, kBg, sf::Color(0, 0, 0, 0), 0.f));

    // Background historico (Liberty Leading the People) si esta cargado.
    const sf::Texture* bg = AssetManager::instance().getTexture("bg_revolution");
    if (bg) {
        sf::Sprite sprite(*bg);
        auto sz = bg->getSize();
        float scale = std::max(1280.f / sz.x, 800.f / sz.y);
        sprite.setScale({scale, scale});
        // Centrar.
        sprite.setPosition({
            (1280.f - sz.x * scale) * 0.5f,
            (800.f - sz.y * scale) * 0.5f
        });
        sprite.setColor(sf::Color(255, 255, 255, 95)); // dim para no competir con UI
        win.draw(sprite);
        // Vignette oscuro arriba/abajo para legibilidad.
        sf::RectangleShape topVeil({1280.f, 280.f});
        topVeil.setPosition({0.f, 0.f});
        topVeil.setFillColor(sf::Color(0, 0, 0, 140));
        win.draw(topVeil);
        sf::RectangleShape bottomVeil({1280.f, 500.f});
        bottomVeil.setPosition({0.f, 300.f});
        bottomVeil.setFillColor(sf::Color(0, 0, 0, 180));
        win.draw(bottomVeil);
    }

    // Heraldica arriba del titulo como sello presidencial.
    {
        unsigned seed = Heraldry::seedFromString("HomoPoliticusNation");
        Heraldry::draw(win, 640.f, 70.f, 38.f, seed);
    }

    // Titulo grande centrado - Cinzel serif si esta disponible
    const sf::Font& titleF = titleFont_ ? *titleFont_ : font;
    sf::Text title(titleF, "HOMO POLITICUS", 84);
    title.setStyle(sf::Text::Bold);
    auto tb = title.getLocalBounds();
    title.setOrigin({tb.size.x / 2.f, tb.size.y / 2.f});
    title.setPosition({640.f, 140.f});
    title.setFillColor(kTitle);
    win.draw(title);

    sf::Text subtitle(titleF, "Lidera. Sobrevive. Decide.", 22);
    auto sb = subtitle.getLocalBounds();
    subtitle.setOrigin({sb.size.x / 2.f, sb.size.y / 2.f});
    subtitle.setPosition({640.f, 220.f});
    subtitle.setFillColor(kMuted);
    win.draw(subtitle);

    sf::Text version(font, "v0.7-beta (Hito 2)", 14);
    version.setPosition({16.f, 770.f});
    version.setFillColor(kMuted);
    win.draw(version);

    // Botones con gradient + hover glow.
    for (size_t i = 0; i < buttons_.size(); ++i) {
        float y = kBtnY0 + (float)i * (kBtnH_ + kBtnGap);
        bool hov = ((int)i == hoveredIndex_);
        sf::Color top = hov ? sf::Color(80, 110, 150) : sf::Color(50, 56, 72);
        sf::Color bot = hov ? sf::Color(40,  70, 110) : sf::Color(28, 32, 42);
        sf::VertexArray grad(sf::PrimitiveType::TriangleStrip, 4);
        grad[0] = sf::Vertex{{kBtnX,         y         }, top, {}};
        grad[1] = sf::Vertex{{kBtnX + kBtnW, y         }, top, {}};
        grad[2] = sf::Vertex{{kBtnX,         y + kBtnH_}, bot, {}};
        grad[3] = sf::Vertex{{kBtnX + kBtnW, y + kBtnH_}, bot, {}};
        win.draw(grad);
        // Inner glow superior (sutil highlight).
        sf::VertexArray glow(sf::PrimitiveType::TriangleStrip, 4);
        sf::Color glowOut(255, 255, 255, hov ? 70 : 25);
        sf::Color glowIn (255, 255, 255, 0);
        glow[0] = sf::Vertex{{kBtnX,         y       }, glowOut, {}};
        glow[1] = sf::Vertex{{kBtnX + kBtnW, y       }, glowOut, {}};
        glow[2] = sf::Vertex{{kBtnX,         y + 6.f }, glowIn,  {}};
        glow[3] = sf::Vertex{{kBtnX + kBtnW, y + 6.f }, glowIn,  {}};
        win.draw(glow);
        // Outline grabado: doble linea.
        win.draw(mkRect(kBtnX, y, kBtnW, kBtnH_, sf::Color(0,0,0,0),
                        hov ? sf::Color(180, 210, 255) : kBorder, hov ? 2.f : 1.f));
        win.draw(mkRect(kBtnX + 3, y + 3, kBtnW - 6, kBtnH_ - 6, sf::Color(0,0,0,0),
                        sf::Color(255, 255, 255, hov ? 50 : 20), 0.5f));
        // Chevron a la izquierda cuando hovered.
        if (hov) {
            sf::ConvexShape chev(3);
            chev.setPoint(0, {kBtnX + 10.f, y + kBtnH_ * 0.5f - 6.f});
            chev.setPoint(1, {kBtnX + 18.f, y + kBtnH_ * 0.5f});
            chev.setPoint(2, {kBtnX + 10.f, y + kBtnH_ * 0.5f + 6.f});
            chev.setFillColor(sf::Color(220, 235, 255));
            win.draw(chev);
        }
        sf::Text label(font, buttons_[i].label, 20);
        label.setStyle(hov ? sf::Text::Bold : sf::Text::Regular);
        label.setFillColor(hov ? sf::Color(245, 250, 255) : kText);
        label.setPosition({kBtnX + (hov ? 32.f : 24.f), y + 16.f});
        win.draw(label);
    }
}
