#include "ui/GameOverScreen.hpp"
#include "ui/AssetManager.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdint>

static const sf::Color kPanel   = sf::Color(28, 30, 42);
static const sf::Color kBorder  = sf::Color(80, 160, 240);
static const sf::Color kText    = sf::Color(230, 232, 240);
static const sf::Color kMuted   = sf::Color(150, 154, 168);
static const sf::Color kAccent  = sf::Color(80, 160, 240);
static const sf::Color kBad     = sf::Color(220, 80, 80);
static const sf::Color kGood    = sf::Color(80, 200, 120);
static const sf::Color kBtn     = sf::Color(50, 55, 70);

static constexpr float kPanelW = 800.f;
static constexpr float kPanelH_ = 600.f;
static constexpr float kPanelX = (1280.f - kPanelW) / 2.f;
static constexpr float kPanelY = (800.f - kPanelH_) / 2.f;

void GameOverScreen::show(EndCondition cond, const Country& c, int turn, double popularitySum) {
    visible_ = true;
    cond_ = cond;
    snapshot_ = c;
    turn_ = turn;
    popSum_ = popularitySum;
    fadeTime_ = 0.f;
    deriveLabel();
    computeScore();
}

void GameOverScreen::hide() { visible_ = false; }

void GameOverScreen::update(float dt) {
    if (visible_) fadeTime_ = std::min(fadeTime_ + dt, 1.0f);
}

void GameOverScreen::deriveLabel() {
    switch (cond_) {
        case EndCondition::COUP_SUCCESS:         label_ = "GOLPE MILITAR EXITOSO"; break;
        case EndCondition::IMPEACHMENT:          label_ = "DESTITUCION POR EL CONGRESO"; break;
        case EndCondition::REVOLUTION:           label_ = "REVOLUCION POPULAR"; break;
        case EndCondition::LAWFARE_REMOVAL:      label_ = "INHABILITACION JUDICIAL"; break;
        case EndCondition::ELECTION_LOSS:        label_ = "DERROTA ELECTORAL"; break;
        case EndCondition::EXILE:                label_ = "EXILIO BAJO PRESION INTERNACIONAL"; break;
        case EndCondition::ASSASSINATION:        label_ = "MAGNICIDIO"; break;
        case EndCondition::NUCLEAR_ANNIHILATION: label_ = "ANIQUILACION NUCLEAR"; break;
        case EndCondition::TERM_COMPLETED:       label_ = "MANDATO COMPLETADO"; break;
        default:                                 label_ = "FIN DE PARTIDA"; break;
    }
}

