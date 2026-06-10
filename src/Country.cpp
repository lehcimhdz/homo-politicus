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
    
    // Initialize 3 neighbor countries
    NeighborCountry north;
    north.name = "Northland";
    north.gdp = 400000000.0;
    north.military_strength = 0.6;
    north.diplomatic_relations = 0.4;
    north.trade_volume = 80000000.0;
    north.ideology = 0.6;
    north.has_territorial_claim = false;
    neighbors.push_back(north);

    NeighborCountry east;
    east.name = "Easteria";
    east.gdp = 200000000.0;
    east.military_strength = 0.3;
    east.diplomatic_relations = 0.1;
    east.trade_volume = 30000000.0;
    east.ideology = 0.3;
    east.has_territorial_claim = true;
    neighbors.push_back(east);

    NeighborCountry south;
    south.name = "Southaven";
    south.gdp = 150000000.0;
    south.military_strength = 0.2;
    south.diplomatic_relations = 0.6;
    south.trade_volume = 40000000.0;
    south.ideology = 0.5;
    south.in_crisis = false;
    neighbors.push_back(south);

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
              << " | Rate: " << economy.royalty_rate * 100 << "%"
              << " | Royalties: $" << economy.state_royalties / 1000000.0 << "M"
              << " | Price: " << economy.commodity_prices << "x"
              << " | Depletion: " << economy.resource_depletion * 100 << "%"
              << " | Conflict: " << economy.community_conflicts * 100 << "%" << std::endl;
    // SWF mandate label
    std::string mandate_label;
    switch (economy.swf_mandate) {
        case 0:  mandate_label = "Conservative"; break;
        case 2:  mandate_label = "Growth";       break;
        default: mandate_label = "Balanced";     break;
    }
    std::cout << "SWF: $" << economy.sovereign_wealth_fund / 1000000.0 << "M"
              << " (saving " << economy.swf_deposit_rate * 100 << "% of royalties)"
              << " | Mandate: " << mandate_label
              << " | Transparent: " << (economy.swf_transparent ? "Yes" : "No")
              << " | Rule: " << (economy.swf_rule_active ? "CONSTITUTIONAL" : "Discretionary")
              << " | Legacy Damage: " << economy.mining_legacy_damage * 100 << "%" << std::endl;
    // Ideology compass
    std::string econ_label = politics.economic_ideology < 0.3 ? "LEFT" :
                             politics.economic_ideology > 0.7 ? "RIGHT" : "CENTER";
    std::string auth_label = politics.auth_dem_axis < 0.3 ? "DEMOCRATIC" :
                             politics.auth_dem_axis > 0.7 ? "AUTHORITARIAN" : "HYBRID";
    std::cout << "Ideology: " << econ_label << " (" << (int)(politics.economic_ideology * 100) << ")"
              << " | " << auth_label << " (" << (int)(politics.auth_dem_axis * 100) << ")"
              << " | Pop wants: " << (int)(politics.population_economic_pref * 100)
              << " | Opposition: " << (int)(politics.opposition_ideology * 100)
              << " | Alignment: " << (politics.ideology_alignment_bonus >= 0 ? "+" : "")
              << (int)(politics.ideology_alignment_bonus * 100) << "%" << std::endl;
    std::cout << "----------------------" << std::endl;
}
