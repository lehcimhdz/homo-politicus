#include "Game.hpp"
#include <iostream>
#include <thread> // For sleep
#include <chrono> // For time duration
#include <cstdlib> // For rand
#include <ctime>   // For time

Game::Game() : isRunning(true), turnCount(0) {
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
    std::cout << "\nCommand (next/exit/tax+/tax-/invest_...): ";
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
    // --- BUDGET COMMANDS ---
    else if (command == "wage-") {
        playerCountry.welfare.minimum_wage *= 0.90; // -10%
        std::cout << ">> DECREE: Cutting Minimum Wage to $" << playerCountry.welfare.minimum_wage << std::endl;
        std::cout << "   (Competitiveness +, Poverty ++, Popularity --)" << std::endl;
        playerCountry.politics.popularity -= 0.06;
    }
    // PENSION REFORM
    else if (command == "retire+") {
        playerCountry.welfare.retirement_age += 1.0;
        std::cout << ">> REFORM: Raising Retirement Age to " << playerCountry.welfare.retirement_age << " years." << std::endl;
        std::cout << "   (Pension Sustainability ++, Popularity ---)" << std::endl;
        playerCountry.politics.popularity -= 0.05; // Very Unpopular
        // Immediate Protest Risk
        if (std::rand() % 100 < 30) {
            std::cout << "[!] PROTESTS: Seniors take the streets!" << std::endl;
            playerCountry.welfare.general_strike_prob += 0.05;
        }
    }
    else if (command == "retire-") {
        playerCountry.welfare.retirement_age -= 1.0;
        std::cout << ">> REFORM: Lowering Retirement Age to " << playerCountry.welfare.retirement_age << " years." << std::endl;
        std::cout << "   (Popularity ++, Pension Sustainability --)" << std::endl;
        playerCountry.politics.popularity += 0.03;
    }
    else if (command == "invest_health") {
        std::cout << ">> Investing $10M in Healthcare..." << std::endl;
        playerCountry.economy.gdp -= 10000000;
        playerCountry.welfare.health_coverage += 0.05;
        playerCountry.politics.popularity += 0.02;
        if (playerCountry.welfare.health_coverage > 1.0) playerCountry.welfare.health_coverage = 1.0;
        std::cout << "   Health Coverage is now " << playerCountry.welfare.health_coverage * 100 << "%" << std::endl;
    }
    else if (command == "invest_security") {
        std::cout << ">> Investing $10M in National Security..." << std::endl;
        playerCountry.economy.gdp -= 10000000;
        playerCountry.security.homicide_rate -= 1.0;
        if (playerCountry.security.homicide_rate < 0.0) playerCountry.security.homicide_rate = 0.0;
        playerCountry.politics.popularity += 0.01;
        std::cout << "   Homicide Rate dropped to " << playerCountry.security.homicide_rate << "/100k" << std::endl;
    }
    else if (command == "invest_infra") {
        std::cout << ">> Investing $50M in Infrastructure..." << std::endl;
        playerCountry.economy.gdp -= 50000000;
        playerCountry.infra.road_connectivity += 0.05;
        playerCountry.economy.growth_rate += 0.001; // Permanent growth boost
        std::cout << "   Road Connectivity is now " << playerCountry.infra.road_connectivity * 100 << "%" << std::endl;
    }
    else if (command == "invest_education") {
        std::cout << ">> Investing $20M in Education..." << std::endl;
        playerCountry.economy.gdp -= 20000000;
        
        // Boosts everything a bit
        playerCountry.welfare.primary_enrollment += 0.005; // School buses/meals
        playerCountry.welfare.literacy_rate += 0.01;
        playerCountry.welfare.educational_quality += 0.01;
        
        // Clamps
        if (playerCountry.welfare.primary_enrollment > 1.0) playerCountry.welfare.primary_enrollment = 1.0;
        if (playerCountry.welfare.literacy_rate > 1.0) playerCountry.welfare.literacy_rate = 1.0;
        if (playerCountry.welfare.educational_quality > 1.0) playerCountry.welfare.educational_quality = 1.0;
        
        std::cout << "   Enrollment: " << playerCountry.welfare.primary_enrollment * 100 << "% | Literacy: " << playerCountry.welfare.literacy_rate * 100 << "%" << std::endl; 
    }
    else {
        std::cout << ">> Unknown command." << std::endl;
    }
}