void GameOverScreen::computeScore() {
    double avgPop = turn_ > 0 ? popSum_ / (double)turn_ : snapshot_.politics.popularity;
    double stability = avgPop * snapshot_.politics.regime_legitimacy;
    double softPower = snapshot_.security.diplomatic_prestige;
    double legacy = (snapshot_.welfare.literacy_rate + snapshot_.welfare.health_coverage +
                     snapshot_.infra.innovation_index + snapshot_.infra.road_connectivity) / 4.0;
    double cohesion = 1.0 - snapshot_.politics.polarization_index;
    double base = stability * 30 + softPower * 15 + legacy * 20 + cohesion * 15;
    base += std::min(20.0, turn_ * 0.5); // bonus por turnos sobrevividos
    double penalty = snapshot_.politics.authoritarian_actions_count * 2.0;
    double s = base - penalty;
    if (cond_ == EndCondition::TERM_COMPLETED) s *= 1.2;
    else if (cond_ == EndCondition::COUP_SUCCESS || cond_ == EndCondition::EXILE
             || cond_ == EndCondition::NUCLEAR_ANNIHILATION) s *= 0.5;
    if (s < 0) s = 0;
    score_ = (int)s;
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

void GameOverScreen::onClick(sf::Vector2f mouse) {
    if (!visible_) return;
    auto inside = [](sf::Vector2f p, float x, float y, float w, float h) {
        return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
    };
    float btnW = 280.f, btnH = 50.f;
    float bx1 = kPanelX + 80, by = kPanelY + kPanelH_ - 80;
    float bx2 = kPanelX + kPanelW - 80 - btnW;
    if (inside(mouse, bx1, by, btnW, btnH)) { if (cb_) cb_(Action::NewGame); return; }
    if (inside(mouse, bx2, by, btnW, btnH)) { if (cb_) cb_(Action::MainMenu); return; }
}

void GameOverScreen::draw(sf::RenderWindow& win, const sf::Font& font) const {
    if (!visible_) return;
    std::uint8_t alpha = (std::uint8_t)(std::min(1.f, fadeTime_) * 230);

    // Background historico segun condicion (Bastille para crisis violenta,
    // Declaration para final pacifico).
    const sf::Texture* bgTex = nullptr;
    bool violent = (cond_ == EndCondition::COUP_SUCCESS ||
                    cond_ == EndCondition::REVOLUTION ||
                    cond_ == EndCondition::ASSASSINATION ||
                    cond_ == EndCondition::NUCLEAR_ANNIHILATION);
    if (violent) bgTex = AssetManager::instance().getTexture("bg_bastille");
    else         bgTex = AssetManager::instance().getTexture("bg_declaration");
    if (bgTex) {
        sf::Sprite sprite(*bgTex);
        auto sz = bgTex->getSize();
        float scale = std::max(1280.f / sz.x, 800.f / sz.y);
        sprite.setScale({scale, scale});
        sprite.setPosition({
            (1280.f - sz.x * scale) * 0.5f,
            (800.f - sz.y * scale) * 0.5f
        });
        sprite.setColor(sf::Color(255, 255, 255, alpha));
        win.draw(sprite);
        // Veil sobre el background.
        sf::Color tint = violent ? sf::Color(80, 10, 10, (uint8_t)(alpha * 0.5f))
                                 : sf::Color(0, 0, 30, (uint8_t)(alpha * 0.5f));
        win.draw(mkRect(0, 0, 1280, 800, tint, sf::Color(0,0,0,0), 0.f));
    } else {
        sf::Color ov(0, 0, 0, alpha);
        win.draw(mkRect(0, 0, 1280, 800, ov, sf::Color(0, 0, 0, 0), 0.f));
    }

    win.draw(mkRect(kPanelX, kPanelY, kPanelW, kPanelH_, kPanel, kBorder, 2.f));

    win.draw(mkText(font, "GAME OVER", 14, kAccent, kPanelX + 40, kPanelY + 30));
    sf::Color condColor = (cond_ == EndCondition::TERM_COMPLETED) ? kGood : kBad;
    win.draw(mkText(font, label_, 32, condColor, kPanelX + 40, kPanelY + 55));

    double avgPop = turn_ > 0 ? popSum_ / (double)turn_ : snapshot_.politics.popularity;
    auto row = [&](const std::string& label, const std::string& value, float y, sf::Color valCol = kText) {
        win.draw(mkText(font, label, 14, kMuted, kPanelX + 40, y));
        win.draw(mkText(font, value, 16, valCol, kPanelX + 320, y));
    };
    float y = kPanelY + 130.f;
    std::ostringstream s1, s2, s3, s4, s5, s6;
    s1 << turn_ << " turnos";
    s2 << (int)(avgPop * 100) << "%";
    s3 << snapshot_.politics.authoritarian_actions_count;
    s4 << (int)(snapshot_.politics.regime_legitimacy * 100) << "%";
    s5 << snapshot_.politics.active_scandals;
    s6 << snapshot_.security.war_duration << " turnos";
    row("Turnos en el poder",       s1.str(), y); y += 26;
    row("Popularidad media",        s2.str(), y); y += 26;
    row("Acciones autoritarias",    s3.str(), y); y += 26;
    row("Legitimidad final",        s4.str(), y); y += 26;
    row("Escandalos al final",      s5.str(), y); y += 26;
    row("Tiempo en guerra",         s6.str(), y); y += 40;

    win.draw(mkText(font, "SCORE FINAL", 14, kMuted, kPanelX + 40, y));
    std::string rank = score_ >= 85 ? "Estadista"
                     : score_ >= 60 ? "Lider competente"
                     : score_ >= 40 ? "Politico pragmatico"
                     : score_ >= 20 ? "Caudillo problematico"
                     : "Dictador fallido";
    sf::Text scoreText(font, std::to_string(score_) + "  -  " + rank, 28);
    scoreText.setFillColor(kAccent);
    scoreText.setPosition({kPanelX + 40, y + 20});
    win.draw(scoreText);

    // Botones
    float btnW = 280.f, btnH = 50.f;
    float bx1 = kPanelX + 80, by = kPanelY + kPanelH_ - 80;
    float bx2 = kPanelX + kPanelW - 80 - btnW;
    win.draw(mkRect(bx1, by, btnW, btnH, kBtn, kBorder, 1.f));
    win.draw(mkText(font, "Nueva partida", 18, kText, bx1 + 80, by + 14));
    win.draw(mkRect(bx2, by, btnW, btnH, kBtn, kBorder, 1.f));
    win.draw(mkText(font, "Menu principal", 18, kText, bx2 + 80, by + 14));
}
