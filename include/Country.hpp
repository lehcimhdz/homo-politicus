#ifndef COUNTRY_HPP
#define COUNTRY_HPP

#include <string>

// Credit Rating Enum
enum class CreditRating {
    AAA, AA, A,
    BBB, BB, B,
    CCC, CC, C,
    SD, // Selective Default
    D   // Default
};

std::string CreditRatingToString(CreditRating rating);
double GetDefaultProbability(CreditRating rating);
double GetInterestRate(CreditRating rating);

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
    double retirement_age = 65.0; // Years
    double minimum_wage = 1000.0;
    double labor_informality = 0.15;
    double general_strike_prob = 0.02;
    double poverty_rate = 0.25; // 25% of population below poverty line

    // Demographics
    double birth_rate = 0.015;
    double death_rate = 0.008;
    double aging_index = 0.3;
    double urban_population_ratio = 0.8; // vs rural
    double land_area = 200000.0; // sq km (Fixed)
    double population_density = 50.0; // people per sq km
    double net_migration_rate = 0.0; // +/- per 1000 people
    int population = 10000000;
    int diaspora_population = 500000; // Citizens living abroad

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
    CreditRating credit_rating = CreditRating::AA;
    double default_prob = 0.01;
    double debt_interest = 2000000.0;

    // Natural Resources/Extraction
    int mining_concessions = 10;
    double royalty_rate = 0.15;         // State's share of extraction value (5%–50%)
    double state_royalties = 5000000.0; // Actual royalties collected after corruption leakage
    double community_conflicts = 0.1;
    double resource_depletion = 0.05;
    double commodity_prices = 1.0;      // Global price multiplier (smoothed)
    double commodity_supercycle = 0.0; // Long structural trend (-0.45 to +0.45)
    bool   supercycle_bull = true;     // Current regime: bull (demand-driven) or bear (oversupply)
    int    supercycle_years = 0;       // Years elapsed in current regime
    int    commodity_hedge_turns = 0;  // Turns remaining with locked-in price

    // Sovereign Wealth Fund
    double sovereign_wealth_fund = 0.0;   // Accumulated savings balance
    double swf_deposit_rate = 0.0;        // Fraction of annual royalties auto-saved (0–0.5)
    int    swf_mandate = 1;               // 0=Conservative 1=Balanced 2=Growth
    bool   swf_transparent = false;       // Public oversight and independent board
    bool   swf_rule_active = false;       // Constitutional fiscal rule: minimum 20% deposit during booms

    // Environmental Legacy
    double mining_legacy_damage = 0.0;    // Accumulated contamination from past extraction (0–1)
    double contaminated_area_km2 = 0.0;   // Surface area affected by tailings/leaching (km²)
    double remediation_cost = 0.0;        // Estimated cleanup liability (USD)
    int    superfund_sites = 0;           // Critical contamination zones requiring emergency intervention
    bool   remediation_active = false;    // State-funded cleanup program currently underway

    // Foreign Trade
    double trade_balance = 0.0;            // Goods: exports − imports
    double services_balance = 0.0;         // Tourism, finance, IP royalties net flow
    double current_account_balance = 0.0;  // trade + services + remittances (full external position)
    double trade_openness = 0.6;           // (exports+imports)/GDP ratio — structural openness
    double average_tariffs = 0.1;
    double tariff_manufactured = 0.08;    // Industrial goods tariff rate
    double tariff_agricultural = 0.15;    // Food/agro tariff rate
    double tariff_strategic = 0.25;       // Defense/tech tariff rate
    double non_tariff_barriers = 0.2;     // Regulatory, quota, bureaucratic friction (0–1)
    int    antidumping_cases = 0;         // Active anti-dumping investigations
    int free_trade_agreements = 5;
    double fta_depth_index = 0.5;           // Avg depth: 0=goods-only, 1=full economic integration
    double rules_of_origin_compliance = 0.8; // Share of exports meeting RoO thresholds
    double trade_diversion_risk = 0.1;      // Risk of cheap imports displacing domestic producers
    double import_dependency = 0.3;
    double food_import_dependency = 0.2;      // Food security risk (0=self-sufficient, 1=fully dependent)
    double energy_import_dependency = 0.5;    // Fuel/electricity import reliance
    double medicine_import_dependency = 0.4;  // Pharmaceutical supply chain exposure
    double supply_chain_vulnerability = 0.3;  // Composite shock propagation risk
    double international_sanctions_prob = 0.0;
    bool   sanctions_active = false;      // Currently under international sanctions
    int    sanctions_tier = 0;            // 0=none 1=targeted(individuals) 2=sectoral 3=comprehensive
    double sanctions_gdp_impact = 0.0;    // Annual GDP drag from active sanctions (fraction)

    // Tourism
    int annual_visitors = 500000;
    int visitors_international = 300000;  // Foreign arrivals (subset of total)
    int visitors_domestic = 200000;       // Internal tourism
    double visa_restrictiveness = 0.3;    // 0=visa-free for most, 1=highly restricted entry
    double tourism_seasonality = 0.4;     // Peak/low season spread (0=flat, 1=extreme)
    double average_tourist_spending = 1000.0;
    double spending_luxury = 5000.0;      // Avg spend: high-end tourists (USD)
    double spending_budget = 200.0;       // Avg spend: backpackers/budget (USD)
    double luxury_tourism_share = 0.1;    // Fraction of visitors in luxury tier
    double tourist_safety = 0.8;
    double tourist_crime_rate = 0.02;         // Crimes targeting tourists (per visitor)
    double kidnapping_tourism_risk = 0.001;   // Kidnapping/extortion probability per trip
    double health_risk_tourists = 0.01;       // Disease/medical emergency rate per visitor
    double heritage_preservation = 0.7;
    int    unesco_sites = 2;                 // Active UNESCO World Heritage designations
    double cultural_commodification = 0.2;   // Risk of tourism eroding authentic culture (0–1)
    double heritage_funding_gdp = 0.001;     // Public spending on preservation (% GDP)
    double foreign_travel_alert_prob = 0.0;
    int    travel_warning_level = 0;          // 0=Normal 1=Exercise caution 2=Reconsider 3=Do not travel
    double reputational_tourism_damage = 0.0; // Cumulative tourist drop from past alerts (0–1)
};

