#include "Game.hpp"
#include <iostream>

int main() {
    std::cout << "Welcome to Homo Politicus!" << std::endl;
    
    // Create the game instance
    Game game;
    
    // Start the game loop
    game.run();

    return 0;
}
