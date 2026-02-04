#include "EventManager.hpp"
#include <iostream>
#include <cstdlib>

EventManager::EventManager() {
    // Constructor
}

void EventManager::triggerRandomEvent(Country& country) {
    int roll = std::rand() % 100; // 0 to 99

    if (roll < 10) { // 0-9: 10% Chance -> PANDEMIC
        std::cout << "\n[!] EVENT: A new Panda-emic Virus spreads!" << std::endl;
        std::cout << "    Effect: Population -5%, GDP -2%" << std::endl;
        country.welfare.population *= 0.95;
        country.economy.gdp *= 0.98;
    }
    else if (roll < 20) { // 10-19: 10% Chance -> TECH BREAKTHROUGH
        std::cout << "\n[!] EVENT: Major AI Breakthrough in your country!" << std::endl;
        std::cout << "    Effect: GDP +5%" << std::endl;
        country.economy.gdp *= 1.05;
    }
    else if (roll < 30) { // 20-29: 10% Chance -> CORRUPTION SCANDAL
        std::cout << "\n[!] EVENT: Minister caught laundering money!" << std::endl;
        std::cout << "    Effect: Popularity -10%" << std::endl;
        country.politics.popularity -= 0.10;
        if (country.politics.popularity < 0.0) country.politics.popularity = 0.0;
    }
}
