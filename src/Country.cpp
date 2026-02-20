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

// Constructor Implementation
// This is where we can set initial values if they aren't set in the struct.
Country::Country() {
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
    std::cout << "----------------------" << std::endl;
}
