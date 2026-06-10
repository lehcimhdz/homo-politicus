#ifndef GAME_HPP
#define GAME_HPP

#include <random>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "Country.hpp"
#include "EventManager.hpp"

enum class EndCondition {
    NONE,
    COUP_SUCCESS,
    IMPEACHMENT,
    REVOLUTION,
    LAWFARE_REMOVAL,
    ELECTION_LOSS,
    EXILE,
    ASSASSINATION,
    NUCLEAR_ANNIHILATION,
    TERM_COMPLETED
};

struct PendingDecision {
    std::string id;
    std::string prompt;
    std::vector<std::string> options;
};

// The Game class acts as the "Central Brain".
// It manages the game loop, updates variables, and handles player input.
class Game {
public:
    Game();
    void run(); // The main loop function

private:
    void processEvents(); // Check for input (placeholder for now)
    void update();        // Update game state (mathematics, simulation)
    void render();        // Draw the game (text output for now)
    void checkGameOver();
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

    std::mt19937 rng; // Mersenne Twister Random Number Generator
};

#endif // GAME_HPP
