#ifndef COUNTRY_HPP
#define COUNTRY_HPP

#include <string>

// 1. Welfare and Society System
struct WelfareSocietySystem {
    // Health
    double pandemic_prob = 0.02; // Realistic: ~2% annual chance (Every 50-100 years)
    double epidemic_prob = 0.1;
    double food_contamination_prob = 0.05;
    double obesity_rate = 0.2;
    double health_coverage = 0.7;
    double food_radiation_prob = 0.0;
    int hospitals = 100;
    // products? (maybe medical supplies inventory)
    double mass_casualty_prob = 0.01;
    double severe_burn_prob = 0.01;
    double major_accident_prob = 0.05; // Transport Infra (Trains/Bridges)
    double aviation_accident_prob = 0.001; // Airplanes (0.1% - Rare but Tragic)
    
    // Social Welfare (Mental)
    double mental_health_index = 0.7; // 0.0 (Despair) to 1.0 (Joy)
    double suicide_rate = 0.00014; // Base: 14 per 100k (Global Average)

    // Education
    double literacy_rate = 0.95;
    double primary_enrollment = 0.98;
    double secondary_enrollment = 0.90;
    double university_enrollment = 0.40;
    double educational_quality = 0.7; // 0.0 to 1.0
    double brain_drain = 0.05;
    double research_spending_gdp = 0.005; // 0.5% of GDP (Start low)

    // Labor and Social Security
    double unemployment_rate = 0.05;
    double union_strength = 0.4;
    double pension_sustainability = 0.6;
    double minimum_wage = 1000.0;
    double labor_informality = 0.15;
    double general_strike_prob = 0.02;

    // Demographics
    double birth_rate = 0.015;
    double death_rate = 0.008;
    double aging_index = 0.3;
    double urban_population_ratio = 0.8; // vs rural
    double population_density = 50.0; // people per sq km
    int population = 10000000;

    // Religion
    double clerical_political_influence = 0.3;
    double interreligious_tension = 0.1;
    double radicalism_prob = 0.05;
    double freedom_of_worship = 0.9;

    // Human Rights
    double torture_index = 0.0;
    double forced_disappearances = 0.0;
    double freedom_of_expression = 0.8;
    double minority_protection = 0.7;
    double un_score = 0.8;
};

// 2. Economic and Financial System
struct EconomicFinancialSystem {
    // Economy
    double gdp = 500000000.0;
    double remittances = 1000000.0;
    double tax_collection = 10000000.0;
    double inflation = 0.03;
    double growth_rate = 0.02;
    double international_reserves = 50000000.0;

    // Central Bank
    double central_bank_autonomy = 0.9;
    double interest_rate = 0.05;
    double monetary_emission = 0.0;
    double exchange_rate_stability = 0.8;

    // Sovereign Debt
    double debt_to_gdp_ratio = 0.4;
    std::string credit_rating = "AA";
    double default_prob = 0.01;
    double debt_interest = 2000000.0;

    // Natural Resources/Extraction
    int mining_concessions = 10;
    double state_royalties = 5000000.0;
    double community_conflicts = 0.1;
    double resource_depletion = 0.05;
    double commodity_prices = 1.0; // Multiplier

    // Foreign Trade
    double trade_balance = 0.0; // Exports - Imports
    double average_tariffs = 0.1;
    int free_trade_agreements = 5;
    double import_dependency = 0.3;
    double international_sanctions_prob = 0.0;

    // Tourism
    int annual_visitors = 500000;
    double average_tourist_spending = 1000.0;
    double tourist_safety = 0.8;
    double heritage_preservation = 0.7;
    double foreign_travel_alert_prob = 0.0;
};

// 3. Political and Institutional System
struct PoliticalInstitutionalSystem {
    // Politics
    double popularity = 0.6;
    double stay_in_power_prob = 0.8;
    double authoritarianism_prob = 0.1;
    double coup_d_etat_prob = 0.01;
    double administrative_corruption = 0.2;

