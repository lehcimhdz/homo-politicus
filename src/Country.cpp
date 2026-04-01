#include "Country.hpp"
#include <iostream>

// Helper to convert CreditRating to std::string
std::string CreditRatingToString(CreditRating rating) {
    switch (rating) {
        case CreditRating::AAA: return "AAA";
        case CreditRating::AA:  return "AA";
        case CreditRating::A:   return "A";
        case CreditRating::BBB: return "BBB";
        case CreditRating::BB:  return "BB";
        case CreditRating::B:   return "B";
        case CreditRating::CCC: return "CCC";
        case CreditRating::CC:  return "CC";
        case CreditRating::C:   return "C";
        case CreditRating::SD:  return "SD";
        case CreditRating::D:   return "D";
        default:                return "Unknown";
    }
}

double GetDefaultProbability(CreditRating rating) {
    switch (rating) {
        case CreditRating::AAA: return 0.001; // 0.1%
        case CreditRating::AA:  return 0.01;  // 1%
        case CreditRating::A:   return 0.03;  // 3%
        case CreditRating::BBB: return 0.05;  // 5%
        case CreditRating::BB:  return 0.10;  // 10%
        case CreditRating::B:   return 0.20;  // 20%
        case CreditRating::CCC: return 0.40;  // 40%
        case CreditRating::CC:  return 0.60;  // 60%
        case CreditRating::C:   return 0.80;  // 80%
        case CreditRating::SD:  return 0.95;  // 95%
        case CreditRating::D:   return 1.0;   // 100%
        default:                return 0.01;
    }
}

double GetInterestRate(CreditRating rating) {
    switch (rating) {
        case CreditRating::AAA: return 0.01;  // 1%
        case CreditRating::AA:  return 0.02;  // 2%
        case CreditRating::A:   return 0.035; // 3.5%
        case CreditRating::BBB: return 0.05;  // 5%
        case CreditRating::BB:  return 0.075; // 7.5%
        case CreditRating::B:   return 0.10;  // 10%
        case CreditRating::CCC: return 0.15;  // 15%
        case CreditRating::CC:  return 0.20;  // 20%
        case CreditRating::C:   return 0.25;  // 25%
        case CreditRating::SD:  return 0.35;  // 35%
        case CreditRating::D:   return 0.50;  // 50% (Distressed)
        default:                return 0.05;
    }
}

// Constructor Implementation
// This is where we can set initial values if they aren't set in the struct.
Country::Country() {
    economy.default_prob = GetDefaultProbability(economy.credit_rating);
    
    // Calculate initial debt interest based on total debt and rating
    double total_debt = economy.gdp * economy.debt_to_gdp_ratio;
    economy.debt_interest = total_debt * GetInterestRate(economy.credit_rating);
    
    std::cout << "A new country has been founded!" << std::endl;
}

Country::~Country() {
    // Nothing to clean up yet
}

void Country::printStatus() {
    std::cout << "--- Country Status ---" << std::endl;
    std::cout << "Population: " << welfare.population << std::endl;
    std::cout << "GDP: $" << economy.gdp << std::endl;
    std::cout << "Inflation: " << economy.inflation * 100 << "%" << std::endl;
    std::cout << "Leader Popularity: " << politics.popularity * 100 << "%" << std::endl;
    std::cout << "Credit Rating: " << CreditRatingToString(economy.credit_rating) << std::endl;
    std::cout << "Unemployment: " << welfare.unemployment_rate * 100 << "%" << std::endl;
    std::cout << "CO2 Emissions: " << infra.co2_emissions << std::endl;
    std::cout << "Mining: " << economy.mining_concessions << " concessions"
              << " | Royalties: $" << economy.state_royalties / 1000000.0 << "M"
              << " | Price: " << economy.commodity_prices << "x"
              << " | Depletion: " << economy.resource_depletion * 100 << "%"
              << " | Conflict: " << economy.community_conflicts * 100 << "%" << std::endl;
    std::cout << "----------------------" << std::endl;
}
