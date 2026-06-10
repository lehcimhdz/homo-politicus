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

    // Pandemic / Epidemic state
    bool pandemic_active = false;
    int pandemic_duration = 0;       // Turns remaining
    double pandemic_severity = 0.0;  // 0.0–1.0 (mild flu to COVID/plague)
    double pandemic_death_toll = 0.0;
    double pandemic_economic_cost = 0.0;

    // Epidemic state
    bool epidemic_active = false;
    int epidemic_duration = 0;
    double epidemic_severity = 0.0;  // 0.0–0.5 (smaller than pandemic)

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
    bool migration_crisis_active = false;
    int migration_wave_duration = 0;
    double refugee_inflow = 0.0;               // Current wave inflow per turn

    double emigration_push_index = 0.3;        // Domestic conditions driving emigration (0–1)
    double refugee_inflow_prob = 0.05;         // Probability of large refugee inflow
    int    refugee_population = 0;             // Current refugees hosted
    double migration_policy_restrictiveness = 0.5; // Border control (0–1)

    double nuclear_attack_prob = 0.001;
    bool   nuclear_strike = false;
    double nuclear_casualties = 0.0;
    double nuclear_contamination = 0.0;        // Long-term radiation (0–1)
    bool   nuclear_umbrella = false;           // Protected by allied nuclear deterrence
    bool   own_nuclear_capability = false;     // Possesses nuclear weapons

    double invasion_prob = 0.01;
    bool war_active = false;
    int war_duration = 0;
    double war_casualties = 0.0;
    double war_cost = 0.0;
    bool invasion_repelled = false;

    int    hostile_neighbors = 0;              // Neighboring states with territorial claims
    double territorial_dispute_intensity = 0.1; // Severity of active disputes (0–1)
    double alliance_protection = 0.5;          // Military alliance coverage (0–1)

    double diplomatic_prestige = 0.6;
    double soft_power_index = 0.5;             // Cultural/educational/aid influence (0–1)
    int    ambassadors_accredited = 80;        // Countries with diplomatic representation
    double multilateral_leadership = 0.3;      // Active role in intl organizations (0–1)

    // Security
    double serial_killer_prob = 0.01;
    double femicide_rate = 3.0;                // Femicides per 100k women per year
    double mass_shooting_prob = 0.005;         // Probability of mass casualty event
    double hate_crime_index = 0.1;             // Bias-motivated violence incidence (0–1)

    double weapons_in_population = 0.3;
    double illegal_weapons_share = 0.4;        // Fraction of firearms unregistered
    double military_grade_weapons_civilian = 0.05; // Military-type weapons in civilian hands (0–1)

    int non_state_groups = 2;
    int    guerrilla_groups = 0;               // Ideological insurgent organizations
    int    criminal_organizations = 2;         // Drug cartels / organized crime
    int    militia_groups = 0;                 // Paramilitary / community defense
    double nsg_territorial_control = 0.05;     // Share of territory NSGs control (0–1)

    double conflict_with_groups = 0.1;
    int    conflict_casualties_annual = 50;    // Annual deaths from state vs NSG
    double peace_negotiation_prob = 0.1;       // Probability of formal peace talks
    bool   ceasefire_active = false;           // Active ceasefire agreement

    double homicide_rate = 10.0;               // Total homicides per 100k
    double organized_crime_homicide_share = 0.4; // Fraction from criminal orgs
    double domestic_violence_homicide_share = 0.15; // Intimate partner violence
    double state_security_killings = 0.5;      // Extrajudicial killings per 100k

    // Intelligence
    double espionage_budget = 500000.0;
    double humint_capability = 0.4;            // Human intelligence networks (0–1)
    double sigint_capability = 0.3;            // Signals/electronic intelligence (0–1)
    double foreign_intelligence_sharing = 0.5; // Five-Eyes-type alliances (0–1)

    double informant_network = 0.4;
    double informant_penetration_nsg = 0.3;    // Informants inside armed groups (0–1)
    double informant_reliability = 0.6;        // Quality of informant intel (0–1)

    double document_leak_prob = 0.1;
    double whistleblower_protection = 0.4;     // Legal protection for whistleblowers (0–1)
    double leak_foreign_intelligence_share = 0.3; // Fraction of leaks foreign-backed

    double cyber_surveillance = 0.3;
    bool   mass_surveillance_active = false;   // Bulk collection in operation
    double social_media_monitoring = 0.4;      // Monitoring of public platforms (0–1)
    double facial_recognition_deployment = 0.2; // Public space facial recognition (0–1)

    double attack_detection_prob = 0.6;
    double terrorism_detection_prob = 0.5;     // Specific to terrorist plots
    double cyberattack_detection_prob = 0.4;   // State/non-state cyber intrusions
    double early_warning_time_hours = 48.0;    // Average advance warning

    // Defense
    double troop_morale = 0.8;
    double veteran_benefit_adequacy = 0.6;     // Veterans' support programs quality (0–1)
    double desertion_rate = 0.005;             // Annual fraction leaving without authorization
    double combat_stress_index = 0.1;          // PTSD/combat fatigue burden (0–1)

    double equipment_modernization = 0.6;
    double equipment_readiness = 0.7;          // Operational availability (0–1)
    double defense_industrial_base = 0.3;      // Domestic arms production (0–1)

    double air_force_strength = 0.5;
    int    combat_aircraft = 30;               // Fighter/attack aircraft count
    double drone_capability = 0.2;             // UAV/UCAV operational capacity (0–1)

    double naval_force_strength = 0.5;
    int    submarines = 0;                     // Submarine fleet size
    double sea_lane_control = 0.4;             // Maritime trade route protection (0–1)
    bool   blue_water_capability = false;      // Can project force beyond coastal waters

    double military_insubordination_prob = 0.05;
    double politicized_officer_corps = 0.2;    // Share of officers with partisan affiliations (0–1)
    double military_business_interests = 0.1;  // Fraction of economy controlled by military (0–1)

    double military_spending_gdp = 0.02;
    double personnel_vs_equipment_ratio = 0.6; // Fraction of budget on personnel vs hardware
    double corruption_in_procurement = 0.15;   // Kickback/overpricing in defense contracts (0–1)

    // Communication and Propaganda
    double media_control = 0.3;
    double media_pluralism = 0.6;              // Number and independence of outlets (0–1)
    int    journalists_jailed = 0;             // Journalists currently imprisoned
    double ad_market_dependence = 0.4;         // Media reliance on govt ad spending (0–1)

    double narrative_reach = 0.5;
    double social_media_reach = 0.6;           // Digital platform penetration (0–1)
    double traditional_media_reach = 0.5;      // TV/radio/print reach (0–1)
    double diaspora_narrative_reach = 0.2;     // Influence over citizens abroad (0–1)

    double fake_news_success_prob = 0.2;
    double media_literacy = 0.5;               // Population's disinfo resistance (0–1)
    double foreign_disinfo_operations = 0.1;   // Foreign-backed disinfo intensity (0–1)
    double fact_checking_ecosystem = 0.4;      // Independent fact-checking strength (0–1)

    double press_freedom = 0.8;
    double journalist_safety = 0.8;            // Physical safety of reporters (0–1)
    double legal_harassment_index = 0.2;       // SLAPP suits against press (0–1)
    double self_censorship_rate = 0.15;        // Fraction of journalists self-censoring
};

