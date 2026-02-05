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

        // NUCLEAR SAFETY CHECK
        // Requirement: You only have nuclear plants if you are Industrialized (Power > 0.6)
        if (country.politics.industrial_power > 0.6) {
            // Base Risk: 1 in 200 (0.5%)
            // Mitigation: High Science (Educational Quality) reduces risk.
            // If Quality is 0.5 -> Risk is normal.
            // If Quality is 1.0 -> Risk is halved.
            int safety_roll = 200 + (int)(country.welfare.educational_quality * 200); 
            
            if (std::rand() % safety_roll == 0) {
                std::cout << "\n[!!!] CATASTROPHE: NUCLEAR REACTOR MELTDOWN!" << std::endl;
                std::cout << "      Cause: Technological failure in high-risk industry." << std::endl;
                std::cout << "      Region evacuated. Food supply contaminated." << std::endl;
                std::cout << "      Effect: GDP -20%, Radiation Permanent." << std::endl;
                
                country.welfare.food_radiation_prob = 1.0; // Permanent contamination
                country.economy.gdp *= 0.80; // Immediate economic crash
                country.politics.popularity -= 0.20; // Massive outrage
            }
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

        // INDUSTRIAL HAZARD CHECK (Refineries, Pipelines, Mines)
        // Risk increases with Fossil Fuel Dependency and Industrial Power
        int hazard_risk = (int)((country.infra.fossil_fuel_dependency * 20) + (country.politics.industrial_power * 10)); // Max ~30%
        
        if (std::rand() % 100 < hazard_risk) {
            std::string accidentType;
            int type = std::rand() % 3;
            if (type == 0) accidentType = "Oil Pipeline Explosion";
            else if (type == 1) accidentType = "Refinery Fire";
            else accidentType = "Chemical Plant Leak";

            std::cout << "\n[!] DISASTER: " << accidentType << "!" << std::endl;
            std::cout << "    Effect: Severe Burns, Ecological Damage, GDP Loss." << std::endl;

            // 1. Severe Burns (Overwhelms hospitals specifically)
            int burn_victims = 200 + (std::rand() % 500);
            country.welfare.severe_burn_prob += 0.05; // Spike in risk perception
            
            // 2. Economic Hit (Infrastructure destroyed)
            country.economy.gdp -= 50000000; // $50M loss
            
            // 3. Environment
            country.infra.co2_emissions += 100; // Smoke/Pollution
            
            std::cout << "    " << burn_victims << " workers suffered critical burns." << std::endl;
            // Simple death calculation for now
            country.welfare.population -= (burn_victims / 10); // 10% mortality immediately
        }

        // TRANSPORT INFRASTRUCTURE ACCIDENT (Bridge Collapse, Train Derailment)
        // Risk is high if connectivity (maintenance) is low.
        // If connectivity is 1.0 (100%), risk is low (1%).
        // If connectivity is 0.5 (50%), risk is high (5%).
        int transport_risk = (int)((1.1 - country.infra.road_connectivity) * 10); 
        if (transport_risk < 1) transport_risk = 1; // Min 1%

        if (std::rand() % 100 < transport_risk) {
            std::cout << "\n[!] INFRASTRUCTURE FAILURE: Massive Transport Accident!" << std::endl;
            std::cout << "    Cause: Poor maintenance / Aging structure." << std::endl;
            
            // Effect: Logistics crippled
            std::cout << "    Effect: Road Connectivity -5%, GDP Growth Stalls." << std::endl;
            country.infra.road_connectivity -= 0.05;
            if (country.infra.road_connectivity < 0.0) country.infra.road_connectivity = 0.0;

            // Casualties
            int victims = 50 + (std::rand() % 100);
            country.welfare.population -= victims;
            std::cout << "    Casualties: " << victims << " dead." << std::endl;
        }

        // AVIATION ACCIDENT (Rare but High Impact)
        // Probability: aviation_accident_prob (0.1%)
        // Impact: National Mourning (Popularity), Tourism drop.
        int aviation_risk = (int)(country.welfare.aviation_accident_prob * 1000); // 0.001 * 1000 = 1
        if (std::rand() % 1000 < aviation_risk) {
            std::cout << "\n[!] TRAGEDY: Commercial Airliner Crash!" << std::endl;
            std::cout << "    A national carrier has gone down." << std::endl;
            
            int passengers = 150 + (std::rand() % 200);
            std::cout << "    Casualties: " << passengers << " confirmed dead." << std::endl;
            country.welfare.population -= passengers;

            // Effect: National Depression
            std::cout << "    Effect: National Mourning (Popularity -5%), Tourism Scare." << std::endl;
            country.politics.popularity -= 0.05;
        }
    }
}