void Game::update() {
    std::cout << "--- Simulating One Year ---" << std::endl;

    // --- OBESITY & HEALTH MECHANICS ---
    // If food is cheap (Low Inflation), people eat more and exercise less.
    if (playerCountry.economy.inflation < 0.03) {
        playerCountry.welfare.obesity_rate += 0.005; // +0.5% Obesity per year if food is cheap
        if (playerCountry.welfare.obesity_rate > 1.0) playerCountry.welfare.obesity_rate = 1.0;
    }

    // Dynamic Death Rate Calculation
    // Base: 0.8%. 
    // Obesity adds up to 2.0% (at 100% obesity).
    // Health removes up to 0.5% (at 100% coverage).
    double base_death_rate = 0.008;
    playerCountry.welfare.death_rate = base_death_rate 
                                     + (playerCountry.welfare.obesity_rate * 0.02) 
                                     - (playerCountry.welfare.health_coverage * 0.005);

    // Clamp Death Rate
    if (playerCountry.welfare.death_rate < 0.001) playerCountry.welfare.death_rate = 0.001;

    // --- MENTAL HEALTH DYNAMICS ---
    // Contextual Evolution:
    // 1. Unemployment: Being jobless is the #1 cause of depression in this model.
    // 2. Inflation: Financial stress.
    // --- RESEARCH & DEVELOPMENT (The Innovation Ladder) ---
    // User Insight: "If basic needs are covered, increase budget".
    // Basics: Unemployment < 8%, Inflation < 5%.
    
    if (playerCountry.welfare.unemployment_rate < 0.08 && playerCountry.economy.inflation < 0.05) {
        // Prosperity: Government feels confident to invest in future.
        // Target: 4% of GDP (Like Israel/Korea).
        if (playerCountry.welfare.research_spending_gdp < 0.04) {
            playerCountry.welfare.research_spending_gdp += 0.001; // +0.1% per year
            std::cout << "[INFO] PROSPERITY: Research Budget increased to " << playerCountry.welfare.research_spending_gdp * 100 << "% of GDP." << std::endl;
        }
    } else {
        // Crisis: Austerity measures. Cut Science first.
        if (playerCountry.welfare.research_spending_gdp > 0.005) { // Floor 0.5%
            playerCountry.welfare.research_spending_gdp -= 0.002;
            std::cout << "[!] AUSTERITY: Research Budget cut due to economic instability." << std::endl;
        }
    }

    // SPENDING EFFECT
    // Money -> Tech Power
    double science_budget_abs = playerCountry.economy.gdp * playerCountry.welfare.research_spending_gdp;
    // Efficiency: Depends on Educational Quality
    double effective_research = science_budget_abs * playerCountry.welfare.educational_quality;
    
    // Tech Power Growth (Slow process)
    // Needs huge investment to move the needle.
    playerCountry.politics.tech_power += (effective_research / 1000000000.0) * 0.05; // Arbitrary scaling
    if (playerCountry.politics.tech_power > 1.0) playerCountry.politics.tech_power = 1.0;

    // 4. Power & Securityion: Loss of hope in society.
    double target_mh = 1.0 
                     - (playerCountry.welfare.unemployment_rate * 2.0) 
                     - (playerCountry.economy.inflation * 1.5)
                     - (playerCountry.politics.administrative_corruption * 0.5); // Corruption hurts soul
    
    // Cap Target
    if (target_mh < 0.1) target_mh = 0.1;
    if (target_mh > 1.0) target_mh = 1.0;

    // Drift actual MH towards Target (Mood changes slowly)
    double mh_drift = (target_mh - playerCountry.welfare.mental_health_index) * 0.2; // 20% adjustment per year
    playerCountry.welfare.mental_health_index += mh_drift;

    // --- SUICIDE RATE CALCULATION ---
    // Inversely proportional to Mental Health.
    // Base 0.00014 (14/100k).
    // If MH drops to 0.5, Rate = Base / 0.5 = 2x Base.
    playerCountry.welfare.suicide_rate = 0.00014 / playerCountry.welfare.mental_health_index;

    // 1. Demographics (Simple Annual Calculation)
    double births = playerCountry.welfare.population * playerCountry.welfare.birth_rate;
    
    // --- MIGRATION DYNAMICS (Vote with your feet) ---
    {
        // Net Migration Rate: Per 1000 people.
        // Positive = Immigration (Brain Gain / Cheap Labor).
        // Negative = Emigration (Brain Drain / Refugee Crisis).
        
        // 1. Economic Pull (GDP per Capita vs Global Average)
        // Assuming Global Avg GDP per Cap is $10k.
        double mig_gdp_per_capita = playerCountry.economy.gdp / playerCountry.welfare.population;
        double economic_pull = (mig_gdp_per_capita - 10000.0) / 10000.0; // Normalized
        
        // 2. Freedom Pull (Rights & Liberties)
        // People flee dictatorships.
        double freedom_score = (playerCountry.welfare.freedom_of_expression 
                              + playerCountry.welfare.freedom_of_worship 
                              + playerCountry.security.press_freedom) / 3.0; // Corrected structs
        double repression_push = playerCountry.welfare.torture_index * 2.0;

        // 3. Safety Pull
        // People flee war and crime.
        double safety_score = 1.0 - (playerCountry.security.homicide_rate / 50.0); // 50 is chaotic
        
        // Total Migration Score (-1.0 to +1.0 roughly)
        double migration_target = (economic_pull * 0.5) 
                                + (freedom_score * 0.3) 
                                + (safety_score * 0.2) 
                                - repression_push;
                                
        // Scale to Rate (Max +/- 2% per year)
        double target_net_migration = migration_target * 0.02;
        
        // Smooth transition
        playerCountry.welfare.net_migration_rate += (target_net_migration - playerCountry.welfare.net_migration_rate) * 0.1;
        
        // Apply Migration
        int net_migrants = playerCountry.welfare.population * playerCountry.welfare.net_migration_rate;
        playerCountry.welfare.population += net_migrants;

        // Brain Drain Logic
        // If migration is negative AND education is high -> Lose Innovation.
        if (playerCountry.welfare.net_migration_rate < -0.005 && playerCountry.welfare.university_enrollment > 0.5) {
            playerCountry.infra.innovation_index -= 0.002; // Smart people leave first
            std::cout << "[!] BRAIN DRAIN: Talented youth are leaving the country." << std::endl;
        }
    }

    // --- DEMOGRAPHIC TRANSITION PART 2 (Death Rate) ---
    // Base Biological Death Rate (Young Population): 0.5%
    double target_death_rate = 0.005;
    
    // 1. Aging Effect (The inevitable)
    // Old people die. +1.5% at max aging.
    target_death_rate += (playerCountry.welfare.aging_index * 0.015);
    
    // 2. Health System (The Shield)
    // High coverage saves lives. -0.5% impact.
    target_death_rate -= (playerCountry.welfare.health_coverage * 0.005);
    
    // 3. Poverty (The Killer)
    // Lack of access, nutrition, stress. +0.5% impact.
    target_death_rate += (playerCountry.welfare.poverty_rate * 0.005);
    
    // 4. Lifestyle (Obesity)
    target_death_rate += (playerCountry.welfare.obesity_rate * 0.005);
    
    // 5. Environment (Pollution)
    // CO2 as proxy for air quality.
    if (playerCountry.infra.co2_emissions > 5000.0) {
        target_death_rate += 0.002;
    }
    
    // Drift (Health changes slowly)
    double death_drift = (target_death_rate - playerCountry.welfare.death_rate) * 0.1;
    playerCountry.welfare.death_rate += death_drift;

    // Floor (Nobody lives forever)
    if (playerCountry.welfare.death_rate < 0.005) playerCountry.welfare.death_rate = 0.005;

    // Add Suicide to Deaths
    // Base Death Rate + Obesity + Suicide
    double deaths_natural = playerCountry.welfare.population * playerCountry.welfare.death_rate;
    double deaths_suicide = playerCountry.welfare.population * playerCountry.welfare.suicide_rate;
    double total_deaths = deaths_natural + deaths_suicide;

    playerCountry.welfare.population += (int)(births - total_deaths);
    
    // --- POPULATION DENSITY (The Pressure Cooker) ---
    // Recalculate based on new population
    playerCountry.welfare.population_density = playerCountry.welfare.population / playerCountry.welfare.land_area;
    
    // EFFECT 1: Agglomeration Bonus (Innovation)
    // High density fosters idea exchange.
    // > 100 people/sqkm starts giving bonus.
    if (playerCountry.welfare.population_density > 100.0) {
        playerCountry.infra.innovation_index += 0.001; 
        if (playerCountry.infra.innovation_index > 1.0) playerCountry.infra.innovation_index = 1.0;
    }
    
    // EFFECT 2: Health Risk (Epidemics)
    // High density eases virus transmission.
    // > 200 people/sqkm increases risk.
    if (playerCountry.welfare.population_density > 200.0) {
        playerCountry.welfare.epidemic_prob += 0.005; 
        if (playerCountry.welfare.epidemic_prob > 0.5) playerCountry.welfare.epidemic_prob = 0.5;
    } else {
        // Natural decay of risk if density is low
        if (playerCountry.welfare.epidemic_prob > 0.1) playerCountry.welfare.epidemic_prob -= 0.001;
    }

    // --- ENROLLMENT DYNAMICS (The Poverty Trap) ---
    // If Unemployment is high (> 10%), families pull kids out of school.
    if (playerCountry.welfare.unemployment_rate > 0.10) {
        playerCountry.welfare.primary_enrollment -= 0.01; 
        std::cout << "[!] SOCIAL CRISIS: High unemployment lowers Primary Enrollment." << std::endl;
    }
    
    // Literacy cannot exceed Primary Enrollment (Long term drag)
    // If Literacy > Enrollment, it decays (old generation dies, new one is uneducated)
    if (playerCountry.welfare.literacy_rate > playerCountry.welfare.primary_enrollment) {
        double decay = (playerCountry.welfare.literacy_rate - playerCountry.welfare.primary_enrollment) * 0.1;
        playerCountry.welfare.literacy_rate -= decay;
    }

    // 2. Economy
    // GDP grows by (growth_rate - inflation) roughly
    // EDUCATION BONUS: Smart workforce = Innovation
    double education_bonus = 0.0;
    if (playerCountry.welfare.literacy_rate > 0.90 && playerCountry.welfare.educational_quality > 0.7) {
        education_bonus = 0.015; // +1.5% Bonus Growth
    }

    double real_growth = playerCountry.economy.growth_rate + education_bonus;
    playerCountry.economy.gdp += playerCountry.economy.gdp * real_growth;

    // --- EDUCATION & DEMOGRAPHICS ---
    // --- DEMOGRAPHIC TRANSITION (The Birth Rate Formula) ---
    // User Insight: "Education and Urbanization reduce birth rates."
    
    // Base Fertility (Developing Nation): 3.5%
    double target_birth_rate = 0.035;
    
    // 1. Education Effect (Female Empowerment)
    // High literacy delays marriage/childbirth significantly.
    // -1.5% at 100% Literacy.
    target_birth_rate -= (playerCountry.welfare.literacy_rate * 0.015);
    
    // 2. Urbanization Effect (Cost of Living)
    // Kids are expensive in cities.
    // -1.0% at 100% Urbanization.
    target_birth_rate -= (playerCountry.welfare.urban_population_ratio * 0.010);
    
    // --- URBANIZATION DYNAMICS (The Great Migration) ---
    // Rural -> Urban Drift.
    
    // Pull Factors (Jobs):
    double modern_economy_score = (playerCountry.politics.tech_power 
                                 + playerCountry.politics.industrial_power 
                                 + playerCountry.politics.financial_power) / 3.0; // 0.0 to ~1.0
    
    // Push Factors (Mechanization & Poverty):
    // High Ag Power = Machines replace farmers.
    double rural_push = (playerCountry.politics.agricultural_power * 0.2) 
                      + (playerCountry.welfare.poverty_rate * 0.1); // Fleeing rural poverty
    
    double target_urbanization = 0.5 + (modern_economy_score * 0.4) + rural_push;
    
    // Resistance (Old people don't move)
    double migration_speed = 0.01 * (1.0 - playerCountry.welfare.aging_index); 
    
    // Apply Drift
    double urbanization_drift = (target_urbanization - playerCountry.welfare.urban_population_ratio) * migration_speed;
    playerCountry.welfare.urban_population_ratio += urbanization_drift;
    
    // Clamp
    if (playerCountry.welfare.urban_population_ratio > 0.95) playerCountry.welfare.urban_population_ratio = 0.95; // Singapore
    if (playerCountry.welfare.urban_population_ratio < 0.10) playerCountry.welfare.urban_population_ratio = 0.10; // Subsistence farming
    
    // 3. Poverty Effect (Survival Strategy)
    // Poor families have more kids for labor/security.
    // +0.5% at 100% Poverty.
    target_birth_rate += (playerCountry.welfare.poverty_rate * 0.005);
    
    // 4. Unemployment Effect (Economic Uncertainty)
    // High unemployment delays family formation.
    // -0.5% impact.
    if (playerCountry.welfare.unemployment_rate > 0.08) {
         target_birth_rate -= 0.002;
    }

    // Apply Drift (Demographics change slowly)
    double demo_drift = (target_birth_rate - playerCountry.welfare.birth_rate) * 0.1; // 10% adjustment/year
    playerCountry.welfare.birth_rate += demo_drift;
    
    // Clamp
    if (playerCountry.welfare.birth_rate < 0.005) playerCountry.welfare.birth_rate = 0.005; // Japan/Korea floor
    if (playerCountry.welfare.birth_rate > 0.05) playerCountry.welfare.birth_rate = 0.05; // Niger ceiling

    // --- LABOR MARKET SATURATION (The Education Paradox) ---
    // Qualified Supply: High School + University Graduates
    double qualified_labor_supply = (playerCountry.welfare.secondary_enrollment + playerCountry.welfare.university_enrollment) / 2.0;
    
    // Economic Demand: Based on advanced sectors (Tech, Finance, modern Industry)
    double labor_demand_quality = (playerCountry.politics.tech_power * 1.2) 
                        + (playerCountry.politics.financial_power * 1.0) 
                        + (playerCountry.politics.industrial_power * 0.5); 
    
    // --- DYNAMIC UNEMPLOYMENT (Complex Model) ---
    // Target Unemployment depends heavily on the economy structure.
    
    // 1. Cyclical: GDP Growth reduces unemployment.
    // If growth is 2% (0.02), it reduces target by 4%.
    double cyclical_factor = - (playerCountry.economy.growth_rate * 2.0);
    

    // --- DEMOGRAPHIC AGING (The Silent Crisis) ---
    // People live longer, but birth rates fall (if educated).
    // Natural Aging Drift: +0.2% per year (Global trend)
    double aging_drift = 0.002;
    // High birth rate slows aging (more young people)
    if (playerCountry.welfare.birth_rate > 0.02) aging_drift -= 0.001; 
    
    playerCountry.welfare.aging_index += aging_drift;
    if (playerCountry.welfare.aging_index > 0.6) playerCountry.welfare.aging_index = 0.6; // Cap (Japan scenario)

    // --- PENSION SYSTEM (The Financial Time Bomb) ---
    // Sustainable if: Workers > Retirees
    // Income: Employment * Formal Labor (Only formal workers pay taxes/pensions)
    double labor_market_participation = (1.0 - playerCountry.welfare.unemployment_rate) * (1.0 - playerCountry.welfare.labor_informality);
    
    // RETIREMENT AGE MODIFIER
    // Standard: 65.
    // Every year above 65 reduces burden by 10%.
    // Every year below 65 increases burden by 10%.
    double age_diff = playerCountry.welfare.retirement_age - 65.0;
    double expense_modifier = 1.0 - (age_diff * 0.10); 
    if (expense_modifier < 0.5) expense_modifier = 0.5; // Min 50% cost (People still get sick)

    double pension_expense = (playerCountry.welfare.aging_index * 2.0) * expense_modifier; // Adjusted Cost
    
    // Balance
    double pension_balance = labor_market_participation - pension_expense;
    
    // Recession Multiplier: If GDP shrinks, tax revenue collapses, hurting the fund instantly.
    if (playerCountry.economy.growth_rate < 0.0) {
        pension_balance -= 0.05; // Crisis penalty
        std::cout << "[!] RECESSION ALERT: Pension contributions plummeting due to economic contraction." << std::endl;
    }

    // Update Sustainability
    playerCountry.welfare.pension_sustainability = pension_balance;
    // It's a slow moving fund (buffer).
    playerCountry.welfare.pension_sustainability += (pension_balance * 0.1); 
    
    // Limits
    if (playerCountry.welfare.pension_sustainability > 1.0) playerCountry.welfare.pension_sustainability = 1.0;
    
    // COLLAPSE CHECK
    if (playerCountry.welfare.pension_sustainability < 0.10) {
        std::cout << "[!!!] PENSION COLLAPSE: The Social Security system is bankrupt." << std::endl;
        std::cout << "      Government forced to print money to pay retirees." << std::endl;
        
        // Bailout Consequence
        playerCountry.economy.gdp -= 100000000; // $100M Bailout
        playerCountry.economy.inflation += 0.05; // Printing money causes Inflation spike
        playerCountry.politics.popularity -= 0.10; // Anger
        playerCountry.welfare.pension_sustainability = 0.30; // Reset with bailout money
    }

    // 2. Structural (Automation): High Tech kills low-skilled jobs.
    // If Tech is 1.0, adds +3% unemployment.
    double automation_factor = playerCountry.politics.tech_power * 0.03;
    
    // 3. Policy (Rigidity): Unions and High Wages make hiring harder.
    double rigidity_factor = (playerCountry.welfare.union_strength * 0.02) 
                           + (playerCountry.welfare.minimum_wage / 10000.0); // Small scaling
                           
    // 4. Mismatch (Oversupply of graduates)
    double mismatch_factor = 0.0;
    if (qualified_labor_supply > labor_demand_quality) {
        mismatch_factor = 0.05; // +5% Structural Unemployment
        
        // Also triggers Brain Drain
        playerCountry.welfare.brain_drain += 0.005;
        if (playerCountry.welfare.brain_drain > 0.4) {
             std::cout << "[!] DEMOGRAPHIC ALERT: Massive Brain Drain! Professionals are fleeing." << std::endl;
             playerCountry.economy.gdp *= 0.998;
        }
    }
    
    // CALCULATE TARGET
    double base_unemployment = 0.04; // Friction
    double target_unemployment = base_unemployment + cyclical_factor + automation_factor + rigidity_factor + mismatch_factor;
    
    // Floor
    if (target_unemployment < 0.02) target_unemployment = 0.02; // Always some friction
    
    // DRIFT (Real economy adjusts slowly)
    double labor_market_flexibility = 0.2; // 20% adjustment per year
    double ue_drift = (target_unemployment - playerCountry.welfare.unemployment_rate) * labor_market_flexibility;
    playerCountry.welfare.unemployment_rate += ue_drift;

     // Wage Stagnation if Unemployment is High (Supply/Demand)
    if (playerCountry.welfare.unemployment_rate > 0.10) {
        playerCountry.welfare.minimum_wage *= 0.995; 
    }

    // --- WAGE POLITICS (The Goldilocks Zone) ---
    // User Insight: Wage should be Median Income (~40% GDP per Capita).
    double gdp_per_capita = playerCountry.economy.gdp / playerCountry.welfare.population;
    double living_wage_target = gdp_per_capita * 0.40; // Rough heuristic for "Median"
    
    // CASE A: TOO LOW (Neoliberal Trap)
    // Wage < 80% of Living Wage
    if (playerCountry.welfare.minimum_wage < (living_wage_target * 0.8)) {
        // Inequality and Poverty Rise
        playerCountry.welfare.poverty_rate += 0.02; 
        // Low Consumption hurts GDP
        playerCountry.economy.growth_rate -= 0.002; 
        // Popularity hit (Working Poor)
        playerCountry.politics.popularity -= 0.01;
        std::cout << "[INFO] INEQUALITY: Wages too low. Poverty rises to " << playerCountry.welfare.poverty_rate * 100 << "%" << std::endl;
    }
    
    // CASE B: TOO HIGH (Populist Trap)
    // Wage > 120% of Living Wage
    else if (playerCountry.welfare.minimum_wage > (living_wage_target * 1.2)) {
        // Businesses pass cost to prices -> Inflation
        playerCountry.economy.inflation += 0.015; 
        // Businesses stop hiring -> Unemployment
        playerCountry.welfare.unemployment_rate += 0.01;
        // Businesses go illegal -> Informality
        playerCountry.welfare.labor_informality += 0.01;
        std::cout << "[!] WAGE SPIRAL: High wages causing Inflation & Informality." << std::endl;
    }
    
    // CASE C: SWEET SPOT
    else {
        // Reduce Poverty slowly
        if (playerCountry.welfare.poverty_rate > 0.05) playerCountry.welfare.poverty_rate -= 0.01;
    }

    // --- DYNAMIC UNION STRENGTH (Organizational Power) ---
    // Unions grow when labor is scarce (Low Unemployment) and Inflation scares people.
    if (playerCountry.welfare.unemployment_rate < 0.06) {
        playerCountry.welfare.union_strength += 0.005; // Workers have leverage
    } else if (playerCountry.welfare.unemployment_rate > 0.12) {
        playerCountry.welfare.union_strength -= 0.005; // Workers are desperate, accept anything
    }
    
    // Inflation drives membership (Safety outcome)
    if (playerCountry.economy.inflation > 0.05) {
        playerCountry.welfare.union_strength += 0.002;
    }
    
    // Informality kills unions (Can't organize ghost workers)
    if (playerCountry.welfare.labor_informality > 0.40) {
        playerCountry.welfare.union_strength -= 0.01;
    }
    
    // Cap
    if (playerCountry.welfare.union_strength > 0.9) playerCountry.welfare.union_strength = 0.9;
    if (playerCountry.welfare.union_strength < 0.05) playerCountry.welfare.union_strength = 0.05;


    // --- UNION BARGAINING (The Counter-Force) ---
    // Unions fight to index wages to inflation.
    if (playerCountry.economy.inflation > 0.02) {
        // They demand to match inflation.
        // Power determines how much they get.
        double wage_indexation = playerCountry.economy.inflation * playerCountry.welfare.union_strength;
        
        // Apply Raise
        if (wage_indexation > 0.001) {
            playerCountry.welfare.minimum_wage *= 1.05; // +5% wage_indexation);
            // std::cout << "[INFO] LABOUR: Unions negotiated a wage increase to match inflation." << std::endl;
        }

        // Dissatisfaction: If they didn't get full inflation match, they get angry.
        double lost_purchasing_power = playerCountry.economy.inflation - wage_indexation;
        if (lost_purchasing_power > 0.01) {
             // ECONOMIC STRIKE MOTIVATION
             playerCountry.welfare.general_strike_prob += (lost_purchasing_power * 5.0); 
        }
    }
    
    // POLITICAL STRIKE MOTIVATION
    // Unions also strike against Corruption and Unpopularity.
    if (playerCountry.politics.administrative_corruption > 0.30) {
        // "Moral Strike" against the regime
        playerCountry.welfare.general_strike_prob += 0.005; 
    }
    if (playerCountry.politics.popularity < 0.25) {
        // "Opportunistic Strike": They smell blood.
        playerCountry.welfare.general_strike_prob += 0.01;
    }

    // GENERAL STRIKE CHECK
    // Probability accumulates until they blow up.
    if (playerCountry.welfare.general_strike_prob > 0.05) { // 5% risk threshold to start rolling
        int roll = std::rand() % 100;
        if (roll < (playerCountry.welfare.general_strike_prob * 100)) {
            std::cout << "[!!!] GENERAL STRIKE! The country is paralyzed." << std::endl;
            // Consequence:
            playerCountry.economy.gdp *= 0.97; // -3% GDP (Production Halt)
            playerCountry.politics.popularity -= 0.05; // Chaos
            playerCountry.welfare.general_strike_prob = 0.0; // Tension released
            
            // Forced Negotiation
            playerCountry.welfare.minimum_wage *= 1.05; // They win +5%
            std::cout << "      Government forced to raise Minimum Wage by 5%." << std::endl;
        } else {
            // Tension cools slightly if no strike happens (fatigue)
             playerCountry.welfare.general_strike_prob *= 0.90;
        }
    }

    // --- THE CYCLE OF POVERTY (Misery Index) ---
    // Unemployment feeds poverty directly.
    // If 10% are unemployed, they are poor.
    double new_poor = playerCountry.welfare.unemployment_rate * 0.5; // Half of unemployed become destitute
    playerCountry.welfare.poverty_rate += new_poor;
    
    // Inflation also creates poverty (Working Poor)
    if (playerCountry.economy.inflation > 0.10) {
        playerCountry.welfare.poverty_rate += 0.01; // Erosion of savings
    }
    
    // Mitigation: Education (Social Mobility)
    // High Literacy allows people to escape poverty over time.
    if (playerCountry.welfare.literacy_rate > 0.80) {
        playerCountry.welfare.poverty_rate -= 0.005; 
    }
    
    // Cap Poverty
    if (playerCountry.welfare.poverty_rate > 0.90) playerCountry.welfare.poverty_rate = 0.90;
    
    // --- CRIME & INSTABILITY (Consequences of Poverty) ---
    // Desperation leads to Crime.
    // Base Homicide Rate grows with Poverty.
    double crime_driver = (playerCountry.welfare.poverty_rate * 2.0) 
                        + (playerCountry.welfare.unemployment_rate * 1.0)
                        + (playerCountry.politics.administrative_corruption * 0.5); // Impunity
    
    playerCountry.security.homicide_rate += crime_driver;
    
    // Security Spending (invest_security) is the only counter-force (Manual command reducing it).
    // So if you don't invest, crime spirals.
    
    // Radicalization: Poor people lose faith in the system.
    if (playerCountry.welfare.poverty_rate > 0.30) {
        playerCountry.politics.polarization_index += 0.02;
    }
    
    // Urban/Rural Divide (The Culture War)
    // Polarization is highest when society is split 50/50.
    // It is lowest when society is homogeneous (mostly rural or mostly urban).
    double urban_split = std::abs(playerCountry.welfare.urban_population_ratio - 0.5); // 0.0 (Split) to 0.5 (Homogeneous)
    // If split is close to 0 (50/50), add polarization.
    if (urban_split < 0.1) {
        playerCountry.politics.polarization_index += 0.01; // Tension between lifestyles
    } else if (urban_split > 0.3) {
        playerCountry.politics.polarization_index -= 0.005; // Homogeneous society
    }

    // Secondary Enrollment Cap (Cascading from Primary)
    
    // Secondary Enrollment Cap (Cascading from Primary)
     if (playerCountry.welfare.secondary_enrollment > playerCountry.welfare.primary_enrollment) {
        playerCountry.welfare.secondary_enrollment -= 0.01; // Decays if base is missing
    }

    // --- CLERICAL INFLUENCE (Faith vs Reason) ---
    // Base: 0.3 (Modern Secular State).
    // Increasers:
    // - Rural Population (Traditionalism).
    // - Poverty (Faith as refuge).
    // Decreasers:
    // - Education (Scientific worldview).
    // - Urbanization (Cosmopolitanism).
    
    double traditionalism = (1.0 - playerCountry.welfare.urban_population_ratio) * 0.5 
                          + playerCountry.welfare.poverty_rate * 0.5;
                          
    double secularization = playerCountry.welfare.literacy_rate * 0.4 
                          + playerCountry.welfare.university_enrollment * 0.4;
                          
    double target_clerical_influence = 0.3 + traditionalism - secularization;
    
    // Clamp
    if (target_clerical_influence < 0.05) target_clerical_influence = 0.05; // Atheist State
    if (target_clerical_influence > 0.95) target_clerical_influence = 0.95; // Theocracy
    
    // Drift
    playerCountry.welfare.clerical_political_influence += (target_clerical_influence - playerCountry.welfare.clerical_political_influence) * 0.05; // Culture changes slowly

    // EFFECTS of Clerical Influence
    // 1. Birth Rate Bonus (Pro-family values)
    // If influence > 0.5, boost birth rate.
    if (playerCountry.welfare.clerical_political_influence > 0.5) {
        // Can offset urbanization effect by up to 0.5%
        double religious_bonus = (playerCountry.welfare.clerical_political_influence - 0.5) * 0.01;
        playerCountry.welfare.birth_rate += religious_bonus;
    }

    // 2. Minority Rights (Conservative pushback)
    // High influence reduces minority protection.
    double rights_penalty = playerCountry.welfare.clerical_political_influence * 0.1;
    playerCountry.welfare.minority_protection -= (rights_penalty * 0.01); // Slow erosion
    if (playerCountry.welfare.minority_protection < 0.1) playerCountry.welfare.minority_protection = 0.1;

    // --- RELIGIOUS TENSION & RADICALISM ---
    // 1. Radicalization Drivers:
    // - Poverty (Despair).
    // - High Clerical Influence (Dogmatism).
    // - Low Education (Lack of critical thinking).
    // - Repression (Reaction to state control).
    
    double radicalism_drivers = (playerCountry.welfare.poverty_rate * 0.4) 
                              + (playerCountry.welfare.clerical_political_influence * 0.3)
                              + (playerCountry.welfare.torture_index * 0.3);
                              
    double radicalism_inhibitors = (playerCountry.welfare.literacy_rate * 0.5) 
                                 + (playerCountry.welfare.freedom_of_worship * 0.5); // Tolerance
                                 
    double target_radicalism = 0.05 + radicalism_drivers - (radicalism_inhibitors * 0.5);
    if (target_radicalism < 0.0) target_radicalism = 0.0;
    
    playerCountry.welfare.radicalism_prob += (target_radicalism - playerCountry.welfare.radicalism_prob) * 0.1;
    
    // 2. Inter-religious Tension
    // Tension rises if there is Radicalism AND Diversity (assumed if freedom > 0.5).
    // If freedom is low, state enforces one religion -> Tension is "suppressed" but latent?
    // Let's model it as: Radicalism * Diversity Factor.
    
    double diversity_factor = playerCountry.welfare.freedom_of_worship; // Proxy for having multiple faiths
    
    double target_tension = playerCountry.welfare.radicalism_prob * 2.0 * diversity_factor;
    
    // Mitigation: Effective police/intelligence can reduce violence (tension manifestation).
    double security_mitigation = playerCountry.security.attack_detection_prob * 0.5;
    target_tension -= security_mitigation;
    
    if (target_tension < 0.0) target_tension = 0.0;
    if (target_tension > 1.0) target_tension = 1.0; // Civil War risk
    
    playerCountry.welfare.interreligious_tension += (target_tension - playerCountry.welfare.interreligious_tension) * 0.1;

    // EFFECTS
    // Tension fuels Polarization.
    if (playerCountry.welfare.interreligious_tension > 0.3) {
        playerCountry.politics.polarization_index += 0.005;
    }
    
    // --- TERRORISM RISK (The Radicalism Trap) ---
    // If Radicalism is high, attacks can happen regardless of general tension.
    // Threshold: 15% Radicalism.
    if (playerCountry.welfare.radicalism_prob > 0.15) {
        double attack_chance = (playerCountry.welfare.radicalism_prob - 0.15); // Base risk
        
        // Intelligence Mitigation (The Shield)
        double intel_shield = playerCountry.security.attack_detection_prob * 0.8;
        double net_risk = attack_chance * (1.0 - intel_shield);
        
        // Roll Dice (Annual Check)
        // Simple random: rand() / RAND_MAX < net_risk
        double roll = (double)rand() / RAND_MAX;
        
        if (roll < net_risk) {
            std::cout << "[!!!] TERRORIST ATTACK: Radical extremists struck a major city!" << std::endl;
            
            // Consequences
            int casualties = 50 + (rand() % 500); // 50-550 deaths
            playerCountry.welfare.population -= casualties;
            
            playerCountry.politics.popularity -= 0.05; // Leader blamed
            playerCountry.economy.gdp *= 0.995; // Economic shock (-0.5%)
            
            // Panic reaction: Demand for security
            playerCountry.politics.polarization_index += 0.02; // Fear divides
            
            std::cout << "      " << casualties << " casualties reported. Market panics." << std::endl;
        } else if (net_risk > 0.05) {
            // Near Miss (Intelligence Victory?)
            if ((double)rand() / RAND_MAX < 0.3) {
                 std::cout << "[INFO] INTELLIGENCE: A major terror plot was foiled by security services." << std::endl;
                 playerCountry.politics.popularity += 0.02; // Credit for safety
            }
        }
    }

    // --- POLITICAL INSTABILITY (The Modern Trap) ---
    // If Education is High, but Corruption is High -> People get angry.
    if (playerCountry.welfare.literacy_rate > 0.90 && playerCountry.politics.administrative_corruption > 0.30) {
        std::cout << "[!] UNREST: Educated citizens demand transparency!" << std::endl;
        playerCountry.politics.polarization_index += 0.05;
        playerCountry.politics.marches += 1; // More protests
        playerCountry.politics.popularity -= 0.02;
    }

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

    // --- HEALTH DEPRECIATION ---
    // Hospitals degrade. Equipment breaks. Doctors retire.
    // You must invest CONSTANTLY to maintain coverage.
    double health_decay = 0.02; // -2% coverage per year naturally
    if (playerCountry.economy.growth_rate < 0) {
        health_decay += 0.02; // Crisis exacerbates decay (budget cuts)
    }
    playerCountry.welfare.health_coverage -= health_decay;
    if (playerCountry.welfare.health_coverage < 0.0) playerCountry.welfare.health_coverage = 0.0;
    
    // --- RADIATION EFFECTS ---
    if (playerCountry.welfare.food_radiation_prob > 0.0) {
        std::cout << "[!] WARNING: Radioactive contamination causes excess deaths and cleanup costs." << std::endl;
        playerCountry.welfare.death_rate += 0.005; // +0.5% deaths per year (Cumulative if not careful, but here fixed addition)
        playerCountry.economy.gdp -= 10000000; // $10M annual cleanup cost
    }
    
    // --- ELECTION LOGIC ---
    turnCount++;
    if (turnCount % 4 == 0) {
        std::cout << "\n=== ELECTION YEAR (Year " << turnCount << ") ===" << std::endl;
        std::cout << "Current Popularity: " << playerCountry.politics.popularity * 100 << "%" << std::endl;
        
        if (playerCountry.politics.popularity > 0.50) {
            std::cout << "VICTORY: The people love you! You have been re-elected for 4 more years." << std::endl;
        } else {
            std::cout << "DEFEAT: You have lost the support of the people." << std::endl;
            std::cout << "GAME OVER." << std::endl;
            isRunning = false; // Stop the game
        }
    }
}

void Game::render() {
    // Clear screen (hacky way for terminal)
    // std::cout << "\033[2J\033[1;1H"; 

    std::cout << "\nGame Tick Update:" << std::endl;
    playerCountry.printStatus();
}
