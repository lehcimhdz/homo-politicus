#include "ui/MainMenu.hpp"
#include "ui/Heraldry.hpp"

static const sf::Color kBg     = sf::Color(15, 17, 26);
static const sf::Color kBtn    = sf::Color(40, 45, 60);
static const sf::Color kBtnH   = sf::Color(70, 100, 140);
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

static sf::Text mkText(const sf::Font& font, const std::string& s, unsigned sz, sf::Color c, float x, float y) {
    sf::Text t(font, s, sz);
    t.setFillColor(c);
    t.setPosition({x, y});
    return t;
}

void MainMenu::draw(sf::RenderWindow& win, const sf::Font& font) const {
    win.draw(mkRect(0, 0, 1280, 800, kBg, sf::Color(0, 0, 0, 0), 0.f));

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

    // Botones
    for (size_t i = 0; i < buttons_.size(); ++i) {
        float y = kBtnY0 + (float)i * (kBtnH_ + kBtnGap);
        sf::Color fill = ((int)i == hoveredIndex_) ? kBtnH : kBtn;
        win.draw(mkRect(kBtnX, y, kBtnW, kBtnH_, fill, kBorder, 1.f));
        win.draw(mkText(font, buttons_[i].label, 20, kText, kBtnX + 24, y + 16));
    }
}
