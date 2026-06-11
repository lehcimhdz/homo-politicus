#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "ui/UIBridge.hpp"
#include "ui/Dashboard.hpp"
#include "ui/MapView.hpp"
#include "ui/ActionPanel.hpp"
#include "ui/DecisionModal.hpp"
#include "ui/AudioSystem.hpp"
#include "ui/MainMenu.hpp"
#include "ui/GameOverScreen.hpp"
#include "ui/TutorialOverlay.hpp"
#include "ui/ParticleEmitter.hpp"
#include "ui/LeaderPortrait.hpp"
#include "ui/MandateTimeline.hpp"
#include "ui/CourtNetwork.hpp"
#include "ui/CourtScene.hpp"
#include "ui/Heraldry.hpp"
#include "Localization.hpp"

enum class AppState { Menu, Playing };
static const std::string LOCALES_DIR = "/Users/michelcano/Documents/Repositorios/homo-politicus-game/content/locales";
static std::string tr(const std::string& key, const std::string& fallback) {
    if (Localization::currentLanguage().empty()) return fallback;
    std::string v = Localization::tr(key);
    if (v.find("[missing:") != std::string::npos) return fallback;
    return v;
}
enum class Tab { Dashboard, Map, Action, Decisions, Achievements, Court };

// Layout (1280x800):
//   TopBar:    1280 x 60
//   SidebarL:  200 x 640 (60..700)
//   MainPanel: 830 x 640
//   SidebarR:  250 x 640
//   BottomBar: 1280 x 100 (700..800)

static const sf::Color kBorder = sf::Color(60, 65, 80);
static const sf::Color kText   = sf::Color(220, 222, 232);
static const sf::Color kMuted  = sf::Color(150, 154, 168);
static const sf::Color kAccent = sf::Color(80, 160, 240);
static const sf::Color kGood   = sf::Color(80, 200, 120);
static const sf::Color kBad    = sf::Color(220, 80, 80);
static const sf::Color kWarn   = sf::Color(240, 180, 60);

// Paleta dinamica: fondo y paneles cambian segun el estado del pais.
struct Palette {
    sf::Color bg;       // fondo general
    sf::Color panel;    // paneles principales
    sf::Color topbar;   // barra superior
    sf::Color sidebar;  // sidebars
    sf::Color accent;   // tinte de acento
};

static const Palette kPaletteCalm    = { {20,22,30},  {32,36,48},  {28,34,52},  {26,30,44},  {80,160,240} };
static const Palette kPaletteCrisis  = { {38,32,18},  {52,46,28},  {58,48,24},  {44,40,24},  {220,180,60} };
static const Palette kPaletteWar     = { {40,16,16},  {64,28,28},  {72,30,30},  {52,24,24},  {220,90,80} };
static const Palette kPaletteEpidemic= { {26,16,38},  {44,28,62},  {52,30,72},  {36,22,52},  {180,120,220} };

static Palette paletteForCountry(const Country& c) {
    if (c.politics.civil_war_active || c.security.war_active) return kPaletteWar;
    if (c.welfare.pandemic_active) return kPaletteEpidemic;
    if (c.economy.inflation > 0.15) return kPaletteCrisis;
    if (c.politics.popularity > 0.55 && c.politics.popular_pressure < 0.4) return kPaletteCalm;
    return kPaletteCalm;
}

static sf::Color lerpColor(sf::Color a, sf::Color b, float t) {
    if (t < 0.f) t = 0.f; if (t > 1.f) t = 1.f;
    return sf::Color(
        (uint8_t)(a.r + (b.r - a.r) * t),
        (uint8_t)(a.g + (b.g - a.g) * t),
        (uint8_t)(a.b + (b.b - a.b) * t),
        (uint8_t)(a.a + (b.a - a.a) * t)
    );
}

static Palette lerpPalette(const Palette& a, const Palette& b, float t) {
    return {
        lerpColor(a.bg, b.bg, t),
        lerpColor(a.panel, b.panel, t),
        lerpColor(a.topbar, b.topbar, t),
        lerpColor(a.sidebar, b.sidebar, t),
        lerpColor(a.accent, b.accent, t),
    };
}

