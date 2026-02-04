#include "Game.hpp"
#include <iostream>
#include <thread> // For sleep
#include <chrono> // For time duration

Game::Game() : isRunning(true) {
    std::cout << "Initializing Game..." << std::endl;
}

void Game::run() {
    while (isRunning) {
        processEvents();
        update();
        render();

        // Slow down the loop so we can read the text
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void Game::processEvents() {
    // In a GUI game, we would check for mouse/keyboard here.
    // For now, we'll just simulate the game running forever until stopped.
}

void Game::update() {
    // Simulation Logic goes here.
    // Example: Population grows by birth rate - death rate
    
    // Simple growth logic just to show it changing
    playerCountry.welfare.population += 100; 
    playerCountry.economy.gdp += 5000;
}

void Game::render() {
    // Clear screen (hacky way for terminal)
    // std::cout << "\033[2J\033[1;1H"; 

    std::cout << "\nGame Tick Update:" << std::endl;
    playerCountry.printStatus();
}
