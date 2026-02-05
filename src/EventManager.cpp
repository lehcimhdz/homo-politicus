#include "EventManager.hpp"
#include <iostream>
#include <cstdlib>

EventManager::EventManager() {
    // Constructor
}

void EventManager::triggerRandomEvent(Country& country) {
    int roll = std::rand() % 100; // 0 to 99

    // Dynamic Probability based on Country stats
    // Example: 0.02 * 100 = 2. So if roll < 2 (0, 1), Pandemic happens.
    int pandemic_threshold = (int)(country.welfare.pandemic_prob * 100);

    if (roll < pandemic_threshold) { // ~2% Chance -> PANDEMIC
        std::cout << "\n[!] EVENT: A new Panda-emic Virus spreads!" << std::endl;
        std::cout << "    Effect: Population -5%, GDP -2%" << std::endl;
        country.welfare.population *= 0.95;
        country.economy.gdp *= 0.98;
    }
    // Shift other events to not overlap
    else if (roll < 20) { // Keep Tech at ~10-20 range for now (Arcade)
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
    else {
        // Check for specific specialized events (independent of the main roll)
        
        // FOOD CONTAMINATION CHECK
        int food_roll = std::rand() % 100;
        int contamination_threshold = (int)(country.welfare.food_contamination_prob * 100); // e.g. 5
        
        if (food_roll < contamination_threshold) {
            std::string foodType;
            int type = std::rand() % 3;
            if (type == 0) foodType = "Meat (Salmonella)";
            else if (type == 1) foodType = "Milk (Listeria)";
            else foodType = "Vegetables (Points for E. Coli)";

            std::cout << "\n[!] HEALTH ALERT: Widespread " << foodType << " contamination detected!" << std::endl;
            std::cout << "    Effect: Inflation +1%, Popularity -3%" << std::endl;
            
            country.economy.inflation += 0.01;      // Food prices rise due to scarcity
            country.politics.popularity -= 0.03;    // Public anger
            if (country.politics.popularity < 0.0) country.politics.popularity = 0.0;
        }

        // NUCLEAR SAFETY CHECK (0.5% Chance)
        if (std::rand() % 200 == 0) {
            std::cout << "\n[!!!] CATASTROPHE: NUCLEAR REACTOR MELTDOWN!" << std::endl;
            std::cout << "      Region evacuated. Food supply contaminated." << std::endl;
            std::cout << "      Effect: GDP -20%, Radiation Permanent." << std::endl;
            
            country.welfare.food_radiation_prob = 1.0; // Permanent contamination
            country.economy.gdp *= 0.80; // Immediate economic crash
            country.politics.popularity -= 0.20; // Massive outrage
        }

        // MASS CASUALTY INCIDENT CHECK (1% Chance)
        int mci_threshold = (int)(country.welfare.mass_casualty_prob * 100);
        if (std::rand() % 100 < mci_threshold) {
             std::cout << "\n[!] EMERGENCY: Mass Casualty Incident (Building Collapse/Fire)!" << std::endl;
             
             int casualties = 500 + (std::rand() % 1500); // 500 to 2000 injured
             int capacity = country.welfare.hospitals * 15; // Each hospital handles 15 critical patients
             
             std::cout << "    Casualties: " << casualties << " | Hospital Capacity: " << capacity << std::endl;

             if (capacity >= casualties) {
                 std::cout << "    RESULT: System handled the crisis. Minimal deaths." << std::endl;
                 country.politics.popularity += 0.01; // Competence bonus
             } else {
                 int excess_deaths = casualties - capacity;
                 std::cout << "    RESULT: SYSTEM COLLAPSE. " << excess_deaths << " preventable deaths." << std::endl;
                 country.welfare.population -= excess_deaths;
                 country.politics.popularity -= 0.05; // Incompetence penalty
             }
        }
    }
}
