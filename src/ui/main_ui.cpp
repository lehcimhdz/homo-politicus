#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
#include <sstream>
#include <iomanip>
#include "ui/UIBridge.hpp"
#include "ui/Dashboard.hpp"
#include "ui/MapView.hpp"
#include "ui/ActionPanel.hpp"
#include "ui/DecisionModal.hpp"
#include "ui/AudioSystem.hpp"
#include "ui/MainMenu.hpp"
#include "ui/GameOverScreen.hpp"
#include "ui/TutorialOverlay.hpp"
#include "Localization.hpp"

enum class AppState { Menu, Playing };
static const std::string LOCALES_DIR = "/Users/michelcano/Documents/Repositorios/homo-politicus-game/content/locales";
static std::string tr(const std::string& key, const std::string& fallback) {
    if (Localization::currentLanguage().empty()) return fallback;
    std::string v = Localization::tr(key);
    if (v.find("[missing:") != std::string::npos) return fallback;
    return v;
}
enum class Tab { Dashboard, Map, Action, Decisions, Achievements };

// Layout (1280x800):
//   TopBar:    1280 x 60
//   SidebarL:  200 x 640 (60..700)
//   MainPanel: 830 x 640
//   SidebarR:  250 x 640
//   BottomBar: 1280 x 100 (700..800)

static const sf::Color kBg     = sf::Color(20, 22, 30);
static const sf::Color kPanel  = sf::Color(32, 36, 48);
static const sf::Color kBorder = sf::Color(60, 65, 80);
static const sf::Color kText   = sf::Color(220, 222, 232);
static const sf::Color kMuted  = sf::Color(150, 154, 168);
static const sf::Color kAccent = sf::Color(80, 160, 240);
static const sf::Color kGood   = sf::Color(80, 200, 120);
static const sf::Color kBad    = sf::Color(220, 80, 80);
static const sf::Color kWarn   = sf::Color(240, 180, 60);

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