// 3. Political and Institutional System
struct PoliticalInstitutionalSystem {
    // Politics
    double popularity = 0.6;
    double popularity_trend = 0.0;           // Monthly delta (positive = rising)
    int    honeymoon_turns_remaining = 4;    // Post-election goodwill period
    double crisis_approval_floor = 0.2;      // Minimum floor during severe crises
    double stay_in_power_prob = 0.8;
    bool   term_limit_active = true;         // Constitutional re-election limit in effect
    int    terms_served = 1;                 // Completed terms in current office
    double incumbent_advantage = 0.15;       // Electoral bonus from office resources
    double electoral_manipulation_capacity = 0.1; // Ability to tilt outcomes (0–1)
    double authoritarianism_prob = 0.1;
    double democratic_backsliding_index = 0.1; // Cumulative erosion of democratic norms (0–1)
    bool   state_of_emergency_active = false;  // Emergency decree powers in effect
    int    emergency_turns_elapsed = 0;        // Turns since emergency declared
    double coup_d_etat_prob = 0.01;
    int    coup_attempts_history = 0;         // Total past coup attempts (increases risk)
    double civilian_military_control = 0.8;   // Civilian oversight of armed forces (0–1)
    double coup_success_prob = 0.3;           // If attempted, probability of success
    double administrative_corruption = 0.2;
    double corruption_perception_index = 60.0; // 0=highly corrupt, 100=very clean (TI scale)
    double petty_corruption = 0.15;            // Bribery in daily public services (0–1)
    double grand_corruption = 0.08;            // High-level embezzlement/state capture (0–1)
    double anticorruption_enforcement = 0.4;   // Effectiveness of anti-corruption agencies (0–1)

    // Governance
    int marches = 0;
    int blockades = 0;
    int mobilizations = 0;
    double protest_intensity = 0.0;            // 0=symbolic, 1=insurrectional
    bool   protest_pro_government = false;     // Direction: pro-govt (true) or opposition-led (false)
    int    protest_duration_turns = 0;         // Consecutive turns of sustained mobilization
    double polarization_index = 0.3;
    double affective_polarization = 0.4;       // Hatred between partisan blocs (0–1)
    double economic_polarization = 0.3;        // Income/class divide (0–1)
    double media_echo_chamber = 0.4;           // Information silo effect (0–1)

    // Legislative
    double congressional_support = 0.6;
    double coalition_cohesion = 0.7;           // Internal unity of governing coalition (0–1)
    int    opposition_seats = 40;              // Opposition seat count (out of 100)
    double veto_player_strength = 0.3;         // Capacity of minorities to block legislation (0–1)
    double law_blockade_prob = 0.2;
    double filibuster_usage = 0.1;             // Rate of procedural obstruction
    double presidential_veto_prob = 0.05;      // Probability executive vetoes own coalition's bill
    double party_fragmentation = 0.4;
    int    effective_parties = 3;              // Effective Number of Parties (Laakso-Taagepera)
    bool   two_party_dominance = false;        // Two-party duopoly in effect
    double coalition_formation_cost = 0.3;     // Resources/concessions needed to build majority (0–1)
    double lobbying_cost = 50000.0;
    double regulatory_capture_index = 0.3;     // Degree industry controls its regulator (0–1)
    double revolving_door_intensity = 0.4;     // Frequency of regulator-industry personnel flow (0–1)
    int pending_bills = 5;
    int    bills_passed_this_turn = 0;         // Legislative throughput
    double legislative_efficiency = 0.5;       // Bills passed / bills introduced ratio
    int    constitutional_reforms_pending = 0; // High-threshold reforms in queue