static bool loadFontFallback(sf::Font& font) {
    const char* candidates[] = {
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/Library/Fonts/Arial.ttf",
    };
    for (const char* p : candidates) {
        if (font.openFromFile(p)) return true;
    }
    return false;
}

// Cinzel: serif romano elegante (Google Fonts OFL) para titulos presidenciales.
// Si no carga, callers usan la fuente sans como fallback.
static bool loadTitleFont(sf::Font& font) {
    const char* candidates[] = {
        "assets/fonts/Cinzel-Regular.ttf",
        "/Users/michelcano/Documents/Repositorios/homo-politicus/assets/fonts/Cinzel-Regular.ttf",
    };
    for (const char* p : candidates) {
        if (font.openFromFile(p)) return true;
    }
    return false;
}

static sf::RectangleShape makePanel(float x, float y, float w, float h, sf::Color fill = sf::Color(32, 36, 48)) {
    sf::RectangleShape r({w, h});
    r.setPosition({x, y});
    r.setFillColor(fill);
    r.setOutlineColor(kBorder);
    r.setOutlineThickness(1.f);
    return r;
}

static sf::Text makeText(const sf::Font& font, const std::string& s, unsigned size, sf::Color color, float x, float y) {
    sf::Text t(font, s, size);
    t.setFillColor(color);
    t.setPosition({x, y});
    return t;
}

static std::string fmtMoney(double v) {
    std::ostringstream oss;
    if (v >= 1e9) oss << "$" << std::fixed << std::setprecision(1) << v / 1e9 << "B";
    else if (v >= 1e6) oss << "$" << std::fixed << std::setprecision(0) << v / 1e6 << "M";
    else oss << "$" << std::fixed << std::setprecision(0) << v;
    return oss.str();
}

static std::string fmtPct(double v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << (v * 100) << "%";
    return oss.str();
}

static void drawProgressBar(sf::RenderWindow& win, float x, float y, float w, float h, float ratio, sf::Color fg) {
    sf::RectangleShape bg({w, h});
    bg.setPosition({x, y});
    bg.setFillColor(sf::Color(50, 55, 70));
    win.draw(bg);
    if (ratio < 0) ratio = 0; if (ratio > 1) ratio = 1;
    sf::RectangleShape fill({w * ratio, h});
    fill.setPosition({x, y});
    fill.setFillColor(fg);
    win.draw(fill);
}

static sf::Color popularityColor(double pop) {
    if (pop < 0.30) return kBad;
    if (pop < 0.50) return kWarn;
    return kGood;
}