static sf::RectangleShape makePanel(float x, float y, float w, float h, sf::Color fill = kPanel) {
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

    Localization::load(LOCALES_DIR, "es");
    UIBridge bridge;
    Dashboard dashboard;
    MapView mapView;
    ActionPanel actionPanel;
    DecisionModal modal;
    AudioSystem audio;
    MainMenu menu;
    GameOverScreen gameOver;
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
                        bridge.tick();
                        dashboard.recordHistory(bridge.country());
                        popularitySumDemo += bridge.country().politics.popularity;
                        audio.play("turn_advance");
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
                    bridge.tick(); dashboard.recordHistory(bridge.country());
                    popularitySumDemo += bridge.country().politics.popularity;
                    audio.play("turn_advance");
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
                if (kp->code == sf::Keyboard::Key::D) {
                    modal.show({"coup_threat",
                        "El alto mando amenaza con tomar el poder. ¿Tu respuesta?",
                        {"purge_military", "negotiate_military", "cede_power", "resist"}});
                    audio.play("decision_appears");
                }
                if (kp->code == sf::Keyboard::Key::G) {
                    gameOver.show(EndCondition::COUP_SUCCESS, bridge.country(),
                                  bridge.turn(), popularitySumDemo);
                    audio.play("game_over");
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

        window.clear(kBg);

        // === Menu state ===
        if (appState == AppState::Menu) {
            if (fontOk) menu.draw(window, font);
            window.display();
            continue;
        }

        // === TopBar ===
        window.draw(makePanel(0, 0, 1280, 60));
        if (fontOk) {
            const Country& c = bridge.country();
            window.draw(makeText(font, "HOMO POLITICUS", 24, kAccent, 16, 16));
            std::ostringstream turnStr;
            turnStr << tr("ui.turn_prefix", "Turno") << " " << bridge.turn();
            window.draw(makeText(font, turnStr.str(), 18, kMuted, 240, 22));

            window.draw(makeText(font, tr("ui.pop_short", "Pop:"), 16, kMuted, 380, 25));
            window.draw(makeText(font, fmtPct(c.politics.popularity), 20, popularityColor(c.politics.popularity), 425, 22));
            drawProgressBar(window, 380, 47, 130, 6, c.politics.popularity, popularityColor(c.politics.popularity));

            window.draw(makeText(font, "GDP: " + fmtMoney(c.economy.gdp), 18, kText, 560, 22));
            std::ostringstream infStr;
            infStr << "Inflacion: " << std::fixed << std::setprecision(1) << (c.economy.inflation * 100) << "%";
            window.draw(makeText(font, infStr.str(), 18, kText, 760, 22));

            // Boton visible de Next turn (solo visual, la tecla N es la real)
            sf::RectangleShape nextBtn({110.f, 36.f});
            nextBtn.setPosition({1015.f, 12.f});
            nextBtn.setFillColor(sf::Color(50, 100, 160));
            nextBtn.setOutlineColor(kAccent);
            nextBtn.setOutlineThickness(2.f);
            window.draw(nextBtn);
            window.draw(makeText(font, "[N] SIGUIENTE", 14, sf::Color(255,255,255), 1025, 21));

            window.draw(makeText(font, "1-5 tabs  D=decision  G=gameover  M=mute  L=lang", 11, kMuted, 1130, 22));
        }

        // === SidebarLeft ===
        window.draw(makePanel(0, 60, 200, 640));
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
        window.draw(makePanel(200, 60, 830, 640));
        float dt = frameClock.restart().asSeconds();
        mapView.update(dt);
        if (fontOk) {
            switch (currentTab) {
                case Tab::Dashboard:
                    window.draw(makeText(font, "DASHBOARD  [1]", 18, kAccent, 220, 76));
                    dashboard.draw(window, font, bridge.country());
                    break;
                case Tab::Map:
                    window.draw(makeText(font, "MAPA  [2]", 18, kAccent, 220, 76));
                    mapView.draw(window, font, bridge.country());
                    break;
                case Tab::Action:
                    window.draw(makeText(font, "ACCION  [3]", 18, kAccent, 220, 76));
                    actionPanel.draw(window, font);
                    if (!lastActionFeedback.empty()) {
                        window.draw(makeText(font, lastActionFeedback, 13, kGood, 220, 96));
                    }
                    break;
                case Tab::Decisions:
                    window.draw(makeText(font, "DECISIONES  [4]  (Sprint 14)", 18, kAccent, 220, 76));
                    break;
                case Tab::Achievements:
                    window.draw(makeText(font, "LOGROS  [5]  (Sprint futuro)", 18, kAccent, 220, 76));
                    break;
            }
        }

        // === SidebarRight ===
        window.draw(makePanel(1030, 60, 250, 640));
        if (fontOk) {
            window.draw(makeText(font, "EVENTOS", 14, kMuted, 1046, 76));
            window.draw(makeText(font, "(Sprint 11)", 12, kMuted, 1046, 96));
            window.draw(makeText(font, "DECISIONES PENDIENTES", 14, kMuted, 1046, 250));
            window.draw(makeText(font, "(Sprint 14)", 12, kMuted, 1046, 270));
        }

        // === BottomBar ===
        window.draw(makePanel(0, 700, 1280, 100));
        if (fontOk) {
            window.draw(makeText(font, "LOG DE TURNO", 14, kMuted, 16, 712));
            window.draw(makeText(font, "(Sprint 11 conectara feedback al area de log)", 14, kMuted, 16, 736));
        }

        // Modal overlay (siempre al final para estar encima)
        if (fontOk) modal.draw(window, font);

        // Tutorial overlay (encima del modal pero debajo del game over)
        if (fontOk) tutorialUI.draw(window, font);

        // Game over screen (encima de TODO)
        gameOver.update(dt);
        if (fontOk) gameOver.draw(window, font);

        window.display();
    }
    return 0;
}