// 5. Infrastructure and Future System
struct InfrastructureFutureSystem {
    // Environment
    double pollution_prob = 0.4;
    double air_quality_index = 0.7;            // PM2.5/AQI (0=toxic, 1=clean)
    double water_quality_index = 0.8;          // River/groundwater (0–1)
    double soil_contamination_index = 0.1;     // Agricultural land degradation (0–1)

    double fauna_extinction_prob = 0.1;
    double biodiversity_index = 0.7;           // Overall ecosystem health (0–1)
    double protected_area_coverage = 0.15;     // Fraction of territory under protection
    double deforestation_rate = 0.01;          // Annual forest cover loss (fraction)
    double illegal_wildlife_trade = 0.1;       // Poaching/trafficking intensity (0–1)

    double drought_prob = 0.1;
    bool drought_active = false;
    int drought_duration = 0;
    double drought_severity = 0.0;
    double crop_loss_pct = 0.0;

    double storm_prob = 0.1;
    bool storm_active = false;
    int storm_aftermath_turns = 0;   // Turns of recovery after impact
    double storm_damage = 0.0;       // Infrastructure damage (absolute $)
    double storm_casualties = 0.0;

    double tornado_prob = 0.01;
    bool tornado_event = false;
    double tornado_damage = 0.0;
    double tornado_casualties = 0.0;
    double flood_prob = 0.15;                  // River/coastal flood probability
    bool flood_active = false;
    int flood_duration = 0;
    double flood_damage = 0.0;
    double climate_vulnerability_index = 0.3;  // Composite climate shock exposure (0–1)
    double climate_adaptation_investment = 0.005; // % GDP in climate adaptation

