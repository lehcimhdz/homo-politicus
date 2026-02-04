#include "Game.hpp"
#include <iostream>
#include <thread> // For sleep
#include <chrono> // For time duration
#include <cstdlib> // For rand
#include <ctime>   // For time

Game::Game() : isRunning(true) {
    std::cout << "Initializing Game..." << std::endl;
    std::srand(std::time(nullptr)); // Seed the random number generator
}

// Helper state to ensure we only update when "next" is called
bool nextTurn = false;

void Game::run() {
    while (isRunning) {
        render();      // Show status first so player knows what to do
        processEvents(); // Wait for command
        
        if (nextTurn) {
            update();      // Only simulate if player said "next"
            nextTurn = false; // Reset for next loop
        }
    }
}

void Game::processEvents() {
    std::cout << "\nCommand (next/exit/tax+/tax-): ";
    std::string command;
    std::cin >> command;

    if (command == "exit") {
        isRunning = false;
    } 
    else if (command == "next") {
        nextTurn = true;
    }
    else if (command == "tax+") {
        playerCountry.economy.tax_collection *= 1.10; // +10% revenue
        playerCountry.politics.popularity -= 0.05;    // -5% popularity
        playerCountry.economy.inflation += 0.01;      // +1% inflation
        std::cout << ">> Taxes RAISED! Revenue up, Popularity down." << std::endl;
    }
    else if (command == "tax-") {
        playerCountry.economy.tax_collection *= 0.90; // -10% revenue
        playerCountry.politics.popularity += 0.03;    // +3% popularity
        std::cout << ">> Taxes LOWERED! Revenue down, Popularity up." << std::endl;
    }
    else {
        std::cout << ">> Unknown command." << std::endl;
    }
}

void Game::update() {
    std::cout << "--- Simulating One Year ---" << std::endl;

    // 1. Demographics (Simple Annual Calculation)
    double births = playerCountry.welfare.population * playerCountry.welfare.birth_rate;
    double deaths = playerCountry.welfare.population * playerCountry.welfare.death_rate;
    playerCountry.welfare.population += (int)(births - deaths);

    // 2. Economy
    // GDP grows by (growth_rate - inflation) roughly
    double real_growth = playerCountry.economy.growth_rate; 
    playerCountry.economy.gdp += playerCountry.economy.gdp * real_growth;
    
    // Inflation fluctuates slightly (randomness simulation)
    // For now, we just keep it steady or increase it slightly
    // playerCountry.economy.inflation += 0.001; 

    // 3. Politics (Populatiry calculation)
    // People hate inflation and unemployment.
    if (playerCountry.economy.inflation > 0.05) {
        playerCountry.politics.popularity -= 0.02; // -2% popularity if inflation High
    }
    if (playerCountry.welfare.unemployment_rate > 0.10) {
       playerCountry.politics.popularity -= 0.03; // -3% popularity if unemployment High
    }
    
    // Clamp popularity between 0 and 1
    if (playerCountry.politics.popularity > 1.0) playerCountry.politics.popularity = 1.0;
    // Clamp popularity between 0 and 1
    if (playerCountry.politics.popularity > 1.0) playerCountry.politics.popularity = 1.0;
    if (playerCountry.politics.popularity < 0.0) playerCountry.politics.popularity = 0.0;

    eventManager.triggerRandomEvent(playerCountry);
}

void Game::render() {
    // Clear screen (hacky way for terminal)
    // std::cout << "\033[2J\033[1;1H"; 

    std::cout << "\nGame Tick Update:" << std::endl;
    playerCountry.printStatus();
}