    // Governance
    int marches = 0;
    int blockades = 0;
    int mobilizations = 0;
    double polarization_index = 0.3;

    // Legislative
    double congressional_support = 0.6;
    double law_blockade_prob = 0.2;
    double party_fragmentation = 0.4;
    double lobbying_cost = 50000.0;
    int pending_bills = 5;

    // Governors
    double regional_loyalty = 0.7;
    double federal_budget_disparity = 0.2;
    double separatism_prob = 0.01;
    double provincial_autonomy = 0.5;

    // Judiciary
    double judicial_independence = 0.8;
    double sentencing_speed = 0.5;
    double ruling_against_state_prob = 0.2;
    double trust_in_justice = 0.6;

    // Prosecutor's Office
    int prosecutors = 500;
    int case_files = 2000;
    int offices = 50;
    double impunity = 0.4;

    // Lobbies
    double industrial_power = 0.5;
    double agricultural_power = 0.5;
    double financial_power = 0.7;
    double tech_power = 0.4;
};

// 4. Power, Security, and International Relations System
struct PowerSecuritySystem {
    // International Relations
    double proximity_to_superpower = 0.5;
    double mass_migration_prob = 0.1;
    double nuclear_attack_prob = 0.001;
    double invasion_prob = 0.01;
    double diplomatic_prestige = 0.6;

    // Security
    double serial_killer_prob = 0.01;
    double weapons_in_population = 0.3;
    int non_state_groups = 2;
    double conflict_with_groups = 0.1;
    double homicide_rate = 10.0; // per 100k

    // Intelligence
    double espionage_budget = 500000.0;
    double informant_network = 0.4;
    double document_leak_prob = 0.1;
    double cyber_surveillance = 0.3;
    double attack_detection_prob = 0.6;

    // Defense
    double troop_morale = 0.8;
    double equipment_modernization = 0.6;
    double air_force_strength = 0.5;
    double naval_force_strength = 0.5;
    double military_insubordination_prob = 0.05;
    double military_spending_gdp = 0.02;

    // Communication and Propaganda
    double media_control = 0.3;
    double narrative_reach = 0.5;
    double fake_news_success_prob = 0.2;
    double press_freedom = 0.8;
};

// 5. Infrastructure and Future System
struct InfrastructureFutureSystem {
    // Environment
    double pollution_prob = 0.4;
    double fauna_extinction_prob = 0.1;
    double drought_prob = 0.1;
    double storm_prob = 0.1;
    double tornado_prob = 0.01;
    double co2_emissions = 1000.0;

    // Science and Technology
    double st_investment_gdp = 0.01;
    double private_investment = 0.02;
    int patent_development = 100;
    double innovation_index = 0.4;

    // Infrastructure
    double road_connectivity = 0.7;
    double port_capacity = 0.6;
    int airports = 5;
    double internet_coverage = 0.8;
    double potable_water_access = 0.9;
    double maintenance_level = 0.6;

    // Energy
    double generation_capacity = 1.2; // relative to demand
    double fossil_fuel_dependency = 0.7;
    double renewables_percentage = 0.2;
    double oil_gas_reserves = 1000000.0;
    double blackout_prob = 0.05;
    double kwh_price = 0.15;

    // Space Race
    int satellite_capacity = 2;
    double space_budget = 0.0;
    double launch_failure_prob = 0.1;
    double technological_prestige = 0.3;

    // AI
    double state_ai_development = 0.2;
    double employment_automation = 0.1;
    double ai_cyberattack_prob = 0.05;
    double algorithmic_ethics = 0.5;
};

class Country {
public:
    Country();
    ~Country();

    // Systems
    WelfareSocietySystem welfare;
    EconomicFinancialSystem economy;
    PoliticalInstitutionalSystem politics;
    PowerSecuritySystem security;
    InfrastructureFutureSystem infra;

    void printStatus();
};

#endif // COUNTRY_HPP
