#ifndef GAME_HPP
#define GAME_HPP

#include <random>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "Country.hpp"
#include "EventManager.hpp"
#include "EndCondition.hpp"
#include "DecisionSystem.hpp"
#include "AchievementTracker.hpp"
#include "EventLoader.hpp"
#include "Tutorial.hpp"

// The Game class acts as the "Central Brain".
// It manages the game loop, updates variables, and handles player input.
class Game {
public:
    Game();
    void run(); // The main loop function

    // ===== Public API for UI integration (Sprint C19.1) =====
    void update();                              // Ejecuta 1 tick de simulación.
    void checkGameOver();                       // Evalua condiciones de fin.
    void incrementTurn() { ++turnCount; }
    void setQuietMode(bool q) { quietMode_ = q; }
    bool quietMode() const { return quietMode_; }

    Country&       playerCountryRef()       { return playerCountry; }
    const Country& playerCountryRef() const { return playerCountry; }
    int            turnCountValue()   const { return turnCount; }
    EndCondition   endConditionValue() const { return endCondition; }
    double         popularitySumValue() const { return popularitySum; }
    double         initialGdpValue()   const { return initialGdp; }
    AchievementTracker&       achievementsRef()       { return achievements; }
    const AchievementTracker& achievementsRef() const { return achievements; }
    EventManager&             eventManagerRef()       { return eventManager; }
    std::vector<PendingDecision>& pendingDecisionsRef() { return pendingDecisions; }

private:
    void processEvents(); // Check for input (placeholder for now)
    void render();        // Draw the game (text output for now)
    void renderEndScreen();
    void resolveDecision(const std::string& choice);
    void saveGame(const std::string& path);
    void loadGame(const std::string& path);
    void registerCommands();
    std::unordered_map<std::string, std::function<void()>> commandHandlers;

    bool isRunning;
    bool nextTurn; // Moved from global
    int turnCount; // Track years in power
    Country playerCountry; // The country instance
    EventManager eventManager; // Handles random events

    EndCondition endCondition = EndCondition::NONE;
    std::vector<PendingDecision> pendingDecisions;
    double popularitySum = 0.0;     // For end-screen average
    double initialGdp = 0.0;        // Para comparativas de logros
    AchievementTracker achievements;
    std::vector<ScriptedEvent> scriptedEvents;
    Tutorial tutorial;
    bool quietMode_ = false;        // Suprime stdout en modo UI.

    std::mt19937 rng; // Mersenne Twister Random Number Generator
};

#endif // GAME_HPP
