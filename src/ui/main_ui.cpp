#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <unordered_set>
#include "DecisionSystem.hpp"

// Suprime stdout durante el scope (los logs del motor son ruido en UI).
namespace {
struct CoutSuppressor {
    std::streambuf* old;
    CoutSuppressor() : old(std::cout.rdbuf(nullptr)) {}
    ~CoutSuppressor() { std::cout.rdbuf(old); }
};
}
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
#include "ui/AssetManager.hpp"
#include "ui/AchievementsView.hpp"
#include "AchievementTracker.hpp"
#include "Localization.hpp"
#include "EventManager.hpp"
#include "Persistence.hpp"
#include "GameMode.hpp"
#include "IronManMode.hpp"
#include "SuccessorMode.hpp"
#include "Advisor.hpp"
#include "llm/LLMProvider.hpp"
#include "ModLoader.hpp"

enum class AppState { Menu, ModeSelect, Settings, Playing };
static const std::string LOCALES_DIR = "/Users/michelcano/Documents/Repositorios/homo-politicus-game/content/locales";
static std::string tr(const std::string& key, const std::string& fallback) {
    if (Localization::currentLanguage().empty()) return fallback;
    std::string v = Localization::tr(key);
    if (v.find("[missing:") != std::string::npos) return fallback;
    return v;
}
enum class Tab { Dashboard, Map, Action, Decisions, Achievements, Court, Advisors };

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
    AssetManager::instance().preloadDefaults();
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
    AchievementTracker achievementTracker;
    AchievementsView achievements;
    achievements.configure(achievementTracker);
    EventManager eventMgr;
    double initialGdp = bridge.country().economy.gdp;
    ModeConfig modeConfig;
    int modeSelectHover = -1;
    bool ironManToggled = false;
    auto llmProvider = LLMFactory::create();  // Anthropic si env, sino Mock
    auto advisorList = Advisors::all();
    int advisorHover = -1;
    std::string advisorLastResponse;
    std::string advisorLastWho;
    // Settings state.
    int settingsHover = -1;
    std::vector<ModLoader::ModMetadata> detectedMods;
    bool modsScanned = false;
    // Scenario picker state (modo Historical).
    bool showingScenarioPicker = false;
    int scenarioPickerHover = -1;
    std::vector<std::string> historicalScenarios = {
        "argentina_2001", "brasil_1985", "cuba_1959", "chile_1973",
        "mexico_1968", "espana_1936", "alemania_1933", "italia_1922",
        "francia_1789", "rusia_1917", "venezuela_1999",
    };
    std::string selectedScenario;
    TutorialOverlay tutorialUI;
    double popularitySumDemo = 0.0;
    int gameSeed = 1;  // incrementa con cada Nueva Partida (rota retratos)
    // Log de eventos para sidebar (los ultimos 6, mas reciente primero).
    struct EventLogEntry { int turn; std::string label; sf::Color color; };
    std::vector<EventLogEntry> eventLog;
    auto pushEvent = [&](const std::string& lbl, sf::Color col) {
        eventLog.insert(eventLog.begin(), {bridge.turn(), lbl, col});
        if (eventLog.size() > 12) eventLog.resize(12);
    };
    // Cola de decisiones pendientes.
    std::vector<PendingDecision> decisionQueue;
    AppState appState = AppState::Menu;
    gameOver.setCallback([&](GameOverScreen::Action a) {
        audio.play("button_click");
        if (a == GameOverScreen::Action::NewGame) {
            bridge.resetCountry();
            dashboard.recordHistory(bridge.country());
            popularitySumDemo = 0.0;
            ++gameSeed;
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
                ++gameSeed;
                appState = AppState::ModeSelect;
                break;
            case MainMenu::Action::Continue:
                bridge.resetCountry();
                dashboard.recordHistory(bridge.country());
                appState = AppState::Playing;
                break;
            case MainMenu::Action::Settings:
                appState = AppState::Settings;
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
    // Auto-tick speeds (Paradox/RimWorld style).
    enum class PlaySpeed { Paused, X1, X2, X4 };
    PlaySpeed playSpeed = PlaySpeed::X1;        // C20.3: default 1x para que se sienta vivo
    PlaySpeed lastNonPausedSpeed = PlaySpeed::X1;
    bool autoPausedBecauseOfDecisions = false;
    float autoTickAcc = 0.f;
    auto intervalForSpeed = [](PlaySpeed s) -> float {
        switch (s) {
            case PlaySpeed::X1: return 3.0f;
            case PlaySpeed::X2: return 1.5f;
            case PlaySpeed::X4: return 0.75f;
            default: return 1e9f;
        }
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
        // Disparar eventos del motor.
        eventMgr.triggerRandomEvent(bridge.mutableCountry());
        // Sprint C20.1: drenar decisiones que el motor real puso en su cola
        // hacia la cola visible del UI. Sin esto el jugador no ve lo que el
        // motor le pide resolver.
        {
            auto& gameDecs = bridge.game().pendingDecisionsRef();
            for (auto& d : gameDecs) {
                DecisionSystem::enqueueOnce(decisionQueue, d);
                pushEvent("Nueva decision: " + d.id, sf::Color(220, 180, 80));
            }
            gameDecs.clear();
        }
        // Sprint C20.4: emitir hasta 3 mensajes criticos del motor al sidebar.
        {
            auto msgs = bridge.drainCriticalMessages();
            int shown = 0;
            for (const auto& m : msgs) {
                if (shown >= 3) break;
                std::string clean = m;
                // Limpiar prefijo de marcador para mostrar.
                sf::Color msgColor(220, 200, 80);
                if (clean.find("[!!!]") != std::string::npos) {
                    msgColor = sf::Color(220, 90, 80);
                    auto p = clean.find("[!!!]");
                    clean = clean.substr(p + 6);
                } else if (clean.find("[!!]") != std::string::npos) {
                    msgColor = sf::Color(240, 160, 80);
                    auto p = clean.find("[!!]");
                    clean = clean.substr(p + 5);
                } else if (clean.find("[INFO]") != std::string::npos) {
                    msgColor = sf::Color(150, 200, 220);
                    auto p = clean.find("[INFO]");
                    clean = clean.substr(p + 7);
                }
                if (clean.size() > 56) clean = clean.substr(0, 54) + "..";
                if (!clean.empty()) pushEvent(clean, msgColor);
                ++shown;
            }
        }
        // Achievement tracking.
        std::unordered_set<std::string> prevUnlocked;
        for (const auto& s : achievementTracker.unlockedList()) prevUnlocked.insert(s);
        achievementTracker.recordHistory(bridge.country());
        achievementTracker.evaluate(bridge.country(), bridge.turn(),
                                    bridge.endCondition(), initialGdp);
        // Reportar nuevos unlocks.
        for (const auto& id : achievementTracker.unlockedList()) {
            if (prevUnlocked.find(id) == prevUnlocked.end()) {
                pushEvent("LOGRO: " + id, sf::Color(240, 200, 80));
                audio.play("achievement");
            }
        }
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
        // Registrar eventos clave en el timeline + sidebar log.
        if (bridge.turn() % 4 == 0 && bridge.turn() > 0) {
            timeline.addEvent(bridge.turn(), "Eleccion", sf::Color(80, 200, 120));
            pushEvent("Eleccion legislativa", sf::Color(80, 200, 120));
        }
        if (dPop < -0.05) {
            timeline.addEvent(bridge.turn(), "Caida pop.", sf::Color(220, 80, 80));
            pushEvent("Caida en popularidad", sf::Color(220, 80, 80));
        } else if (dPop > 0.05) {
            timeline.addEvent(bridge.turn(), "Repunte", sf::Color(80, 200, 120));
            pushEvent("Repunte popular", sf::Color(80, 200, 120));
        }
        // Eventos por estado del pais.
        if (cc.economy.inflation > 0.15) {
            pushEvent("Crisis inflacionaria", sf::Color(220, 180, 60));
            DecisionSystem::enqueueOnce(decisionQueue, {
                "monetary_crisis",
                "La inflacion supera el 15%. La gente exige medidas.",
                {"subir_tasa", "control_precios", "no_actuar"}
            });
        }
        if (cc.politics.popular_pressure > 0.7) {
            pushEvent("Presion popular critica", sf::Color(220, 80, 80));
            DecisionSystem::enqueueOnce(decisionQueue, {
                "popular_pressure",
                "La presion popular es altisima. ?Como responder?",
                {"dialogo", "represion", "concesiones", "ignorar"}
            });
        }
        if (cc.politics.military_pressure > 0.7) {
            pushEvent("Tension con las FFAA", sf::Color(200, 90, 80));
            DecisionSystem::enqueueOnce(decisionQueue, {
                "coup_threat",
                "El alto mando amenaza con tomar el poder. ?Tu respuesta?",
                {"purgar", "negociar", "ceder", "resistir"}
            });
        }
        if (cc.welfare.pandemic_active) {
            pushEvent("Pandemia activa", sf::Color(180, 120, 220));
        }
        if (cc.security.war_active) {
            pushEvent("Guerra externa", sf::Color(220, 60, 60));
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
        // Sprint C20.2: aplicar la decision al motor real.
        {
            CoutSuppressor suppress;
            bridge.game().resolveDecisionPublic(choice);
        }
        pushEvent("Resuelto: " + choice, sf::Color(80, 200, 120));
        modal.hide();
    });
    modal.setSkipCallback([&]() {
        lastActionFeedback = ">> Decision saltada (-credibilidad)";
        audio.play("warning");
        // Resolver como "skip" en el motor.
        {
            CoutSuppressor suppress;
            bridge.game().resolveDecisionPublic("skip");
        }
        pushEvent("Decision saltada (-credibilidad)", sf::Color(220, 100, 80));
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
                else if (appState == AppState::ModeSelect) {
                    if (showingScenarioPicker) {
                        scenarioPickerHover = -1;
                        for (int i = 0; i < (int)historicalScenarios.size(); ++i) {
                            int col = i % 3, row = i / 3;
                            float cx = 220.f + (float)col * 290.f;
                            float cy = 180.f + (float)row * 110.f;
                            if (pos.x >= cx && pos.x <= cx + 270.f && pos.y >= cy && pos.y <= cy + 90.f) {
                                scenarioPickerHover = i;
                                break;
                            }
                        }
                    } else {
                        modeSelectHover = -1;
                        for (int i = 0; i < 3; ++i) {
                            float cx = 220.f + (float)i * 290.f;
                            if (pos.x >= cx && pos.x <= cx + 270.f && pos.y >= 220.f && pos.y <= 570.f) {
                                modeSelectHover = i;
                                break;
                            }
                        }
                    }
                }
                else if (appState == AppState::Settings) {
                    settingsHover = (pos.x >= 550.f && pos.x <= 730.f && pos.y >= 720.f && pos.y <= 770.f) ? 999 : -1;
                }
                else if (modal.visible()) modal.onMouseMove(pos);
                else if (currentTab == Tab::Action) actionPanel.onMouseMove(pos);
                else if (currentTab == Tab::Dashboard) dashboard.onMouseMove(pos);
                else if (currentTab == Tab::Court) court.onMouseMove(pos);
                else if (currentTab == Tab::Map) mapView.onMouseMove(pos);
                else if (currentTab == Tab::Achievements) achievements.onMouseMove(pos);
                else if (currentTab == Tab::Advisors) {
                    advisorHover = -1;
                    for (int i = 0; i < (int)advisorList.size() && i < 5; ++i) {
                        float ay = 130.f + (float)i * 96.f;
                        if (pos.x >= 220.f && pos.x <= 1000.f && pos.y >= ay && pos.y <= ay + 84.f) {
                            advisorHover = i;
                            break;
                        }
                    }
                }
            }
            if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Left) {
                    sf::Vector2f pos((float)mb->position.x, (float)mb->position.y);
                    if (gameOver.visible()) gameOver.onClick(pos);
                    else if (appState == AppState::Menu) menu.onClick(pos);
                    else if (appState == AppState::ModeSelect) {
                        if (showingScenarioPicker) {
                            // Click en alguno de los 11 escenarios.
                            for (int i = 0; i < (int)historicalScenarios.size(); ++i) {
                                int col = i % 3, row = i / 3;
                                float cx = 220.f + (float)col * 290.f;
                                float cy = 180.f + (float)row * 110.f;
                                if (pos.x >= cx && pos.x <= cx + 270.f && pos.y >= cy && pos.y <= cy + 90.f) {
                                    selectedScenario = historicalScenarios[i];
                                    modeConfig.scenarioId = selectedScenario;
                                    if (ironManToggled) IronManMode::enable();
                                    else IronManMode::disable();
                                    tutorialUI.start();
                                    appState = AppState::Playing;
                                    showingScenarioPicker = false;
                                    audio.play("button_click");
                                    pushEvent("Escenario: " + selectedScenario, sf::Color(80, 160, 240));
                                    break;
                                }
                            }
                        } else {
                            const GameMode modes[] = {GameMode::Sandbox, GameMode::Missions, GameMode::Historical};
                            for (int i = 0; i < 3; ++i) {
                                float cx = 220.f + (float)i * 290.f;
                                if (pos.x >= cx && pos.x <= cx + 270.f && pos.y >= 220.f && pos.y <= 570.f) {
                                    modeConfig.mode = modes[i];
                                    if (modes[i] == GameMode::Historical) {
                                        showingScenarioPicker = true;
                                        audio.play("button_click");
                                    } else {
                                        if (ironManToggled) IronManMode::enable();
                                        else IronManMode::disable();
                                        tutorialUI.start();
                                        appState = AppState::Playing;
                                        audio.play("button_click");
                                    }
                                    break;
                                }
                            }
                            // Click en iron man toggle.
                            if (pos.x >= 430.f && pos.x <= 850.f && pos.y >= 600.f && pos.y <= 660.f) {
                                ironManToggled = !ironManToggled;
                                audio.play("button_click");
                            }
                        }
                    }
                    else if (appState == AppState::Settings) {
                        // Click en volumen slider (track 280..1000, y 186..200).
                        if (pos.x >= 280.f && pos.x <= 1000.f && pos.y >= 184.f && pos.y <= 204.f) {
                            float pct = (pos.x - 280.f) / 720.f;
                            audio.setVolume(pct * 100.f);
                        }
                        // Click en idioma.
                        if (pos.y >= 280.f && pos.y <= 316.f) {
                            const char* langs[] = {"es", "en", "pt", "fr", "de", "it"};
                            for (int i = 0; i < 6; ++i) {
                                float bx = 280.f + (float)i * 120.f;
                                if (pos.x >= bx && pos.x <= bx + 110.f) {
                                    Localization::load(LOCALES_DIR, langs[i]);
                                    audio.play("button_click");
                                    break;
                                }
                            }
                        }
                        // Click en Volver.
                        if (pos.x >= 550.f && pos.x <= 730.f && pos.y >= 720.f && pos.y <= 770.f) {
                            appState = AppState::Menu;
                            audio.play("button_click");
                        }
                    }
                    else if (tutorialUI.visible()) tutorialUI.onClick(pos);
                    else if (modal.visible()) modal.onClick(pos);
                    else if (pos.x >= 1015 && pos.x <= 1105 && pos.y >= 12 && pos.y <= 48) {
                        // Click en el boton SIGUIENTE
                        doTick();
                    }
                    else if (pos.y >= 15 && pos.y <= 45) {
                        // Botones de velocidad.
                        struct { PlaySpeed s; float x; } sbtns[4] = {
                            {PlaySpeed::Paused, 890.f},
                            {PlaySpeed::X1,     920.f},
                            {PlaySpeed::X2,     950.f},
                            {PlaySpeed::X4,     980.f},
                        };
                        for (auto& sb : sbtns) {
                            if (pos.x >= sb.x && pos.x <= sb.x + 26.f) {
                                playSpeed = sb.s;
                                if (playSpeed != PlaySpeed::Paused) lastNonPausedSpeed = playSpeed;
                                audio.play("button_click");
                                break;
                            }
                        }
                    }
                    else if (pos.x >= 8 && pos.x <= 192 && pos.y >= 314 && pos.y <= 538) {
                        // Click en SidebarLeft ACCIONES (7 items).
                        int idx = (int)((pos.y - 314.f) / 32.f);
                        const Tab tabFor[] = {Tab::Dashboard, Tab::Map, Tab::Action, Tab::Decisions,
                                              Tab::Achievements, Tab::Court, Tab::Advisors};
                        if (idx >= 0 && idx < 7) currentTab = tabFor[idx];
                        audio.play("button_click");
                    }
                    else if (currentTab == Tab::Action) actionPanel.onClick(pos);
                    else if (currentTab == Tab::Achievements) achievements.onClick(pos);
                    else if (currentTab == Tab::Advisors) {
                        for (int i = 0; i < (int)advisorList.size() && i < 5; ++i) {
                            float ay = 130.f + (float)i * 96.f;
                            // Click en boton Consultar.
                            if (pos.x >= 858.f && pos.x <= 998.f && pos.y >= ay + 26.f && pos.y <= ay + 56.f) {
                                advisorLastWho = advisorList[i]->id();
                                advisorLastResponse = advisorList[i]->respondWithLLM(
                                    llmProvider.get(), bridge.country(), "");
                                pushEvent("Consulta a " + advisorList[i]->name_es(),
                                          sf::Color(80, 160, 240));
                                audio.play("button_click");
                                break;
                            }
                        }
                    }
                }
            }
            if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Escape) {
                    if (appState == AppState::Settings) appState = AppState::Menu;
                    else if (appState == AppState::ModeSelect) {
                        if (showingScenarioPicker) showingScenarioPicker = false;
                        else appState = AppState::Menu;
                    }
                    else if (appState == AppState::Playing) appState = AppState::Menu;
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
                if (kp->code == sf::Keyboard::Key::Num7) currentTab = Tab::Advisors;
                if (kp->code == sf::Keyboard::Key::D) {
                    PendingDecision next;
                    if (!decisionQueue.empty()) {
                        next = decisionQueue.front();
                        decisionQueue.erase(decisionQueue.begin());
                    } else {
                        next = {"coup_threat",
                            "El alto mando amenaza con tomar el poder. ¿Tu respuesta?",
                            {"purgar", "negociar", "ceder", "resistir"}};
                    }
                    modal.show(next);
                    audio.play("decision_appears");
                    particles.emit(ParticleEmitter::Preset::RedSpark, 640.f, 400.f, 40);
                    timeline.addEvent(bridge.turn(), "Decision", sf::Color(80, 160, 240));
                    pushEvent("Decision tomada: " + next.id, sf::Color(80, 160, 240));
                }
                if (kp->code == sf::Keyboard::Key::G) {
                    gameOver.show(EndCondition::COUP_SUCCESS, bridge.country(),
                                  bridge.turn(), popularitySumDemo);
                    audio.play("game_over");
                    particles.emit(ParticleEmitter::Preset::GraySmoke, 640.f, 400.f, 30);
                }
                if (kp->code == sf::Keyboard::Key::U) {
                    // Continuar como Sucesor (tecla U) si elegible.
                    if (gameOver.visible() && SuccessorMode::isEligible(EndCondition::TERM_COMPLETED)) {
                        SuccessorMode::applyTo(bridge.mutableCountry(), EndCondition::TERM_COMPLETED);
                        popularitySumDemo = 0.0;
                        gameOver.hide();
                        appState = AppState::Playing;
                        pushEvent("Continuando como sucesor", sf::Color(80, 200, 120));
                        audio.play("achievement");
                    }
                }
                if (kp->code == sf::Keyboard::Key::T) {
                    tutorialUI.start();
                    audio.play("button_click");
                }
                if (kp->code == sf::Keyboard::Key::S) {
                    if (!IronManMode::isManualSaveAllowed()) {
                        pushEvent("IRON MAN: save manual bloqueado", sf::Color(220, 90, 80));
                        audio.play("warning");
                    } else {
                        bool ok = Persistence::save(bridge.country(), bridge.turn(),
                                                    popularitySumDemo, bridge.endCondition(),
                                                    "save_slot1.txt",
                                                    achievementTracker.serialize());
                        pushEvent(ok ? "Partida guardada (slot 1)" : "Error al guardar",
                                  ok ? sf::Color(80, 200, 120) : sf::Color(220, 80, 80));
                        audio.play("button_click");
                    }
                }
                if (kp->code == sf::Keyboard::Key::P) {
                    if (!IronManMode::isManualLoadAllowed()) {
                        pushEvent("IRON MAN: load manual bloqueado", sf::Color(220, 90, 80));
                        audio.play("warning");
                    } else {
                        Country tmp; int t = 0; double pSum = 0;
                        EndCondition end = EndCondition::NONE;
                        std::string achLine;
                        bool ok = Persistence::load(tmp, t, pSum, end, "save_slot1.txt", &achLine);
                        if (ok) {
                            bridge.mutableCountry() = tmp;
                            popularitySumDemo = pSum;
                            achievementTracker.deserialize(achLine);
                            achievements.update(achievementTracker);
                            pushEvent("Partida cargada (slot 1)", sf::Color(80, 200, 120));
                        } else {
                            pushEvent("Error al cargar (no existe slot)", sf::Color(220, 80, 80));
                        }
                        audio.play("button_click");
                    }
                }
                if (kp->code == sf::Keyboard::Key::Space) {
                    if (tutorialUI.visible()) {
                        tutorialUI.advance();
                        audio.play("button_click");
                    } else if (appState == AppState::Playing) {
                        // Toggle pausa <-> ultima velocidad usada.
                        if (playSpeed == PlaySpeed::Paused) {
                            playSpeed = lastNonPausedSpeed;
                        } else {
                            lastNonPausedSpeed = playSpeed;
                            playSpeed = PlaySpeed::Paused;
                        }
                        audio.play("button_click");
                    }
                }
                if (kp->code == sf::Keyboard::Key::Add ||
                    kp->code == sf::Keyboard::Key::Equal) {
                    if (playSpeed == PlaySpeed::Paused) playSpeed = PlaySpeed::X1;
                    else if (playSpeed == PlaySpeed::X1) playSpeed = PlaySpeed::X2;
                    else if (playSpeed == PlaySpeed::X2) playSpeed = PlaySpeed::X4;
                    if (playSpeed != PlaySpeed::Paused) lastNonPausedSpeed = playSpeed;
                    audio.play("button_click");
                }
                if (kp->code == sf::Keyboard::Key::Hyphen ||
                    kp->code == sf::Keyboard::Key::Subtract) {
                    if (playSpeed == PlaySpeed::X4) playSpeed = PlaySpeed::X2;
                    else if (playSpeed == PlaySpeed::X2) playSpeed = PlaySpeed::X1;
                    else if (playSpeed == PlaySpeed::X1) playSpeed = PlaySpeed::Paused;
                    if (playSpeed != PlaySpeed::Paused) lastNonPausedSpeed = playSpeed;
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
        // Auto-tick (Sprint C19.2 + C20.3): si playSpeed != Paused y no hay
        // modal, acumular dt. Auto-pausa si hay 3+ decisiones pendientes.
        if (decisionQueue.size() >= 3 && playSpeed != PlaySpeed::Paused
            && !autoPausedBecauseOfDecisions) {
            lastNonPausedSpeed = playSpeed;
            playSpeed = PlaySpeed::Paused;
            autoPausedBecauseOfDecisions = true;
            pushEvent("PAUSA: 3+ decisiones pendientes. Resolve con [D] o tab [4].",
                      sf::Color(240, 200, 80));
        }
        if (decisionQueue.empty()) autoPausedBecauseOfDecisions = false;

        if (appState == AppState::Playing && playSpeed != PlaySpeed::Paused
            && !modal.visible() && !gameOver.visible() && !tutorialUI.visible()) {
            autoTickAcc += dt;
            float interval = intervalForSpeed(playSpeed);
            while (autoTickAcc >= interval) {
                autoTickAcc -= interval;
                doTick();
            }
        } else {
            autoTickAcc = 0.f;
        }
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

        // === Settings state ===
        if (appState == AppState::Settings && fontOk) {
            window.draw(makePanel(0, 0, 1280, 800, sf::Color(15, 17, 26)));
            sf::Text title(fTitle, "CONFIGURACION", 48);
            title.setStyle(sf::Text::Bold);
            title.setFillColor(sf::Color(80, 160, 240));
            auto lb = title.getLocalBounds();
            title.setOrigin({lb.position.x + lb.size.x / 2.f, 0.f});
            title.setPosition({640.f, 60.f});
            window.draw(title);

            // Audio volume slider.
            window.draw(makeText(font, "VOLUMEN AUDIO", 14, kMuted, 280, 160));
            float volTrackX = 280.f, volTrackY = 190.f, volTrackW = 720.f, volTrackH = 8.f;
            sf::RectangleShape track({volTrackW, volTrackH});
            track.setPosition({volTrackX, volTrackY});
            track.setFillColor(sf::Color(40, 44, 60));
            track.setOutlineColor(sf::Color(80, 90, 120));
            track.setOutlineThickness(1.f);
            window.draw(track);
            float volPct = audio.volume() / 100.f;
            sf::RectangleShape fill({volTrackW * volPct, volTrackH});
            fill.setPosition({volTrackX, volTrackY});
            fill.setFillColor(sf::Color(80, 160, 240));
            window.draw(fill);
            sf::CircleShape knob(10.f);
            knob.setOrigin({10.f, 10.f});
            knob.setPosition({volTrackX + volTrackW * volPct, volTrackY + volTrackH / 2.f});
            knob.setFillColor(sf::Color(180, 210, 255));
            knob.setOutlineColor(sf::Color(80, 160, 240));
            knob.setOutlineThickness(2.f);
            window.draw(knob);
            std::ostringstream volLabel;
            volLabel << (int)audio.volume() << " / 100";
            window.draw(makeText(font, volLabel.str(), 12, kText, 1020, 184));

            // Language picker.
            window.draw(makeText(font, "IDIOMA", 14, kMuted, 280, 250));
            const char* langs[] = {"es", "en", "pt", "fr", "de", "it"};
            const char* langNames[] = {"Espanol", "English", "Portugues", "Francais", "Deutsch", "Italiano"};
            for (int i = 0; i < 6; ++i) {
                float bx = 280.f + (float)i * 120.f;
                bool active = (Localization::currentLanguage() == langs[i]);
                sf::RectangleShape btn({110.f, 36.f});
                btn.setPosition({bx, 280.f});
                btn.setFillColor(active ? sf::Color(60, 100, 160) : sf::Color(40, 44, 60));
                btn.setOutlineColor(active ? sf::Color(180, 210, 255) : sf::Color(80, 90, 120));
                btn.setOutlineThickness(active ? 2.f : 1.f);
                window.draw(btn);
                sf::Text lt(font, langNames[i], 13);
                lt.setStyle(active ? sf::Text::Bold : sf::Text::Regular);
                lt.setFillColor(active ? sf::Color(245, 250, 255) : sf::Color(200, 205, 220));
                auto llb = lt.getLocalBounds();
                lt.setOrigin({llb.position.x + llb.size.x / 2.f, llb.position.y + llb.size.y / 2.f});
                lt.setPosition({bx + 55.f, 298.f});
                window.draw(lt);
            }

            // Mods detectados.
            if (!modsScanned) {
                detectedMods = ModLoader::scan("mods/");
                if (detectedMods.empty()) {
                    detectedMods = ModLoader::scan("/Users/michelcano/Documents/Repositorios/homo-politicus-game/mods/");
                }
                modsScanned = true;
            }
            window.draw(makeText(font, "MODS DETECTADOS", 14, kMuted, 280, 360));
            if (detectedMods.empty()) {
                window.draw(makeText(font, "(Ninguno. Crear mod en mods/<id>/mod.yaml)",
                                     12, kMuted, 280, 384));
            } else {
                for (size_t i = 0; i < detectedMods.size() && i < 5; ++i) {
                    const auto& m = detectedMods[i];
                    float my = 384.f + (float)i * 60.f;
                    sf::RectangleShape mc({720.f, 50.f});
                    mc.setPosition({280.f, my});
                    mc.setFillColor(m.enabled ? sf::Color(38, 56, 42) : sf::Color(32, 32, 38));
                    mc.setOutlineColor(m.enabled ? sf::Color(80, 200, 120) : sf::Color(80, 90, 110));
                    mc.setOutlineThickness(1.5f);
                    window.draw(mc);
                    sf::Text mn(font, m.name + " (v" + m.version + ")", 14);
                    mn.setStyle(sf::Text::Bold);
                    mn.setFillColor(sf::Color(225, 230, 240));
                    mn.setPosition({292.f, my + 6.f});
                    window.draw(mn);
                    window.draw(makeText(font, m.author + " - " + m.description.substr(0, 60),
                                         11, kMuted, 292.f, my + 26.f));
                    sf::Text st(font, m.enabled ? "ACTIVO" : "Inactivo", 11);
                    st.setStyle(sf::Text::Bold);
                    st.setFillColor(m.enabled ? sf::Color(80, 200, 120) : kMuted);
                    st.setPosition({940.f, my + 16.f});
                    window.draw(st);
                }
            }

            // Boton Volver.
            float backY = 720.f;
            bool backHov = (settingsHover == 999);
            sf::RectangleShape back({180.f, 50.f});
            back.setPosition({550.f, backY});
            back.setFillColor(backHov ? sf::Color(80, 110, 160) : sf::Color(40, 50, 70));
            back.setOutlineColor(backHov ? sf::Color(180, 210, 255) : sf::Color(80, 95, 130));
            back.setOutlineThickness(2.f);
            window.draw(back);
            sf::Text bt(font, "Volver al menu", 16);
            bt.setStyle(sf::Text::Bold);
            bt.setFillColor(sf::Color(245, 250, 255));
            auto bb = bt.getLocalBounds();
            bt.setOrigin({bb.position.x + bb.size.x / 2.f, bb.position.y + bb.size.y / 2.f});
            bt.setPosition({640.f, backY + 25.f});
            window.draw(bt);

            window.display();
            continue;
        }

        // === ModeSelect state ===
        if (appState == AppState::ModeSelect && fontOk) {
            // Background revolution con dim.
            const sf::Texture* bg = AssetManager::instance().getTexture("bg_revolution");
            if (bg) {
                sf::Sprite spr(*bg);
                auto sz = bg->getSize();
                float sc = std::max(1280.f / sz.x, 800.f / sz.y);
                spr.setScale({sc, sc});
                spr.setPosition({(1280.f - sz.x * sc) * 0.5f, (800.f - sz.y * sc) * 0.5f});
                spr.setColor(sf::Color(255, 255, 255, 80));
                window.draw(spr);
            }
            window.draw(makePanel(0, 0, 1280, 800, sf::Color(15, 17, 26, 180)));

            if (showingScenarioPicker) {
                sf::Text title(fTitle, "ELEGI ESCENARIO HISTORICO", 40);
                title.setStyle(sf::Text::Bold);
                title.setFillColor(sf::Color(220, 180, 80));
                auto lb = title.getLocalBounds();
                title.setOrigin({lb.position.x + lb.size.x / 2.f, 0.f});
                title.setPosition({640.f, 60.f});
                window.draw(title);
                window.draw(makeText(font, "11 escenarios historicos disponibles. Click para arrancar.",
                                     13, kMuted, 240, 130));
                for (size_t i = 0; i < historicalScenarios.size(); ++i) {
                    int col = (int)i % 3, row = (int)i / 3;
                    float cx = 220.f + (float)col * 290.f;
                    float cy = 180.f + (float)row * 110.f;
                    bool hov = (scenarioPickerHover == (int)i);
                    sf::Color top = hov ? sf::Color(100, 80, 50) : sf::Color(50, 42, 32);
                    sf::Color bot = hov ? sf::Color(60, 45, 25)  : sf::Color(30, 24, 18);
                    sf::VertexArray g(sf::PrimitiveType::TriangleStrip, 4);
                    g[0] = sf::Vertex{{cx,         cy      }, top, {}};
                    g[1] = sf::Vertex{{cx + 270.f, cy      }, top, {}};
                    g[2] = sf::Vertex{{cx,         cy + 90.f}, bot, {}};
                    g[3] = sf::Vertex{{cx + 270.f, cy + 90.f}, bot, {}};
                    window.draw(g);
                    sf::RectangleShape outline({270.f, 90.f});
                    outline.setPosition({cx, cy});
                    outline.setFillColor(sf::Color::Transparent);
                    outline.setOutlineColor(hov ? sf::Color(240, 200, 90) : sf::Color(140, 110, 50));
                    outline.setOutlineThickness(hov ? 2.f : 1.5f);
                    window.draw(outline);
                    sf::Text n(fTitle, historicalScenarios[i], 18);
                    n.setStyle(sf::Text::Bold);
                    n.setFillColor(hov ? sf::Color(255, 235, 180) : sf::Color(220, 200, 160));
                    auto nlb = n.getLocalBounds();
                    n.setOrigin({nlb.position.x + nlb.size.x / 2.f, 0.f});
                    n.setPosition({cx + 135.f, cy + 32.f});
                    window.draw(n);
                }
                window.draw(makeText(font, "Esc = volver a modos", 12, kMuted, 16, 770));
                window.display();
                continue;
            }

            sf::Text title(fTitle, "ELEGI TU MODO", 56);
            title.setStyle(sf::Text::Bold);
            title.setFillColor(sf::Color(80, 160, 240));
            auto lb = title.getLocalBounds();
            title.setOrigin({lb.position.x + lb.size.x / 2.f, 0.f});
            title.setPosition({640.f, 80.f});
            window.draw(title);

            const struct { GameMode mode; const char* name; const char* desc; } modes[] = {
                { GameMode::Sandbox,    "SANDBOX",    "Jugar libre. Sin objetivos.\nIdeal para experimentar." },
                { GameMode::Missions,   "MISIONES",   "Objetivos especificos por dificultad.\nProgreso medible." },
                { GameMode::Historical, "HISTORICO",  "Escenarios reales con eventos\nforzados. Re-juega la historia." },
            };
            for (int i = 0; i < 3; ++i) {
                float cx = 220.f + (float)i * 290.f;
                float cy = 220.f;
                float cw = 270.f;
                float ch = 350.f;
                bool hov = (modeSelectHover == i);
                sf::Color top = hov ? sf::Color(80, 110, 160) : sf::Color(40, 48, 70);
                sf::Color bot = hov ? sf::Color(30, 60, 110)  : sf::Color(22, 26, 40);
                sf::VertexArray grad(sf::PrimitiveType::TriangleStrip, 4);
                grad[0] = sf::Vertex{{cx,      cy     }, top, {}};
                grad[1] = sf::Vertex{{cx + cw, cy     }, top, {}};
                grad[2] = sf::Vertex{{cx,      cy + ch}, bot, {}};
                grad[3] = sf::Vertex{{cx + cw, cy + ch}, bot, {}};
                window.draw(grad);
                sf::RectangleShape outline({cw, ch});
                outline.setPosition({cx, cy});
                outline.setFillColor(sf::Color::Transparent);
                outline.setOutlineColor(hov ? sf::Color(180, 210, 255) : sf::Color(80, 95, 130));
                outline.setOutlineThickness(hov ? 3.f : 1.5f);
                window.draw(outline);
                sf::Text n(fTitle, modes[i].name, 30);
                n.setStyle(sf::Text::Bold);
                n.setFillColor(hov ? sf::Color(245, 250, 255) : sf::Color(220, 225, 240));
                auto nlb = n.getLocalBounds();
                n.setOrigin({nlb.position.x + nlb.size.x / 2.f, 0.f});
                n.setPosition({cx + cw / 2.f, cy + 30.f});
                window.draw(n);
                std::string desc = modes[i].desc;
                size_t pos = 0, lineY = 0;
                while (pos < desc.size()) {
                    size_t nl = desc.find('\n', pos);
                    std::string line = desc.substr(pos, nl - pos);
                    sf::Text lt(font, line, 14);
                    lt.setFillColor(hov ? sf::Color(220, 230, 245) : sf::Color(180, 184, 200));
                    auto llb = lt.getLocalBounds();
                    lt.setOrigin({llb.position.x + llb.size.x / 2.f, 0.f});
                    lt.setPosition({cx + cw / 2.f, cy + 90.f + (float)lineY * 22.f});
                    window.draw(lt);
                    ++lineY;
                    if (nl == std::string::npos) break;
                    pos = nl + 1;
                }
            }

            // Iron Man toggle.
            float ironY = 600.f;
            sf::RectangleShape iron({420.f, 60.f});
            iron.setPosition({430.f, ironY});
            iron.setFillColor(ironManToggled ? sf::Color(120, 30, 30) : sf::Color(40, 44, 60));
            iron.setOutlineColor(ironManToggled ? sf::Color(220, 90, 90) : sf::Color(80, 95, 130));
            iron.setOutlineThickness(2.f);
            window.draw(iron);
            sf::Text iLabel(font, ironManToggled
                ? "IRON MAN: ACTIVO (sin save/load manual)"
                : "Activar IRON MAN (hardcore)", 16);
            iLabel.setStyle(sf::Text::Bold);
            iLabel.setFillColor(ironManToggled ? sf::Color(255, 230, 230) : sf::Color(200, 205, 220));
            auto ilb = iLabel.getLocalBounds();
            iLabel.setOrigin({ilb.position.x + ilb.size.x / 2.f, ilb.position.y + ilb.size.y / 2.f});
            iLabel.setPosition({640.f, ironY + 30.f});
            window.draw(iLabel);

            window.draw(makeText(font, "Esc = volver al menu", 12, kMuted, 16, 770));
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

            // Separadores verticales sutiles entre secciones.
            auto drawSep = [&](float sx) {
                sf::RectangleShape sep({1.f, 38.f});
                sep.setPosition({sx + shakeX, 11.f});
                sep.setFillColor(sf::Color(255, 255, 255, 35));
                window.draw(sep);
            };
            drawSep(225.f);   // antes de Turno
            drawSep(370.f);   // antes de Pop
            drawSep(540.f);   // antes de GDP
            drawSep(750.f);   // antes de Inflacion

            window.draw(makeText(font, tr("ui.pop_short", "Pop:"), 16, kMuted, 380 + shakeX, 25));
            window.draw(makeText(font, fmtPct(c.politics.popularity), 20, popularityColor(c.politics.popularity), 425 + shakeX, 22));
            drawProgressBar(window, 380 + shakeX, 47, 130, 6, c.politics.popularity, popularityColor(c.politics.popularity));

            window.draw(makeText(font, "GDP: " + fmtMoney(c.economy.gdp), 18, kText, 560 + shakeX, 22));
            std::ostringstream infStr;
            infStr << "Inflacion: " << std::fixed << std::setprecision(1) << (c.economy.inflation * 100) << "%";
            window.draw(makeText(font, infStr.str(), 18, kText, 760 + shakeX, 22));

            // Botones de velocidad (Pausa / 1x / 2x / 4x).
            {
                struct SpeedBtn { PlaySpeed s; const char* label; float x; };
                SpeedBtn btns[4] = {
                    {PlaySpeed::Paused, "||",  890.f},
                    {PlaySpeed::X1,     "1x",  920.f},
                    {PlaySpeed::X2,     "2x",  950.f},
                    {PlaySpeed::X4,     "4x",  980.f},
                };
                for (const auto& b : btns) {
                    bool active = (playSpeed == b.s);
                    sf::RectangleShape r({26.f, 30.f});
                    r.setPosition({b.x + shakeX, 15.f});
                    r.setFillColor(active ? sf::Color(80, 160, 240) : sf::Color(40, 50, 70));
                    r.setOutlineColor(active ? sf::Color(180, 210, 255) : sf::Color(80, 95, 130));
                    r.setOutlineThickness(active ? 2.f : 1.f);
                    window.draw(r);
                    sf::Text t(font, b.label, 13);
                    t.setStyle(active ? sf::Text::Bold : sf::Text::Regular);
                    t.setFillColor(active ? sf::Color(255, 255, 255) : sf::Color(200, 210, 230));
                    auto lb = t.getLocalBounds();
                    t.setOrigin({lb.position.x + lb.size.x / 2.f, lb.position.y + lb.size.y / 2.f});
                    t.setPosition({b.x + 13.f + shakeX, 30.f});
                    window.draw(t);
                }
            }
            // Boton visible de Next turn (solo visual, la tecla N es la real)
            sf::RectangleShape nextBtn({90.f, 36.f});
            nextBtn.setPosition({1015.f + shakeX, 12.f});
            nextBtn.setFillColor(sf::Color(50, 100, 160));
            nextBtn.setOutlineColor(kAccent);
            nextBtn.setOutlineThickness(2.f);
            window.draw(nextBtn);
            window.draw(makeText(font, "[N] SIG.", 14, sf::Color(255,255,255), 1033 + shakeX, 21));

            window.draw(makeText(font, "1-6 tabs  D=dec  G=over  S=save  P=load  M=mute  L=lang", 11, kMuted, 1130 + shakeX, 22));
            // Indicador de modo en TopBar.
            {
                std::string modeName = GameModeRegistry::name(modeConfig.mode);
                if (IronManMode::isActive()) modeName += " | IRON MAN";
                sf::Text mt(font, modeName, 11);
                mt.setStyle(sf::Text::Bold);
                mt.setFillColor(IronManMode::isActive() ? sf::Color(220, 90, 90) : sf::Color(180, 200, 240));
                mt.setPosition({1130 + shakeX, 38.f});
                window.draw(mt);
            }
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
            const char* actions[] = {"Dashboard", "Mapa", "Accion", "Decisiones", "Logros", "Corte", "Asesores"};
            const Tab tabFor[] = {Tab::Dashboard, Tab::Map, Tab::Action, Tab::Decisions, Tab::Achievements, Tab::Court, Tab::Advisors};
            for (int i = 0; i < 7; ++i) {
                bool active = (currentTab == tabFor[i]);
                float ty = 314.f + i * 32;
                if (active) {
                    // Fondo highlight + barra accent a la izquierda.
                    sf::RectangleShape bg({184.f, 28.f});
                    bg.setPosition({8.f, ty});
                    bg.setFillColor(sf::Color(40, 60, 100, 180));
                    window.draw(bg);
                    sf::RectangleShape bar({3.f, 28.f});
                    bar.setPosition({8.f, ty});
                    bar.setFillColor(kAccent);
                    window.draw(bar);
                }
                sf::Text it(font, actions[i], 16);
                it.setStyle(active ? sf::Text::Bold : sf::Text::Regular);
                it.setFillColor(active ? kAccent : kText);
                it.setPosition({16.f, ty + 4.f});
                window.draw(it);
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
                    sf::Text hdr(fTitle, "DECISIONES  [4]", 18);
                    hdr.setFillColor(kAccent); hdr.setStyle(sf::Text::Bold);
                    hdr.setPosition({220.f, 76.f}); window.draw(hdr);

                    // Si modo Missions, mostrar objetivos arriba.
                    if (modeConfig.mode == GameMode::Missions) {
                        auto objs = GameModeRegistry::objectivesFor(GameMode::Missions, "");
                        sf::Text ohdr(font, "OBJETIVOS DE MISION", 13);
                        ohdr.setFillColor(sf::Color(220, 180, 80));
                        ohdr.setStyle(sf::Text::Bold);
                        ohdr.setPosition({220.f, 108.f});
                        window.draw(ohdr);
                        for (size_t i = 0; i < objs.size() && i < 4; ++i) {
                            float oy = 130.f + (float)i * 22.f;
                            // Checkbox.
                            sf::RectangleShape box({14.f, 14.f});
                            box.setPosition({226.f, oy + 1.f});
                            box.setFillColor(objs[i].achieved ? sf::Color(80, 200, 120) : sf::Color(40, 50, 70));
                            box.setOutlineColor(sf::Color(160, 170, 200));
                            box.setOutlineThickness(1.f);
                            window.draw(box);
                            if (objs[i].achieved) {
                                sf::Text ck(font, "v", 12);
                                ck.setStyle(sf::Text::Bold);
                                ck.setFillColor(sf::Color(20, 30, 20));
                                ck.setPosition({229.f, oy});
                                window.draw(ck);
                            }
                            window.draw(makeText(font, objs[i].description_es, 12,
                                                 objs[i].achieved ? sf::Color(180, 230, 200) : kText,
                                                 246.f, oy));
                        }
                    }

                    float pendY = (modeConfig.mode == GameMode::Missions) ? 240.f : 102.f;
                    window.draw(makeText(font,
                        "Decisiones acumuladas: " + std::to_string(decisionQueue.size()),
                        13, kMuted, 220, pendY));
                    if (decisionQueue.empty()) {
                        window.draw(makeText(font,
                            "No hay decisiones pendientes. Avanza turnos para que aparezcan.",
                            14, kMuted, 220, 140));
                    } else {
                        float baseListY = (modeConfig.mode == GameMode::Missions) ? 280.f : 140.f;
                        for (size_t i = 0; i < decisionQueue.size() && i < 5; ++i) {
                            const auto& d = decisionQueue[i];
                            float dy = baseListY + (float)i * 75.f;
                            sf::RectangleShape card({780.f, 70.f});
                            card.setPosition({220.f, dy});
                            card.setFillColor(sf::Color(38, 42, 56));
                            card.setOutlineColor(sf::Color(80, 90, 120));
                            card.setOutlineThickness(1.f);
                            window.draw(card);
                            sf::Text title(font, d.id, 16);
                            title.setStyle(sf::Text::Bold);
                            title.setFillColor(kAccent);
                            title.setPosition({232.f, dy + 8.f});
                            window.draw(title);
                            window.draw(makeText(font, d.prompt, 12, kText, 232.f, dy + 30.f));
                            std::string opts;
                            for (size_t j = 0; j < d.options.size(); ++j) {
                                if (j > 0) opts += "  /  ";
                                opts += d.options[j];
                            }
                            window.draw(makeText(font, "Opciones: " + opts, 11, kMuted, 232.f, dy + 50.f));
                        }
                        window.draw(makeText(font, "Presiona [D] para resolver la primera.",
                                             13, kWarn, 220, 632));
                    }
                    break;
                }
                case Tab::Achievements: {
                    sf::Text hdr(fTitle, "LOGROS  [5]", 18);
                    hdr.setFillColor(kAccent); hdr.setStyle(sf::Text::Bold);
                    hdr.setPosition({220.f, 76.f}); window.draw(hdr);
                    achievements.update(achievementTracker);
                    achievements.draw(window, font, 218.f, 102.f, 794.f, 580.f);
                    break;
                }
                case Tab::Advisors: {
                    sf::Text hdr(fTitle, "ASESORES  [7]", 18);
                    hdr.setFillColor(kAccent); hdr.setStyle(sf::Text::Bold);
                    hdr.setPosition({220.f, 76.f}); window.draw(hdr);

                    std::string llmName = llmProvider ? llmProvider->name() : "(none)";
                    bool llmReal = llmProvider && llmProvider->isAvailable() && llmName != "MockProvider";
                    window.draw(makeText(font,
                        "Provider: " + llmName + (llmReal ? " (LIVE)" : " (offline mock)"),
                        12, llmReal ? sf::Color(80, 200, 120) : kMuted, 220, 102));

                    // Lista de 5 asesores con botones "Consultar".
                    for (size_t i = 0; i < advisorList.size() && i < 5; ++i) {
                        const auto& adv = advisorList[i];
                        float ay = 130.f + (float)i * 96.f;
                        bool hov = (advisorHover == (int)i);
                        sf::Color top = hov ? sf::Color(60, 80, 120) : sf::Color(38, 42, 56);
                        sf::Color bot = hov ? sf::Color(30, 50,  90) : sf::Color(24, 28, 40);
                        sf::VertexArray g(sf::PrimitiveType::TriangleStrip, 4);
                        g[0] = sf::Vertex{{220.f,        ay      }, top, {}};
                        g[1] = sf::Vertex{{220.f + 780.f, ay      }, top, {}};
                        g[2] = sf::Vertex{{220.f,        ay + 84.f}, bot, {}};
                        g[3] = sf::Vertex{{220.f + 780.f, ay + 84.f}, bot, {}};
                        window.draw(g);
                        sf::RectangleShape card({780.f, 84.f});
                        card.setPosition({220.f, ay});
                        card.setFillColor(sf::Color::Transparent);
                        card.setOutlineColor(hov ? sf::Color(120, 160, 220) : sf::Color(70, 80, 110));
                        card.setOutlineThickness(hov ? 2.f : 1.f);
                        window.draw(card);
                        // Avatar circular (retrato rotativo segun seed + i).
                        const sf::Texture* tex = AssetManager::instance().pickPortrait(gameSeed, (int)i + 4);
                        if (tex) {
                            float ar = 30.f;
                            sf::CircleShape disk(ar);
                            disk.setOrigin({ar, ar});
                            disk.setPosition({252.f, ay + 42.f});
                            disk.setTexture(tex);
                            auto sz = tex->getSize();
                            int side = (int)std::min(sz.x, sz.y);
                            disk.setTextureRect(sf::IntRect({(int)((sz.x - side) / 2), 0}, {side, side}));
                            disk.setOutlineColor(sf::Color(190, 160, 80));
                            disk.setOutlineThickness(1.5f);
                            window.draw(disk);
                        }
                        sf::Text aname(font, adv->name_es(), 16);
                        aname.setStyle(sf::Text::Bold);
                        aname.setFillColor(sf::Color(225, 230, 245));
                        aname.setPosition({296.f, ay + 12.f});
                        window.draw(aname);
                        sf::Text aid(font, adv->id(), 10);
                        aid.setFillColor(kMuted);
                        aid.setPosition({296.f, ay + 32.f});
                        window.draw(aid);
                        // Boton consultar.
                        sf::RectangleShape btn({140.f, 30.f});
                        btn.setPosition({858.f, ay + 26.f});
                        btn.setFillColor(hov ? sf::Color(80, 130, 200) : sf::Color(50, 80, 130));
                        btn.setOutlineColor(sf::Color(120, 160, 220));
                        btn.setOutlineThickness(1.f);
                        window.draw(btn);
                        sf::Text bt(font, "Consultar", 14);
                        bt.setStyle(sf::Text::Bold);
                        bt.setFillColor(sf::Color(245, 250, 255));
                        auto bb = bt.getLocalBounds();
                        bt.setOrigin({bb.position.x + bb.size.x / 2.f, bb.position.y + bb.size.y / 2.f});
                        bt.setPosition({928.f, ay + 41.f});
                        window.draw(bt);
                        // Si esta es el ultimo consultado, mostrar respuesta inline.
                        if (advisorLastWho == adv->id() && !advisorLastResponse.empty()) {
                            // Recortar a 90 chars y multi-line.
                            std::string resp = advisorLastResponse;
                            sf::Text r(font, resp.substr(0, std::min<size_t>(resp.size(), 160)), 12);
                            r.setFillColor(sf::Color(200, 220, 240));
                            r.setPosition({296.f, ay + 52.f});
                            window.draw(r);
                        }
                    }
                    if (!advisorLastResponse.empty()) {
                        window.draw(makeText(font,
                            "Ultimo consejo de: " + advisorLastWho,
                            11, kMuted, 220, 660));
                    } else {
                        window.draw(makeText(font,
                            "Click en \"Consultar\" para pedir consejo en base al estado actual.",
                            12, kMuted, 220, 660));
                    }
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
                    scene.draw(window, font, 218.f, 102.f, 794.f, 230.f, bridge.country(), gameSeed);
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
                // Retrato rotativo segun seed de partida (rol 0 = lider).
                const sf::Texture* portraitTex = AssetManager::instance().pickPortrait(gameSeed, 0);
                const char* leaderName = AssetManager::instance().pickPortraitName(gameSeed, 0);
                if (portraitTex) {
                    LeaderPortrait::drawTextured(window, font, portraitTex,
                                                 leaderName, "Presidente",
                                                 1155.f, 158.f, 42.f, regimeAccent, legit);
                } else {
                    LeaderPortrait::drawDetailed(window, font, "Presidente", "Mandato actual",
                                                 1155.f, 158.f, 42.f, expr, regimeAccent, legit);
                }
                (void)expr;
            }
            // Asesores (3 compactos).
            window.draw(makeText(font, "ASESORES", 14, kMuted, 1046, 248));
            const struct { const char* role; } advisorRoles[] = {
                {"Hacienda"},
                {"Defensa"},
                {"Gabinete"},
            };
            const float advR = 20.f;
            const float advSpacing = 76.f;
            const float advBaseX = 1064.f;
            for (int i = 0; i < 3; ++i) {
                float ax = advBaseX + (float)(i % 3) * advSpacing;
                float ay = 296.f;
                const sf::Texture* at = AssetManager::instance().pickPortrait(gameSeed, i + 1);
                const char* advName = AssetManager::instance().pickPortraitName(gameSeed, i + 1);
                if (at) {
                    sf::CircleShape disk(advR);
                    disk.setOrigin({advR, advR});
                    disk.setPosition({ax, ay});
                    disk.setTexture(at);
                    auto sz = at->getSize();
                    int side = (int)std::min(sz.x, sz.y);
                    sf::IntRect rect({(int)((sz.x - side) / 2), 0}, {side, side});
                    disk.setTextureRect(rect);
                    disk.setOutlineColor(sf::Color(190, 160, 80));
                    disk.setOutlineThickness(1.5f);
                    window.draw(disk);
                } else {
                    LeaderPortrait::drawCompact(window, font, advName, ax, ay, advR);
                }
                sf::Text nm(font, advName, 10);
                nm.setFillColor(kText);
                auto lb = nm.getLocalBounds();
                nm.setOrigin({lb.position.x + lb.size.x / 2.f, 0.f});
                nm.setPosition({ax, ay + 24.f});
                window.draw(nm);
                sf::Text rl(font, advisorRoles[i].role, 9);
                rl.setFillColor(kMuted);
                auto lb2 = rl.getLocalBounds();
                rl.setOrigin({lb2.position.x + lb2.size.x / 2.f, 0.f});
                rl.setPosition({ax, ay + 38.f});
                window.draw(rl);
            }
            // EVENTOS reales (ultimos 5).
            window.draw(makeText(font, "EVENTOS", 14, kMuted, 1046, 358));
            if (eventLog.empty()) {
                window.draw(makeText(font, "Sin novedades", 11, kMuted, 1046, 378));
            } else {
                for (size_t i = 0; i < eventLog.size() && i < 5; ++i) {
                    const auto& ev = eventLog[i];
                    float ey = 378.f + i * 22.f;
                    // Dot de color al inicio.
                    sf::CircleShape dot(3.f);
                    dot.setOrigin({3.f, 3.f});
                    dot.setPosition({1052.f, ey + 6.f});
                    dot.setFillColor(ev.color);
                    window.draw(dot);
                    // Turno y label.
                    std::ostringstream s;
                    s << "T" << ev.turn << "  " << ev.label;
                    window.draw(makeText(font, s.str(), 11, kText, 1062, ey));
                }
            }
            // DECISIONES PENDIENTES reales.
            window.draw(makeText(font, "DECISIONES PEND.", 14, kMuted, 1046, 504));
            if (decisionQueue.empty()) {
                window.draw(makeText(font, "Sin decisiones", 11, kMuted, 1046, 524));
            } else {
                window.draw(makeText(font,
                    std::to_string(decisionQueue.size()) + " en cola",
                    11, kWarn, 1046, 524));
                for (size_t i = 0; i < decisionQueue.size() && i < 4; ++i) {
                    float dy = 542.f + i * 22.f;
                    sf::CircleShape dot(3.f);
                    dot.setOrigin({3.f, 3.f});
                    dot.setPosition({1052.f, dy + 6.f});
                    dot.setFillColor(kAccent);
                    window.draw(dot);
                    window.draw(makeText(font, decisionQueue[i].id, 11, kText, 1062, dy));
                }
                // Hint para abrir.
                window.draw(makeText(font, "[D] siguiente", 10, kMuted, 1046, 642));
            }
        }

        // === BottomBar: Mandate Timeline ===
        {
            sf::Color top = currentPalette.topbar;
            sf::Color bot(
                (uint8_t)std::max(0, top.r - 12),
                (uint8_t)std::max(0, top.g - 12),
                (uint8_t)std::max(0, top.b - 8));
            sf::VertexArray g(sf::PrimitiveType::TriangleStrip, 4);
            g[0] = sf::Vertex{{0.f,    700.f}, top, {}};
            g[1] = sf::Vertex{{1280.f, 700.f}, top, {}};
            g[2] = sf::Vertex{{0.f,    800.f}, bot, {}};
            g[3] = sf::Vertex{{1280.f, 800.f}, bot, {}};
            window.draw(g);
            // Separador superior dorado sutil.
            sf::RectangleShape topLine({1280.f, 1.f});
            topLine.setPosition({0.f, 700.f});
            topLine.setFillColor(sf::Color(180, 150, 80, 80));
            window.draw(topLine);
        }
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
