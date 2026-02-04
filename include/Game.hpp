#ifndef GAME_HPP
#define GAME_HPP

#include "Country.hpp"
#include "EventManager.hpp"

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

    bool isRunning;
    int turnCount; // Track years in power
    Country playerCountry; // The country instance
    EventManager eventManager; // Handles random events
};

#endif // GAME_HPP