int main(int argc, char** argv) {
    bool headless = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--headless") headless = true;
    }

    sf::RenderWindow window(sf::VideoMode({1280, 800}), "Homo Politicus");
    window.setFramerateLimit(60);

    sf::Font font;
    bool fontOk = loadFontFallback(font);
    sf::Font titleFont;
    bool titleFontOk = loadTitleFont(titleFont);
    const sf::Font& fTitle = titleFontOk ? titleFont : font;

    Localization::load(LOCALES_DIR, "es");
    UIBridge bridge;
    Dashboard dashboard;
    MapView mapView;
    ActionPanel actionPanel;
    DecisionModal modal;
    AudioSystem audio;
    MainMenu menu;
    menu.setTitleFont(titleFontOk ? &titleFont : nullptr);
    GameOverScreen gameOver;
    ParticleEmitter particles;
    MandateTimeline timeline;
    timeline.addEvent(0, "Asuncion", sf::Color(80, 160, 240));
    CourtNetwork court;
    court.configure(bridge.country());
    CourtScene scene;
    TutorialOverlay tutorialUI;
    double popularitySumDemo = 0.0;
    AppState appState = AppState::Menu;
    gameOver.setCallback([&](GameOverScreen::Action a) {
        audio.play("button_click");
        if (a == GameOverScreen::Action::NewGame) {
            bridge.resetCountry();
            dashboard.recordHistory(bridge.country());
            popularitySumDemo = 0.0;
            gameOver.hide();
            appState = AppState::Playing;
        } else if (a == GameOverScreen::Action::MainMenu) {
            gameOver.hide();
            appState = AppState::Menu;
        }
    });
    menu.setCallback([&](MainMenu::Action a) {
        audio.play("button_click");
        switch (a) {
            case MainMenu::Action::NewGame:
                bridge.resetCountry();
                dashboard.recordHistory(bridge.country());
                tutorialUI.start();
                appState = AppState::Playing;
                break;
            case MainMenu::Action::Continue:
                bridge.resetCountry();
                dashboard.recordHistory(bridge.country());
                appState = AppState::Playing;
                break;
            case MainMenu::Action::Quit:
                window.close();
                break;
            default: break;
        }
    });
    Tab currentTab = Tab::Dashboard;
    sf::Clock frameClock;
    Palette currentPalette = paletteForCountry(bridge.country());
    // Animacion de paso de turno: sweep + shake del TopBar + glow en cards.
    float turnSweep = 0.f;  // 0..0.3s
    float turnShake = 0.f;  // 0..0.15s
    auto triggerTurnAnim = [&]() {
        turnSweep = 0.3f;
        turnShake = 0.15f;
        dashboard.triggerTurnGlow();
    };
    // Cinematic transitions:
    Tab previousTab = currentTab;
    float tabTransition = 0.f;   // 0..0.25s
    bool prevModalVisible = false;
    float modalOpenAnim = 0.f;   // 0..0.20s
    float modalCloseAnim = 0.f;  // 0..0.25s (despues de cerrar, flash)
    bool prevGameOverVisible = false;
    float gameOverFade = 0.f;    // 0..1 (fades a rojo)
    auto doTick = [&]() {
        double prevPop = bridge.country().politics.popularity;
        double prevGDP = bridge.country().economy.gdp;
        bridge.tick();
        dashboard.recordHistory(bridge.country());
        const Country& cc = bridge.country();
        popularitySumDemo += cc.politics.popularity;
        audio.play("turn_advance");
        triggerTurnAnim();
        double dPop = cc.politics.popularity - prevPop;
        if (cc.economy.gdp > prevGDP * 1.03) {
            particles.emit(ParticleEmitter::Preset::BoomGold, 615.f, 220.f, 28);
        }
        if (dPop > 0.03) {
            particles.emit(ParticleEmitter::Preset::Confetti, 615.f, 220.f, 40);
        } else if (dPop < -0.03) {
            particles.emit(ParticleEmitter::Preset::GraySmoke, 615.f, 220.f, 20);
        }
        if (bridge.turn() % 4 == 0 && cc.politics.popularity > 0.5) {
            particles.emit(ParticleEmitter::Preset::Confetti, 615.f, 220.f, 50);
        }
        // Registrar eventos clave en el timeline.
        if (bridge.turn() % 4 == 0 && bridge.turn() > 0) {
            timeline.addEvent(bridge.turn(), "Eleccion", sf::Color(80, 200, 120));
        }
        if (dPop < -0.05) {
            timeline.addEvent(bridge.turn(), "Caida pop.", sf::Color(220, 80, 80));
        } else if (dPop > 0.05) {
            timeline.addEvent(bridge.turn(), "Repunte", sf::Color(80, 200, 120));
        }
    };
    std::string lastActionFeedback;
    actionPanel.setCallback([&](const std::string& id) {
        lastActionFeedback = ">> " + id;
        audio.play("button_click");
    });
    modal.setChoiceCallback([&](const std::string& choice) {
        lastActionFeedback = ">> Decision: " + choice;
        audio.play("button_click");
        modal.hide();
    });
    modal.setSkipCallback([&]() {
        lastActionFeedback = ">> Decision saltada (-credibilidad)";
        audio.play("warning");
        modal.hide();
    });
    dashboard.recordHistory(bridge.country());

    if (headless) {
        std::cout << "[headless] window=1280x800 font=" << fontOk
                  << " gdp=" << bridge.country().economy.gdp
                  << " pop=" << bridge.country().politics.popularity
                  << std::endl;
        window.close();
        return 0;
    }

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* mm = event->getIf<sf::Event::MouseMoved>()) {
                sf::Vector2f pos((float)mm->position.x, (float)mm->position.y);
                if (appState == AppState::Menu) menu.onMouseMove(pos);
                else if (modal.visible()) modal.onMouseMove(pos);
                else if (currentTab == Tab::Action) actionPanel.onMouseMove(pos);
                else if (currentTab == Tab::Dashboard) dashboard.onMouseMove(pos);
                else if (currentTab == Tab::Court) court.onMouseMove(pos);
                else if (currentTab == Tab::Map) mapView.onMouseMove(pos);
            }
            if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Left) {
                    sf::Vector2f pos((float)mb->position.x, (float)mb->position.y);
                    if (gameOver.visible()) gameOver.onClick(pos);
                    else if (appState == AppState::Menu) menu.onClick(pos);
                    else if (tutorialUI.visible()) tutorialUI.onClick(pos);
                    else if (modal.visible()) modal.onClick(pos);
                    else if (pos.x >= 1015 && pos.x <= 1125 && pos.y >= 12 && pos.y <= 48) {
                        // Click en el boton SIGUIENTE
                        doTick();
                    }
                    else if (currentTab == Tab::Action) actionPanel.onClick(pos);
                }
            }
            if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Escape) {
                    if (appState == AppState::Playing) appState = AppState::Menu;
                    else window.close();
                }
                if (kp->code == sf::Keyboard::Key::N) {
                    doTick();
                }
                if (kp->code == sf::Keyboard::Key::R) {
                    bridge.resetCountry(); dashboard.recordHistory(bridge.country());
                    audio.play("warning");
                }
                if (kp->code == sf::Keyboard::Key::M) {
                    audio.setEnabled(!audio.isEnabled());
                    lastActionFeedback = audio.isEnabled() ? ">> Audio ON" : ">> Audio OFF";
                }
                if (kp->code == sf::Keyboard::Key::L) {
                    std::string next = Localization::currentLanguage() == "en" ? "es" : "en";
                    Localization::load(LOCALES_DIR, next);
                    lastActionFeedback = ">> Idioma: " + next;
                }
                if (kp->code == sf::Keyboard::Key::Num1) currentTab = Tab::Dashboard;
                if (kp->code == sf::Keyboard::Key::Num2) currentTab = Tab::Map;
                if (kp->code == sf::Keyboard::Key::Num3) currentTab = Tab::Action;
                if (kp->code == sf::Keyboard::Key::Num4) currentTab = Tab::Decisions;
                if (kp->code == sf::Keyboard::Key::Num5) currentTab = Tab::Achievements;
                if (kp->code == sf::Keyboard::Key::Num6) currentTab = Tab::Court;
                if (kp->code == sf::Keyboard::Key::D) {
                    modal.show({"coup_threat",
                        "El alto mando amenaza con tomar el poder. ¿Tu respuesta?",
                        {"purge_military", "negotiate_military", "cede_power", "resist"}});
                    audio.play("decision_appears");
                    particles.emit(ParticleEmitter::Preset::RedSpark, 640.f, 400.f, 40);
                    timeline.addEvent(bridge.turn(), "Decision", sf::Color(80, 160, 240));
                }
                if (kp->code == sf::Keyboard::Key::G) {
                    gameOver.show(EndCondition::COUP_SUCCESS, bridge.country(),
                                  bridge.turn(), popularitySumDemo);
                    audio.play("game_over");
                    particles.emit(ParticleEmitter::Preset::GraySmoke, 640.f, 400.f, 30);
                }
                if (kp->code == sf::Keyboard::Key::T) {
                    tutorialUI.start();
                    audio.play("button_click");
                }
                if (kp->code == sf::Keyboard::Key::Space && tutorialUI.visible()) {
                    tutorialUI.advance();
                    audio.play("button_click");
                }
            }
        }

        float dt = frameClock.restart().asSeconds();
        // Interpolar paleta hacia el estado actual (~200ms para alcanzar el target).
        {
            Palette target = paletteForCountry(bridge.country());
            float k = dt / 0.2f; if (k > 1.f) k = 1.f;
            currentPalette = lerpPalette(currentPalette, target, k);
        }
        // Avanzar timers de animacion de turno.
        if (turnSweep > 0.f) { turnSweep -= dt; if (turnSweep < 0.f) turnSweep = 0.f; }
        if (turnShake > 0.f) { turnShake -= dt; if (turnShake < 0.f) turnShake = 0.f; }
        particles.update(dt);
        // Cinematic transitions: detectar cambios.
        if (currentTab != previousTab) {
            tabTransition = 0.25f;
            previousTab = currentTab;
        }
        if (tabTransition > 0.f) { tabTransition -= dt; if (tabTransition < 0.f) tabTransition = 0.f; }
        {
            bool mv = modal.visible();
            if (mv && !prevModalVisible) {
                modalOpenAnim = 0.20f;
            } else if (!mv && prevModalVisible) {
                modalCloseAnim = 0.25f;
            }
            prevModalVisible = mv;
            if (modalOpenAnim  > 0.f) { modalOpenAnim  -= dt; if (modalOpenAnim  < 0.f) modalOpenAnim  = 0.f; }
            if (modalCloseAnim > 0.f) { modalCloseAnim -= dt; if (modalCloseAnim < 0.f) modalCloseAnim = 0.f; }
        }
        {
            bool gv = gameOver.visible();
            if (gv && !prevGameOverVisible) gameOverFade = 0.f;
            if (gv) gameOverFade = std::min(1.f, gameOverFade + dt * 1.5f);
            else    gameOverFade = std::max(0.f, gameOverFade - dt * 1.5f);
            prevGameOverVisible = gv;
        }
        // Seleccionar ambient segun estado del pais.
        {
            const Country& cc = bridge.country();
            AudioSystem::Ambient amb = AudioSystem::Ambient::Calm;
            if (cc.politics.civil_war_active || cc.security.war_active || cc.welfare.pandemic_active) {
                amb = AudioSystem::Ambient::Crisis;
            } else if (cc.politics.popularity < 0.40 || cc.politics.popular_pressure > 0.6 ||
                       cc.economy.inflation > 0.12) {
                amb = AudioSystem::Ambient::Tension;
            }
            if (audio.currentAmbient() != amb) audio.setAmbient(amb);
            audio.updateAmbient(dt);
        }
        float shakeX = 0.f;
        if (turnShake > 0.f) {
            float phase = turnShake * 60.f;
            shakeX = std::sin(phase) * 2.f;
        }
        window.clear(currentPalette.bg);

        // === Menu state ===
        if (appState == AppState::Menu) {
            if (fontOk) menu.draw(window, font);
            window.display();
            continue;
        }

        // === TopBar ===
        window.draw(makePanel(shakeX, 0, 1280, 60, currentPalette.topbar));
        if (fontOk) {
            const Country& c = bridge.country();
            // Heraldica del pais a la izquierda del titulo.
            {
                unsigned seed = Heraldry::seedFromString("HomoPoliticusNation");
                Heraldry::draw(window, 24.f + shakeX, 30.f, 16.f, seed);
            }
            {
                sf::Text titleHdr(fTitle, "HOMO POLITICUS", 22);
                titleHdr.setFillColor(kAccent);
                titleHdr.setStyle(sf::Text::Bold);
                titleHdr.setPosition({48.f + shakeX, 18.f});
                window.draw(titleHdr);
            }
            std::ostringstream turnStr;
            turnStr << tr("ui.turn_prefix", "Turno") << " " << bridge.turn();
            window.draw(makeText(font, turnStr.str(), 18, kMuted, 290 + shakeX, 22));

            window.draw(makeText(font, tr("ui.pop_short", "Pop:"), 16, kMuted, 380 + shakeX, 25));
            window.draw(makeText(font, fmtPct(c.politics.popularity), 20, popularityColor(c.politics.popularity), 425 + shakeX, 22));
            drawProgressBar(window, 380 + shakeX, 47, 130, 6, c.politics.popularity, popularityColor(c.politics.popularity));

            window.draw(makeText(font, "GDP: " + fmtMoney(c.economy.gdp), 18, kText, 560 + shakeX, 22));
            std::ostringstream infStr;
            infStr << "Inflacion: " << std::fixed << std::setprecision(1) << (c.economy.inflation * 100) << "%";
            window.draw(makeText(font, infStr.str(), 18, kText, 760 + shakeX, 22));

            // Boton visible de Next turn (solo visual, la tecla N es la real)
            sf::RectangleShape nextBtn({110.f, 36.f});
            nextBtn.setPosition({1015.f + shakeX, 12.f});
            nextBtn.setFillColor(sf::Color(50, 100, 160));
            nextBtn.setOutlineColor(kAccent);
            nextBtn.setOutlineThickness(2.f);
            window.draw(nextBtn);
            window.draw(makeText(font, "[N] SIGUIENTE", 14, sf::Color(255,255,255), 1025 + shakeX, 21));

            window.draw(makeText(font, "1-5 tabs  D=decision  G=gameover  M=mute  L=lang", 11, kMuted, 1130 + shakeX, 22));
        }

        // === SidebarLeft ===
        window.draw(makePanel(0, 60, 200, 640, currentPalette.sidebar));
        if (fontOk) {
            window.draw(makeText(font, "SISTEMAS", 14, kMuted, 16, 76));
            const char* systems[] = {"Bienestar", "Economia", "Politica", "Seguridad", "Infraestructura"};
            for (int i = 0; i < 5; ++i) {
                window.draw(makeText(font, systems[i], 16, kText, 16, 104.f + i * 32));
            }
            window.draw(makeText(font, "ACCIONES", 14, kMuted, 16, 290));
            const char* actions[] = {"Dashboard", "Mapa", "Accion", "Decisiones", "Logros"};
            for (int i = 0; i < 5; ++i) {
                window.draw(makeText(font, actions[i], 16, kText, 16, 318.f + i * 32));
            }
        }

        // === MainPanel: tab activa ===
        window.draw(makePanel(200, 60, 830, 640, currentPalette.panel));
        mapView.update(dt);
        mapView.setTurn(bridge.turn());
        dashboard.update(dt, bridge.country());
        if (fontOk) {
            switch (currentTab) {
                case Tab::Dashboard: {
                    sf::Text hdr(fTitle, "DASHBOARD  [1]", 18);
                    hdr.setFillColor(kAccent); hdr.setStyle(sf::Text::Bold);
                    hdr.setPosition({220.f, 76.f}); window.draw(hdr);
                    dashboard.draw(window, font, bridge.country());
                    break;
                }
                case Tab::Map: {
                    sf::Text hdr(fTitle, "MAPA  [2]", 18);
                    hdr.setFillColor(kAccent); hdr.setStyle(sf::Text::Bold);
                    hdr.setPosition({220.f, 76.f}); window.draw(hdr);
                    mapView.draw(window, font, bridge.country());
                    break;
                }
                case Tab::Action: {
                    sf::Text hdr(fTitle, "ACCION  [3]", 18);
                    hdr.setFillColor(kAccent); hdr.setStyle(sf::Text::Bold);
                    hdr.setPosition({220.f, 76.f}); window.draw(hdr);
                    actionPanel.draw(window, font);
                    if (!lastActionFeedback.empty()) {
                        window.draw(makeText(font, lastActionFeedback, 13, kGood, 220, 96));
                    }
                    break;
                }
                case Tab::Decisions: {
                    sf::Text hdr(fTitle, "DECISIONES  [4]  (Sprint 14)", 18);
                    hdr.setFillColor(kAccent); hdr.setStyle(sf::Text::Bold);
                    hdr.setPosition({220.f, 76.f}); window.draw(hdr);
                    break;
                }
                case Tab::Achievements: {
                    sf::Text hdr(fTitle, "LOGROS  [5]  (Sprint futuro)", 18);
                    hdr.setFillColor(kAccent); hdr.setStyle(sf::Text::Bold);
                    hdr.setPosition({220.f, 76.f}); window.draw(hdr);
                    break;
                }
                case Tab::Court: {
                    sf::Text hdr(fTitle, "CORTE  [6]", 18);
                    hdr.setFillColor(kAccent); hdr.setStyle(sf::Text::Bold);
                    hdr.setPosition({220.f, 76.f}); window.draw(hdr);
                    // Palacio (escena) arriba + red abajo.
                    scene.update(dt);
                    if (modal.visible()) scene.setDialog("Una decision critica espera mi resolucion.");
                    else scene.clearDialog();
                    scene.draw(window, font, 218.f, 102.f, 794.f, 230.f, bridge.country());
                    court.update(dt);
                    court.draw(window, font, 218.f, 340.f, 794.f, 310.f);
                    if (court.hovered() >= 0) {
                        window.draw(makeText(font, court.hoveredDetail(), 12, kAccent, 220, 668));
                    }
                    break;
                }
            }
        }

        // === SidebarRight ===
        window.draw(makePanel(1030, 60, 250, 640, currentPalette.sidebar));
        if (fontOk) {
            // Retrato del presidente (detallado, con expresion + wardrobe contextual).
            window.draw(makeText(font, "PRESIDENCIA", 14, kMuted, 1046, 76));
            {
                const Country& cc = bridge.country();
                LeaderPortrait::Expression expr = LeaderPortrait::Expression::Neutral;
                if (cc.politics.popularity > 0.65) expr = LeaderPortrait::Expression::Happy;
                else if (cc.politics.popularity < 0.30) expr = LeaderPortrait::Expression::Angry;
                else if (cc.politics.popular_pressure > 0.6 || cc.security.war_active) expr = LeaderPortrait::Expression::Worried;
                // Color de uniforme segun regimen.
                sf::Color regimeAccent;
                if (cc.politics.auth_dem_axis > 0.6) {
                    regimeAccent = sf::Color(80, 30, 30);     // rojo oscuro autoritario
                } else if (cc.politics.civilian_military_control < 0.5) {
                    regimeAccent = sf::Color(60, 80, 50);     // verde militar
                } else {
                    regimeAccent = sf::Color(40, 60, 110);    // azul democratico
                }
                // En crisis economica/social: uniforme deslucido (oscurecido).
                bool inCrisis = (cc.economy.inflation > 0.15) || (cc.politics.popular_pressure > 0.7)
                              || cc.welfare.pandemic_active;
                if (inCrisis) {
                    regimeAccent.r = (uint8_t)(regimeAccent.r * 0.65f);
                    regimeAccent.g = (uint8_t)(regimeAccent.g * 0.65f);
                    regimeAccent.b = (uint8_t)(regimeAccent.b * 0.65f);
                }
                // Bonus de legitimacy si turno reciente fue eleccion ganada.
                float legit = (float)cc.politics.regime_legitimacy;
                if (bridge.turn() > 0 && bridge.turn() % 4 == 0 && cc.politics.popularity > 0.5) {
                    legit = std::min(1.f, legit + 0.25f); // medalla extra
                }
                LeaderPortrait::drawDetailed(window, font, "Presidente", "Mandato actual",
                                             1155.f, 158.f, 42.f, expr, regimeAccent, legit);
            }
            // Asesores (3 compactos).
            window.draw(makeText(font, "ASESORES", 14, kMuted, 1046, 248));
            const struct { const char* name; const char* role; } advisors[] = {
                {"Economia",  "Min. Hacienda"},
                {"Defensa",   "Min. Seguridad"},
                {"Gabinete",  "Jefe de Gab."},
            };
            for (int i = 0; i < 3; ++i) {
                float ax = 1062.f + (i % 3) * 70.f;
                float ay = 296.f;
                LeaderPortrait::drawCompact(window, font, advisors[i].name, ax, ay, 22.f);
                sf::Text nm(font, advisors[i].name, 10);
                nm.setFillColor(kText);
                auto lb = nm.getLocalBounds();
                nm.setOrigin({lb.position.x + lb.size.x / 2.f, 0.f});
                nm.setPosition({ax, ay + 28.f});
                window.draw(nm);
                sf::Text rl(font, advisors[i].role, 9);
                rl.setFillColor(kMuted);
                auto lb2 = rl.getLocalBounds();
                rl.setOrigin({lb2.position.x + lb2.size.x / 2.f, 0.f});
                rl.setPosition({ax, ay + 40.f});
                window.draw(rl);
            }
            window.draw(makeText(font, "EVENTOS", 14, kMuted, 1046, 372));
            window.draw(makeText(font, "(Sprint 11)", 12, kMuted, 1046, 392));
            window.draw(makeText(font, "DECISIONES PEND.", 14, kMuted, 1046, 478));
            window.draw(makeText(font, "(Sprint 14)", 12, kMuted, 1046, 498));
        }

        // === BottomBar: Mandate Timeline ===
        window.draw(makePanel(0, 700, 1280, 100, currentPalette.topbar));
        if (fontOk) {
            window.draw(makeText(font, "TIMELINE DEL MANDATO", 12, kMuted, 16, 706));
            timeline.draw(window, font, 16.f, 718.f, 1248.f, 76.f, bridge.turn(), 60);
        }

        // Particulas (encima del MainPanel pero debajo de modal/tutorial).
        particles.draw(window);

        // Tab transition: veil sobre MainPanel se desvanece.
        if (tabTransition > 0.f) {
            float t = tabTransition / 0.25f;  // 1 -> 0
            uint8_t alpha = (uint8_t)(160 * t);
            sf::RectangleShape veil({830.f, 640.f});
            veil.setPosition({200.f, 60.f});
            veil.setFillColor(sf::Color(20, 22, 30, alpha));
            window.draw(veil);
        }

        // Modal close flash dorado (despues de cerrar el modal).
        if (modalCloseAnim > 0.f) {
            float t = modalCloseAnim / 0.25f; // 1 -> 0
            uint8_t alpha = (uint8_t)(120 * t);
            sf::RectangleShape veil({1280.f, 800.f});
            veil.setPosition({0.f, 0.f});
            veil.setFillColor(sf::Color(240, 200, 90, alpha));
            window.draw(veil);
        }

        // Sweep effect: linea blanca cruza el MainPanel horizontalmente en 300ms.
        if (turnSweep > 0.f) {
            float progress = 1.f - (turnSweep / 0.3f);
            float sweepX = 200.f + (830.f) * progress;
            sf::RectangleShape line({3.f, 640.f});
            line.setPosition({sweepX, 60.f});
            uint8_t alpha = (uint8_t)(180 * (1.f - std::abs(progress - 0.5f) * 2.f));
            line.setFillColor(sf::Color(255, 255, 255, alpha));
            window.draw(line);
        }

        // Modal overlay (siempre al final para estar encima)
        if (fontOk) modal.draw(window, font);
        // Modal zoom-in: en los primeros 200ms del modal, mostrar veil de zoom.
        if (modalOpenAnim > 0.f && modal.visible()) {
            float t = modalOpenAnim / 0.20f; // 1 -> 0
            // Borde dorado pulsante expandiendose desde el centro.
            float side = 1280.f * (1.f - t);
            sf::RectangleShape ring({side, 800.f * (1.f - t)});
            ring.setOrigin({side * 0.5f, 800.f * (1.f - t) * 0.5f});
            ring.setPosition({640.f, 400.f});
            ring.setFillColor(sf::Color(0, 0, 0, 0));
            ring.setOutlineColor(sf::Color(240, 200, 90, (uint8_t)(180 * t)));
            ring.setOutlineThickness(4.f);
            window.draw(ring);
        }

        // Tutorial overlay (encima del modal pero debajo del game over)
        if (fontOk) tutorialUI.draw(window, font);

        // Game over screen (encima de TODO)
        gameOver.update(dt);
        if (fontOk) gameOver.draw(window, font);
        // Game over fade rojo: el veil rojo crece con gameOverFade.
        if (gameOverFade > 0.001f) {
            sf::RectangleShape veil({1280.f, 800.f});
            veil.setPosition({0.f, 0.f});
            veil.setFillColor(sf::Color(120, 20, 20, (uint8_t)(80 * gameOverFade)));
            window.draw(veil);
        }

        window.display();
    }
    return 0;
}
