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
    
    // Add Suicide to Deaths
    // Base Death Rate + Obesity + Suicide
    double deaths_natural = playerCountry.welfare.population * playerCountry.welfare.death_rate;
    double deaths_suicide = playerCountry.welfare.population * playerCountry.welfare.suicide_rate;
    double total_deaths = deaths_natural + deaths_suicide;

    playerCountry.welfare.population += (int)(births - total_deaths);

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
    // High literacy reduces birth rate naturally
    if (playerCountry.welfare.literacy_rate > 0.95) {
        playerCountry.welfare.birth_rate -= 0.0002;
        if (playerCountry.welfare.birth_rate < 0.005) playerCountry.welfare.birth_rate = 0.005; // Floor
    }

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
    
    // Expense: Aging Index (Retirees) * Generosity (Sustainability factor itself acts as coverage)
    // We assume expense grows with Aging.
    double pension_expense = playerCountry.welfare.aging_index * 2.0; // Retirees are expensive
    
    // Balance
    double pension_balance = labor_market_participation - pension_expense;
    
    // Recession Multiplier: If GDP shrinks, tax revenue collapses, hurting the fund instantly.
    if (playerCountry.economy.growth_rate < 0.0) {
        pension_balance -= 0.05; // Crisis penalty
        std::cout << "[!] RECESSION ALERT: Pension contributions plummeting due to economic contraction." << std::endl;
    }

    // Update Sustainability
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
            double old_wage = playerCountry.welfare.minimum_wage;
            playerCountry.welfare.minimum_wage *= (1.0 + wage_indexation);
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

    // Secondary Enrollment Cap (Cascading from Primary)
    
    // Secondary Enrollment Cap (Cascading from Primary)
     if (playerCountry.welfare.secondary_enrollment > playerCountry.welfare.primary_enrollment) {
        playerCountry.welfare.secondary_enrollment -= 0.01; // Decays if base is missing
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