    double co2_emissions = 1000000.0;
    double co2_per_capita = 5.0;               // Tons CO2 per person per year
    double emissions_trajectory = -0.01;       // Annual change rate (negative=declining)
    bool   carbon_tax_active = false;          // Carbon pricing in force
    double carbon_tax_rate = 0.0;              // USD per ton CO2

    // Science and Technology
    double st_investment_gdp = 0.01;
    double private_rd_gdp = 0.02;              // Private sector R&D (% GDP)
    double total_rd_gdp = 0.03;                // Combined R&D intensity
    double researcher_density = 500.0;         // Researchers per million population

    double private_investment = 0.02;
    double fdi_inflow_gdp = 0.03;              // Foreign direct investment inflows (% GDP)
    double investment_climate_index = 0.6;     // Ease of doing business (0–1)
    double capital_flight_risk = 0.1;          // Probability of capital flight event

    int patent_development = 100;
    int    patents_granted_international = 10; // Patents approved in foreign jurisdictions
    double patent_commercialization = 0.2;     // Fraction of patents reaching market (0–1)
    double technology_transfer_index = 0.3;    // Absorption of foreign technology (0–1)

    double innovation_index = 0.4;
    double startup_ecosystem_strength = 0.3;   // VC funding, accelerators (0–1)
    double university_research_quality = 0.4;  // Academic output/citations (0–1)
    double industry_academia_linkage = 0.3;    // Tech transfer between academia/industry (0–1)

    // Infrastructure
    double road_connectivity = 0.7;
    double rail_connectivity = 0.4;            // Rail freight + passenger network (0–1)
    double road_quality_index = 0.6;           // Pavement condition (0–1)
    double logistics_performance = 0.6;        // World Bank LPI-style composite (0–1)

    double port_capacity = 0.6;
    int    major_ports = 2;                    // Deep-water commercial ports
    double port_efficiency = 0.6;              // Container turnaround (0–1)

    int airports = 5;
    int    international_airports = 1;         // Airports with international routes
    double aviation_safety_rating = 0.8;       // ICAO compliance (0–1)

    double internet_coverage = 0.8;
    double broadband_penetration = 0.5;        // Fixed broadband per capita
    double avg_internet_speed_mbps = 30.0;     // National average download
    double digital_divide_rural = 0.3;         // Urban-rural access gap (0–1)

    double potable_water_access = 0.9;
    double water_stress_index = 0.3;           // Demand vs freshwater supply (0–1)
    double sanitation_coverage = 0.85;         // Access to adequate sanitation (0–1)
    double water_infrastructure_age = 30.0;    // Average age of water systems (years)

    double maintenance_level = 0.6;
    double infrastructure_depreciation_rate = 0.03; // Annual value lost to neglect
    double deferred_maintenance_backlog = 0.0; // Cumulative unfunded repairs (USD)
    double public_works_spending_gdp = 0.04;   // Annual infra investment (% GDP)