    // Governors
    double regional_loyalty = 0.7;
    double fiscal_transfer_adequacy = 0.6;     // Perceived fairness of center-region fiscal flows (0–1)
    int    regions_in_conflict = 0;            // Regions with active political confrontation

    double federal_budget_disparity = 0.2;
    double regional_gdp_gini = 0.3;            // Economic inequality across regions
    double lagging_region_share = 0.2;         // Fraction of regions below 60% national avg GDP

    double separatism_prob = 0.01;
    int    active_separatist_movements = 0;    // Organized movements currently active
    double separatist_support_pop = 0.05;      // Share of population with separatist sympathies
    bool   independence_referendum_pending = false; // Formal referendum process underway

    double provincial_autonomy = 0.5;
    double fiscal_autonomy = 0.4;              // Share of sub-national own-revenue vs transfers
    double legislative_autonomy = 0.5;         // Scope of regional lawmaking authority

    // Judiciary
    double judicial_independence = 0.8;
    double court_packing_risk = 0.1;           // Probability of executive court manipulation
    double judicial_budget_adequacy = 0.7;     // Court funding vs caseload needs (0–1)

    double sentencing_speed = 0.5;
    int    case_backlog = 50000;               // Pending unresolved cases
    double pretrial_detention_rate = 0.35;     // Fraction of prisoners awaiting trial
    double average_case_duration_years = 3.0;  // Mean time from charge to verdict

    double ruling_against_state_prob = 0.2;
    bool   constitutional_review_active = true; // Courts can strike down legislation
    double state_compliance_rate = 0.7;        // Rate at which state complies with adverse rulings

    double trust_in_justice = 0.6;
    double elite_trust_in_justice = 0.7;       // Business/wealthy perception
    double poor_trust_in_justice = 0.3;        // Lower-income population perception

    // Prosecutor's Office
    int prosecutors = 500;
    int case_files = 2000;
    int offices = 50;
    int    specialized_units = 3;              // Anti-corruption, organized crime, financial crime
    double witness_protection_capacity = 0.4;  // Ability to protect key witnesses (0–1)

    double impunity = 0.4;
    double impunity_homicide = 0.7;            // Impunity specifically for murder
    double impunity_corruption = 0.85;         // Impunity for political corruption
    double impunity_organized_crime = 0.75;    // Organized crime conviction rate gap

    // Lobbies
    double industrial_power = 0.5;
    double agricultural_power = 0.5;
    double financial_power = 0.7;
    double tech_power = 0.4;
    double extractive_sector_power = 0.6;      // Mining/oil lobby influence
    double media_ownership_concentration = 0.5; // Oligarchic media control (0–1)
    double total_lobby_rent_gdp = 0.02;        // Economic cost of rent-seeking (% GDP)
};

// 4. Power, Security, and International Relations System
struct PowerSecuritySystem {
    // International Relations
    double proximity_to_superpower = 0.5;
    double us_alignment = 0.5;                 // Alignment with US bloc (0–1)
    double china_alignment = 0.3;              // Alignment with China bloc (0–1)
    double non_aligned_index = 0.4;            // Strategic autonomy (0–1)
    double geopolitical_rent = 0.0;            // Aid/investment received for alignment (USD)

    double mass_migration_prob = 0.1;
    double emigration_push_index = 0.3;        // Domestic conditions driving emigration (0–1)
    double refugee_inflow_prob = 0.05;         // Probability of large refugee inflow
    int    refugee_population = 0;             // Current refugees hosted
    double migration_policy_restrictiveness = 0.5; // Border control (0–1)

    double nuclear_attack_prob = 0.001;
    bool   nuclear_umbrella = false;           // Protected by allied nuclear deterrence
    bool   own_nuclear_capability = false;     // Possesses nuclear weapons

    double invasion_prob = 0.01;
    int    hostile_neighbors = 0;              // Neighboring states with territorial claims
    double territorial_dispute_intensity = 0.1; // Severity of active disputes (0–1)
    double alliance_protection = 0.5;          // Military alliance coverage (0–1)

    double diplomatic_prestige = 0.6;
    double soft_power_index = 0.5;             // Cultural/educational/aid influence (0–1)
    int    ambassadors_accredited = 80;        // Countries with diplomatic representation
    double multilateral_leadership = 0.3;      // Active role in intl organizations (0–1)

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