    // Energy
    double generation_capacity = 1.2;
    double grid_resilience = 0.6;              // Resistance to cascading failures (0–1)
    double transmission_loss_rate = 0.08;      // Electricity lost in transmission
    double peak_demand_coverage = 0.9;         // Capacity to meet seasonal peak

    double fossil_fuel_dependency = 0.7;
    double energy_transition_speed = 0.0;      // Annual reduction in fossil dependency
    double stranded_asset_risk = 0.2;          // Fossil asset devaluation exposure (0–1)
    bool   net_zero_commitment = false;        // Formal net-zero target in law

    double renewables_percentage = 0.2;
    double solar_capacity_gw = 1.0;            // Installed solar (GW)
    double wind_capacity_gw = 0.5;             // Installed wind (GW)
    double energy_storage_hours = 0.5;         // Grid storage capacity (hours)
    double grid_curtailment_rate = 0.05;       // Renewable energy wasted

    double oil_gas_reserves = 1000000.0;
    double reserve_depletion_rate = 0.03;      // Annual depletion fraction
    double exploration_success_rate = 0.2;     // Probability exploration finds reserves

    double blackout_prob = 0.05;
    bool blackout_active = false;
    int blackout_duration = 0;
    double blackout_economic_loss = 0.0;
    double saidi_hours = 10.0;                 // System Average Interruption Duration (hrs/yr)
    double critical_infrastructure_backup = 0.6; // Hospitals/govt on backup power (0–1)
    double cyber_grid_vulnerability = 0.2;     // Grid cyberattack exposure (0–1)

    double kwh_price = 0.15;
    double energy_subsidy_gdp = 0.01;          // Energy subsidies as % GDP
    double energy_affordability = 0.7;         // Share able to afford adequate energy (0–1)
    double industrial_kwh_price = 0.10;        // Industrial electricity price (USD/kWh)

    // Space Race
    int satellite_capacity = 2;
    int    satellites_civil = 2;               // Communication/weather/navigation
    int    satellites_military = 0;            // Reconnaissance/signals intelligence

    double space_budget = 0.0;
    double space_budget_gdp = 0.0;             // Space spending as % GDP
    bool   own_launch_capability = false;      // Independent orbital launch vehicle
    int    international_space_partnerships = 0; // Active bilateral space agreements

    double launch_failure_prob = 0.1;
    int    successful_launches = 0;            // Cumulative successful launches
    bool   human_spaceflight_capable = false;  // Can launch crewed missions

    double technological_prestige = 0.3;
    double space_prestige = 0.2;               // International standing in space (0–1)
    double tech_export_share = 0.05;           // High-tech goods as fraction of exports

    // AI
    double state_ai_development = 0.2;
    bool   ai_national_strategy = false;       // Formal national AI strategy exists
    double ai_talent_pool = 0.2;               // Domestic AI research talent (0–1)
    double ai_compute_capacity = 0.1;          // GPU/HPC infrastructure (0–1)
    double ai_data_governance = 0.3;           // Legal framework for AI data use (0–1)

    double employment_automation = 0.1;
    double manufacturing_automation_risk = 0.4; // Factory/assembly job exposure
    double service_automation_risk = 0.3;      // White-collar/service job exposure
    double automation_retraining_investment = 0.001; // % GDP in reskilling programs

    double ai_cyberattack_prob = 0.05;
    double critical_infrastructure_cyber_risk = 0.15; // Grid/water/finance exposure (0–1)
    double state_sponsored_cyber_threat = 0.1; // Foreign state hacking intensity (0–1)
    double cyber_defense_maturity = 0.4;       // National cybersecurity capability (0–1)

    double algorithmic_ethics = 0.5;
    bool   ai_ethics_law = false;              // AI regulation enacted
    double algorithmic_bias_index = 0.3;       // Measured bias in public AI (0–1)
    double ai_accountability_framework = 0.3;  // Audit/explainability requirements (0–1)
    double autonomous_weapons_restraint = 0.7; // Compliance with lethal AI norms (0–1)
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
