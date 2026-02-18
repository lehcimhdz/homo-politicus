#include "Game.hpp"
#include <iostream>
#include <thread> // For sleep
#include <chrono> // For time duration

Game::Game() : isRunning(true), nextTurn(false), turnCount(0), rng(std::random_device{}()) {
    std::cout << "Initializing Game..." << std::endl;
    // std::srand removed, using std::mt19937 initialized in member initializer list
}

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
        std::uniform_int_distribution<> dist(0, 99);
        if (dist(rng) < 30) {
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
    
    // --- RELIGIOUS FREEDOM COMMANDS ---
    else if (command == "worship+") {
        playerCountry.welfare.freedom_of_worship += 0.1;
        if (playerCountry.welfare.freedom_of_worship > 1.0) playerCountry.welfare.freedom_of_worship = 1.0;
        
        std::cout << ">> POLICY: Religious Freedom increased to " << playerCountry.welfare.freedom_of_worship << std::endl;
        std::cout << "   (Happiness +, Secularism +, Tension +)" << std::endl;
        
        // Benefits of Liberty
        playerCountry.welfare.mental_health_index += 0.02; // People are happier
        playerCountry.welfare.clerical_political_influence -= 0.05; // Separation of Church/State
        
        // Risks of Liberty (Diversity Tension)
        // Initial shock of new ideas/cults can cause friction
        playerCountry.welfare.interreligious_tension += 0.05; 
    }
    else if (command == "worship-") {
        playerCountry.welfare.freedom_of_worship -= 0.1;
        if (playerCountry.welfare.freedom_of_worship < 0.0) playerCountry.welfare.freedom_of_worship = 0.0;
        
        std::cout << ">> POLICY: State Religion enforced." << std::endl;
        std::cout << "   (Control +, Radicalism +, Happiness -)" << std::endl;
        
        // Costs of Repression
        playerCountry.welfare.mental_health_index -= 0.03; // Oppression
        playerCountry.welfare.radicalism_prob += 0.04; // Underground resistance
        playerCountry.welfare.clerical_political_influence += 0.10; // Church empowers State
        
        // Benefit of Homogeneity
        playerCountry.welfare.interreligious_tension -= 0.10; // Enforced peace
    }
    
    // --- HUMAN RIGHTS COMMANDS ---
    else if (command == "torture+") {
        playerCountry.welfare.torture_index += 0.1;
        if (playerCountry.welfare.torture_index > 1.0) playerCountry.welfare.torture_index = 1.0;
        
        std::cout << ">> SECRET ORDER: Enhanced Interrogation techniques authorized." << std::endl;
        std::cout << "   (Intel ++, Radicalism +, UN Score -, Rights -)" << std::endl;
        
        // The Dark Trade-off
        playerCountry.security.attack_detection_prob += 0.15; // Useful info extracted
        if (playerCountry.security.attack_detection_prob > 1.0) playerCountry.security.attack_detection_prob = 1.0;
        
        playerCountry.welfare.radicalism_prob += 0.05; // Revenge/Martyrdom
        playerCountry.welfare.un_score -= 0.15; // International Condemnation
    }
    else if (command == "torture-") {
        playerCountry.welfare.torture_index -= 0.1;
        if (playerCountry.welfare.torture_index < 0.0) playerCountry.welfare.torture_index = 0.0;
        
        std::cout << ">> REFORM: Banning torture and closing secret prisons." << std::endl;
        std::cout << "   (Intel --, Radicalism -, UN Score ++)" << std::endl;
        
        // Restoring Dignity
        playerCountry.security.attack_detection_prob -= 0.10; // Harder to get info?
        if (playerCountry.security.attack_detection_prob < 0.0) playerCountry.security.attack_detection_prob = 0.0;
        
        playerCountry.welfare.radicalism_prob -= 0.03; // Healing society
        playerCountry.welfare.un_score += 0.10; // International Praise
    }
    
    // --- FORCED DISAPPEARANCES (Dirty War) ---
    else if (command == "disappear+") {
        playerCountry.welfare.forced_disappearances += 0.2;
        if (playerCountry.welfare.forced_disappearances > 1.0) playerCountry.welfare.forced_disappearances = 1.0;

        std::cout << ">> SECRET OPERATION: ' Night and Fog ' decree signed." << std::endl;
        std::cout << "   (Protests --, Fear ++, UN Score ---, Hate +++)" << std::endl;

        // The Logic of Terror
        // 1. Fear paralyzes the streets
        playerCountry.politics.mobilizations = 0; // Immediate silence
        playerCountry.welfare.general_strike_prob *= 0.5; 
        
        // 2. The cost is deep hatred
        playerCountry.welfare.radicalism_prob += 0.08; 
        playerCountry.welfare.un_score -= 0.20; // Pariah state
        playerCountry.politics.popularity -= 0.10; // People are terrified, not happy
    }
    else if (command == "disappear-") {
        playerCountry.welfare.forced_disappearances -= 0.2;
        if (playerCountry.welfare.forced_disappearances < 0.0) playerCountry.welfare.forced_disappearances = 0.0;

        std::cout << ">> JUSTICE: Truth Commission established to find the missing." << std::endl;
        std::cout << "   (Justice ++, Protests ++, UN Score ++)" << std::endl;

        // The Cost of Truth
        // 1. Justice emboldens people to speak out
        playerCountry.politics.mobilizations += 2; // "Never Again!" marches
        
        // 2. Healing
        playerCountry.welfare.radicalism_prob -= 0.05; 
        playerCountry.welfare.un_score += 0.15;
        playerCountry.politics.popularity += 0.05; 
    }

    // --- FREEDOM OF EXPRESSION (The Fourth Estate) ---
    else if (command == "press+") {
        playerCountry.welfare.freedom_of_expression += 0.1;
        if (playerCountry.welfare.freedom_of_expression > 1.0) playerCountry.welfare.freedom_of_expression = 1.0;
        
        std::cout << ">> POLICY: Censorship lifted. Press is free." << std::endl;
        std::cout << "   (Innovation ++, Corruption --, Stability -)" << std::endl;
        
        // Benefits of Truth
        playerCountry.infra.innovation_index += 0.05; // Ideas flow freely
        playerCountry.politics.administrative_corruption -= 0.05; // Watchdog effect
        
        // The Pain of Truth
        // Scandals are revealed!
        std::uniform_real_distribution<> dist(0.0, 1.0);
        if (dist(rng) < 0.4) {
            std::cout << "[!] SCANDAL: Free press exposes government corruption!" << std::endl;
            playerCountry.politics.popularity -= 0.08;
            playerCountry.politics.polarization_index += 0.03;
        }
    }
    else if (command == "press-") {
        playerCountry.welfare.freedom_of_expression -= 0.1;
        if (playerCountry.welfare.freedom_of_expression < 0.0) playerCountry.welfare.freedom_of_expression = 0.0;
        
        std::cout << ">> POLICY: Media access restricted. Internet filtered." << std::endl;
        std::cout << "   (Control ++, Innovation --, Corruption ++)" << std::endl;
        
        // Benefits of Silence
        // Leader controls the narrative.
        playerCountry.politics.popularity += 0.02; // "Everything is fine"
        
        // Costs of Silence
        playerCountry.infra.innovation_index -= 0.08; // Stagnation
        playerCountry.politics.administrative_corruption += 0.05; // Impunity
        playerCountry.welfare.radicalism_prob += 0.02; // Anger builds in dark corners
    }
    // --- MINORITY RIGHTS (The Diversity Paradox) ---
    else if (command == "minority+") {
        playerCountry.welfare.minority_protection += 0.1;
        if (playerCountry.welfare.minority_protection > 1.0) playerCountry.welfare.minority_protection = 1.0;
        
        std::cout << ">> POLICY: Anti-discrimination laws passed." << std::endl;
        std::cout << "   (UN Score ++, Radicalism --, Popularity -, Polarization +)" << std::endl;
        
        // International Praise & Social Peace
        playerCountry.welfare.un_score += 0.05;
        playerCountry.welfare.radicalism_prob -= 0.03; // Minorities feel included
        
        // The Backlash (if society is conservative/polarized)
        playerCountry.politics.popularity -= 0.03; // "Woke agenda" backlash
        playerCountry.politics.polarization_index += 0.02; // Culture war
    }
    else if (command == "minority-") {
        playerCountry.welfare.minority_protection -= 0.1;
        if (playerCountry.welfare.minority_protection < 0.0) playerCountry.welfare.minority_protection = 0.0;
        
        std::cout << ">> POLICY: Nationalistic rhetoric adopted. Minorities scapegoated." << std::endl;
        std::cout << "   (Popularity ++, UN Score --, Radicalism ++, Brain Drain ++)" << std::endl;
        
        // Populist Boost
        playerCountry.politics.popularity += 0.04; // "Put our people first"
        
        // The Human Cost
        playerCountry.welfare.un_score -= 0.08;
        playerCountry.welfare.radicalism_prob += 0.05; // Margins become dangerous
    }
    // --- DIPLOMACY (The World Stage) ---
    else if (command == "diplomacy+") {
        std::cout << ">> MISSION: Diplomatic lobby launched." << std::endl;
        playerCountry.economy.gdp -= 50000000; // $50M cost
        playerCountry.welfare.un_score += 0.05;
        if (playerCountry.welfare.un_score > 1.0) playerCountry.welfare.un_score = 1.0;
        
        // Lower Sanctions Risk
        playerCountry.economy.international_sanctions_prob -= 0.05;
        if (playerCountry.economy.international_sanctions_prob < 0.0) playerCountry.economy.international_sanctions_prob = 0.0;
        
        std::cout << "   (UN Score ++, Sanctions Risk --, Cost $50M)" << std::endl;
    }
    else if (command == "diplomacy-") {
        std::cout << ">> POLICY: 'Sovereignty First' doctrine." << std::endl;
        playerCountry.politics.popularity += 0.03; // Nationalist boost
        playerCountry.welfare.un_score -= 0.10;
        if (playerCountry.welfare.un_score < 0.0) playerCountry.welfare.un_score = 0.0;
        
        // Sanctions Risk increases
        playerCountry.economy.international_sanctions_prob += 0.05;
        
        std::cout << "   (Popularity ++, UN Score --, Sanctions Risk ++)" << std::endl;
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
    else if (command == "minority-") {
        playerCountry.welfare.minority_protection -= 0.1;
        if (playerCountry.welfare.minority_protection < 0.0) playerCountry.welfare.minority_protection = 0.0;
        
        playerCountry.politics.popularity += 0.04;    // Populist boost
        playerCountry.welfare.radicalism_prob += 0.05; 
        playerCountry.welfare.un_score -= 0.08; 
        std::cout << ">> POLICY: Nationalist rhetoric enforced. Popularity up, minorities excluded." << std::endl;
        std::cout << ">> POLICY: Nationalist rhetoric enforced. Popularity up, minorities excluded." << std::endl;
    }
    // --- CENTRAL BANK & MONETARY COMMANDS ---
    else if (command == "autonomy+") {
        playerCountry.economy.central_bank_autonomy += 0.2;
        if (playerCountry.economy.central_bank_autonomy > 1.0) playerCountry.economy.central_bank_autonomy = 1.0;
        
        playerCountry.welfare.un_score += 0.05; // Investors like it
        std::cout << ">> INSTITUTION: Central Bank granted more independence." << std::endl;
        std::cout << "   (FDI ++, Inflation Expectations --, Control --)" << std::endl;
    }
    else if (command == "autonomy-") {
        playerCountry.economy.central_bank_autonomy -= 0.2;
        if (playerCountry.economy.central_bank_autonomy < 0.0) playerCountry.economy.central_bank_autonomy = 0.0;
        
        playerCountry.welfare.un_score -= 0.10; // Investors hate it
        playerCountry.economy.international_reserves *= 0.90; // Capital Flight immediate shock
        std::cout << ">> INSTITUTION: Central Bank intervened by the Executive." << std::endl;
        std::cout << "   (Control ++, FDI --, Capital Flight !!!)" << std::endl;
    }
    else if (command == "interest+") {
        // Autonomy Check: If high autonomy, politician interfering costs popularity
        if (playerCountry.economy.central_bank_autonomy > 0.6) {
             playerCountry.politics.popularity -= 0.05;
             std::cout << "[!] INTERFERENCE: The Central Bank resents your pressure. (Popularity -5%)" << std::endl;
        }
        
        playerCountry.economy.interest_rate += 0.01; // +1%
        playerCountry.politics.popularity -= 0.02;    // People hate high mortgage rates
        std::cout << ">> MONETARY: Interest Rate raised to " << playerCountry.economy.interest_rate * 100 << "%" << std::endl;
        std::cout << "   (Inflation Control ++, Growth --)" << std::endl;
    }
    else if (command == "interest-") {
        playerCountry.economy.interest_rate -= 0.01; 
        if (playerCountry.economy.interest_rate < 0.0) playerCountry.economy.interest_rate = 0.0;
        std::cout << ">> MONETARY: Interest Rate lowered. Borrowing is cheap!" << std::endl;
        std::cout << "   (Growth ++, Inflation Risk ++)" << std::endl;
    }
    else if (command == "print+") {
        // Autonomy Check: Cannot print money if Bank is Independent
         if (playerCountry.economy.central_bank_autonomy > 0.5) {
             std::cout << ">> BLOCKED: Central Bank Governor refuses to monetize debt. Autonomy is too high." << std::endl;
             std::cout << "   (Lower Autonomy first with 'autonomy-', but beware capital flight)" << std::endl;
         } else {
            // Emergency Funding: Seigniorage (1% of GDP)
            double print_amount = playerCountry.economy.gdp * 0.01;
            
            playerCountry.economy.monetary_emission += 0.01; // +1% Money Supply
            playerCountry.economy.international_reserves += print_amount;
            
            std::cout << "[!!!] DECREE: Central Bank ordered to print money." << std::endl;
            std::cout << "      Seigniorage Revenue: $" << print_amount / 1000000 << "M added to Reserves." << std::endl;
            std::cout << "      (Inflation Surge Incoming!)" << std::endl;
         }
    }
    else if (command == "reform_currency") {
        if (playerCountry.economy.inflation > 0.20) {
            std::cout << "[!!!] CURRENCY REFORM: The Government slashes zeros from the currency." << std::endl;
            std::cout << "      Savings wiped out. Austerity imposed. Inflation reset." << std::endl;
            
            playerCountry.economy.inflation = 0.02; // Reset to 2%
            playerCountry.economy.monetary_emission = 0.0;
            playerCountry.politics.popularity -= 0.25; // Massive anger
            playerCountry.welfare.poverty_rate += 0.05; // Short term pain
        } else {
             std::cout << ">> REJECTED: Inflation is not high enough for such a drastic measure." << std::endl;
        }
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

        // DIASPORA LOGIC
        // If people leave (net_migrants < 0), they join the Diaspora.
        // If people enter (net_migrants > 0), they are immigrants (not diaspora, but we could track them too).
        // For now, only outgoing adds to diaspora.
        if (net_migrants < 0) {
            playerCountry.welfare.diaspora_population += std::abs(net_migrants);
        }
        
        // Assimilation / Death in Diaspora
        // 2% of the diaspora stops sending money or dies annually.
        playerCountry.welfare.diaspora_population *= 0.98; 

        // Brain Drain Logic
        // If migration is negative AND education is high -> Lose Innovation.
        if (playerCountry.welfare.net_migration_rate < -0.005 && playerCountry.welfare.university_enrollment > 0.5) {
            playerCountry.infra.innovation_index -= 0.002; // Smart people leave first
            std::cout << "[!] BRAIN DRAIN: Talented youth are leaving the country." << std::endl;
        }
    }

    // 3. Minority Persecution -> Brain Drain
    if (playerCountry.welfare.minority_protection < 0.3) {
        playerCountry.welfare.brain_drain += 0.005; // Minorities (often educated/merchant class) leave
        std::cout << "[!] EXODUS: Persecuted minorities are fleeing the country." << std::endl;
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
    
    // EFFECT 2: Diversity Dividend (Innovation)
    // High minority protection fosters diverse perspectives.
    if (playerCountry.welfare.minority_protection > 0.8) {
        playerCountry.infra.innovation_index += 0.002; 
        // std::cout << "[INFO] DIVERSITY: Inclusive society boosting creativity." << std::endl;
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

    // --- DEEP ECONOMIC MODEL (GDP Calculation) ---
    // GDP = Consumption + Investment + Government + (Exports - Imports)
    // We simulate growth based on factors of production: Labor, Capital, TFP.

    // 1. Labor Factor (Workforce Quality & Quantity)
    // Quantity: 
    double workforce_participation = (1.0 - playerCountry.welfare.unemployment_rate) 
                                   * (1.0 - (playerCountry.welfare.aging_index * 0.5)); // Age reduces active workforce
    
    // Quality (Human Capital):
    double human_capital = (playerCountry.welfare.literacy_rate * 0.4) 
                         + (playerCountry.welfare.secondary_enrollment * 0.3)
                         + (playerCountry.welfare.health_coverage * 0.3); // Healthy workers work better
                         
    double labor_factor = workforce_participation * human_capital;

    // 2. Capital Factor (Infrastructure & Machines)
    double physical_capital = (playerCountry.infra.road_connectivity * 0.3)
                            + (playerCountry.infra.port_capacity * 0.2)
                            + (playerCountry.politics.industrial_power * 0.3)
                            + (playerCountry.politics.financial_power * 0.2);

    // 3. TFP (Total Factor Productivity - Innovation/Efficiency)
    double tfp = 1.0 + (playerCountry.infra.innovation_index * 0.05) // Tech bonus
                     + (playerCountry.politics.tech_power * 0.05)
                     - (playerCountry.politics.administrative_corruption * 0.10); // Corruption is inefficiency
                     
                     
    // 4. Consumption Engine (Demand Side)
    // Spending Power: Wage / GDP per Capita
    // Ideal: Wage is ~40-50% of GDP/Capita.
    
    // REMITTANCES EFFECT:
    // Remittances act as a direct cash injection to households, effectively boosting purchasing power.
    // Calculate Remittances first:
    // Avg sending: $2000 per diaspora member annually (Standard assumption)
    double remittance_income = playerCountry.welfare.diaspora_population * 2000.0;
    
    // Sanctions block money transfer!
    if (playerCountry.economy.international_sanctions_prob > 0.5) { // If blockade is active
         remittance_income *= 0.1; // 90% blocked
         std::cout << "[!] BLOCKADE: Remittances cannot reach the country." << std::endl;
    }
    
    playerCountry.economy.remittances = remittance_income;
    
    // Total Household Income for Consumption = Wages (simplified) + Remittances
    // We treat Remittances as a % boost to the Wage purchasing power logic.
    double gdp_per_capita_prev = playerCountry.economy.gdp / playerCountry.welfare.population;
    
    // Remittance per capita involved in consumption
    double rem_per_capita = remittance_income / playerCountry.welfare.population;
    
    // FISCAL DRAG (Laffer Curve / Disposable Income)
    // Taxes reduce the money people actually have to spend.
    // Calculate Effective Tax Rate
    double effective_tax_rate = playerCountry.economy.tax_collection / playerCountry.economy.gdp;
    
    // Disposable Income = (Wage + Remittances) * (1 - TaxRate)
    // We apply tax rate to the purchasing power calculation.
    // However, poor people pay less taxes (progressive), but let's assume flat VAT/Income blend for simplicity or average burden.
    
    double gross_purchasing_power = (playerCountry.welfare.minimum_wage + rem_per_capita) / (gdp_per_capita_prev * 0.4); 
    double net_purchasing_power = gross_purchasing_power * (1.0 - effective_tax_rate);
    
    // If purchasing power is low (< 0.8), consumption drags growth.
    // If high (> 1.2), it overheats (handled in inflation already), captures demand here.
    double consumption_modifier = 0.0;
    if (net_purchasing_power < 0.8) consumption_modifier = -0.01; // Low demand
    if (net_purchasing_power > 1.0) consumption_modifier = 0.01; // High demand
    
    // CALCULATE ORGANIC GROWTH RATE
    
    // --- GLOBAL BUSINESS CYCLE & TRADE ---
    // The world economy fluctuates in a sine wave (Booms and Recessions).
    // turnCount drives the cycle. Period = ~12 years (0.5 multiplier).
    double global_growth_trend = 0.03 + (0.02 * sin(turnCount * 0.5));
    // Add noise
    std::uniform_real_distribution<> noise_dist(-0.005, 0.005);
    global_growth_trend += noise_dist(rng);
    
    // Trade Exposure (Export Orientation)
    // High Tech + High Industry = Export Economy.
    double export_exposure = (playerCountry.politics.industrial_power + playerCountry.politics.tech_power) / 2.0;
    
    // Internal Market Strength (Consumption Base)
    // Large population with money shields from global shocks.
    // Normalized somewhat to 10M pop baseline.
    double internal_market_strength = (playerCountry.welfare.population / 10000000.0) * consumption_modifier; 
    
    // Display Global Context
    if (global_growth_trend > 0.04) std::cout << "[INFO] GLOBAL ECONOMY: Boom Times! World trade is surging." << std::endl;
    else if (global_growth_trend < 0.015) std::cout << "[INFO] GLOBAL ECONOMY: Recession. World demand is weak." << std::endl;
    
    // Base potential growth
    // Starts with Global Trend, modified by exposure.
    // If you are closed economy (exposure 0), you get a flat 0.02 base.
    // If you are open (exposure 1), you get the full global volatile rate.
    
    double growth_from_trade = (global_growth_trend - 0.02) * export_exposure * 1.5; // Leveraged effect
    
    double potential_growth = 0.02 + growth_from_trade; 

    
    // Labor Contribution (Okun's Law approximate)
    // If unemployment > 5%, growth suffers.
    if (playerCountry.welfare.unemployment_rate > 0.05) {
        potential_growth -= (playerCountry.welfare.unemployment_rate - 0.05) * 0.5;
    }
    
    // Capital & TFP Bonus
    if (physical_capital > 0.7) potential_growth += 0.01;
    if (tfp > 1.0) potential_growth += (tfp - 1.0);
    
    // TAX DRAG on Investment (Laffer Curve Part 2)
    // If taxes > 25%, capital flight / lack of investment.
    if (effective_tax_rate > 0.25) {
        double tax_penalty = (effective_tax_rate - 0.25) * 0.1; // 10% penalty for every point over 25%
        potential_growth -= tax_penalty;
        // std::cout << "[INFO] HIGH TAXES: Investors are wary. Growth slowed." << std::endl;
    }

    // INTEREST RATE DRAG (Monetary Policy Cost)
    // High rates make borrowing for machines/tech expensive.
    if (playerCountry.economy.interest_rate > 0.08) {
        potential_growth -= (playerCountry.economy.interest_rate - 0.08) * 0.5;
    }
    
    // Apply Consumption
    potential_growth += consumption_modifier;
    
    // Update the tracked growth rate (for display/trends)
    playerCountry.economy.growth_rate = potential_growth;
    
    // Final Real Growth (adjusted by external shocks elsewhere like Sanctions)
    // Note: Sanctions are applied later as a multiplier on GDP directly, so here we set the "Trend".
    
    // Apply Growth to GDP
    playerCountry.economy.gdp += playerCountry.economy.gdp * playerCountry.economy.growth_rate;
    
    // --- FISCAL LINKAGE (Taxes) ---
    // Tax Collection is now derived from GDP.
    // Base Tax Rate: ~20% of GDP implied by initial values ($10M / $500M = 2%). Wait, initial is low ($10M tax on $500M GDP = 2%).
    // Let's assume a "Systemic Tax Rate" that the player modifies via tax+/tax-.
    // We'll calculate the *Effective Rate* from current values and maintain it unless changed.
    
    double current_effective_rate = playerCountry.economy.tax_collection / playerCountry.economy.gdp;
    // Bounds check to prevent weirdness after GDP jumps
    if (current_effective_rate < 0.01) current_effective_rate = 0.01;
    if (current_effective_rate > 0.50) current_effective_rate = 0.50;
    
    // Recalculate Collection based on new GDP
    playerCountry.economy.tax_collection = playerCountry.economy.gdp * current_effective_rate;

    // --- ENROLLMENT DYNAMICS (The Poverty Trap) ---

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
    
    // --- WAGE-PRICE SPIRAL (Automatic Indexation) ---
    // If Inflation is high, Unions demand catch-up raises.
    if (playerCountry.economy.inflation > 0.05 && playerCountry.welfare.union_strength > 0.30) {
        double forced_raise = playerCountry.welfare.minimum_wage * (playerCountry.economy.inflation * 0.8);
        playerCountry.welfare.minimum_wage += forced_raise;
        
        // This causes even MORE inflation next turn (Feedback Loop)
        playerCountry.economy.inflation += 0.01; 
        
        std::cout << "[!] INDEXATION: Unions forced a wage hike of $" << forced_raise << " to match inflation." << std::endl;
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
        std::uniform_int_distribution<> dist(0, 99);
        int roll = dist(rng);
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
    
    // Structural Poverty due to Discrimination
    // If protection is low, minorities are excluded from economy.
    if (playerCountry.welfare.minority_protection < 0.4) {
        playerCountry.welfare.poverty_rate += 0.005; 
    }
    
    // Remittances buffer (Poverty Reduction)
    // If Remittances are > 5% of GDP, they significantly reduce poverty.
    double rem_gdp_ratio = playerCountry.economy.remittances / playerCountry.economy.gdp;
    if (rem_gdp_ratio > 0.05) {
        playerCountry.welfare.poverty_rate -= 0.01; // Direct relief
        // std::cout << "[INFO] DIASPORA: Remittances are keeping families above water." << std::endl;
    } else if (rem_gdp_ratio > 0.01) {
        playerCountry.welfare.poverty_rate -= 0.002;
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
        std::uniform_real_distribution<> dist(0.0, 1.0);
        double roll = dist(rng);
        
        if (roll < net_risk) {
            std::cout << "[!!!] TERRORIST ATTACK: Radical extremists struck a major city!" << std::endl;
            
            // Consequences
            std::uniform_int_distribution<> casualties_dist(50, 549); // 50 + (0 to 499)
            int casualties = casualties_dist(rng); 
            playerCountry.welfare.population -= casualties;
            
            playerCountry.politics.popularity -= 0.05; // Leader blamed
            playerCountry.economy.gdp *= 0.995; // Economic shock (-0.5%)
            
            // Panic reaction: Demand for security
            playerCountry.politics.polarization_index += 0.02; // Fear divides
            
            std::cout << "      " << casualties << " casualties reported. Market panics." << std::endl;
        } else if (net_risk > 0.05) {
            // Near Miss (Intelligence Victory?)
            if (dist(rng) < 0.3) {
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

    // --- FISCAL BALANCE & AUTOMATIC MONETIZATION (Fiscal Dominance) ---
    // 1. Calculate Fiscal Balance
    // Revenue
    double govt_revenue = playerCountry.economy.tax_collection;
    
    // Spending (Simulated)
    // Governments tend to spend slightly more than they tax (Deficit Bias).
    // Plus Debt Service (Interest Payments).
    double effective_tax_rate_now = playerCountry.economy.tax_collection / playerCountry.economy.gdp;
    double base_spending = playerCountry.economy.gdp * (effective_tax_rate_now + 0.01); // 1% Structural Deficit
    double debt_service_cost = playerCountry.economy.gdp * playerCountry.economy.debt_to_gdp_ratio * playerCountry.economy.interest_rate;
    
    double total_spending = base_spending + debt_service_cost;
    double fiscal_balance = govt_revenue - total_spending;
    
    // 2. Fiscal Dominance Check
    if (fiscal_balance < 0) {
        // We have a DEFICIT
        double deficit = -fiscal_balance;
        
        if (playerCountry.economy.central_bank_autonomy < 0.3) {
            // LOW AUTONOMY: The Government forces the Bank to print money to cover the deficit.
            // This is "Monetization of the Deficit".
            
            double monetization_pct = deficit / playerCountry.economy.gdp;
            
            playerCountry.economy.monetary_emission += monetization_pct;
            // The printed money theoretically enters reserves/economy
            playerCountry.economy.international_reserves += deficit;
            
            std::cout << "[!] FISCAL DOMINANCE: Deficit of $" << deficit/1000000 << "M covered by printing money." << std::endl;
            std::cout << "    (Autonomy too low to refuse. Inflation increasing by " << monetization_pct*100 << "%)" << std::endl;
            
        } else {
            // HIGH AUTONOMY: The Bank refuses. Govt must issue DEBT.
            playerCountry.economy.debt_to_gdp_ratio += (deficit / playerCountry.economy.gdp);
             // std::cout << "[INFO] DEFICIT FUNDED BY DEBT. (Bank is Autonomous)" << std::endl;
        }
    }

    // --- INFLATION ENGINE (Dynamic Model) ---
    // Inflation = Base + MonetaryPull + DemandPull + CostPush - PolicyCooling
    
    double base_inflation = 0.02; // 2% natural target
    
    // 1. Monetary Pull (Printing money)
    double monetary_pull = playerCountry.economy.monetary_emission * 0.2;
    
    // 2. Demand Pull (Overheated consumption)
    // net_purchasing_power is already adjusted by taxes and remittances in previous section
    double demand_pull = 0.0;
    // We need to re-fetch net_purchasing_power from the scope (it's in Game::update)
    // If net_purchasing_power > 1.1, add (net_purchasing_power - 1.1) * 0.1 to inflation.
    
    // 3. Cost Push (Devaluation / Resilience)
    double cost_push = 0.0;
    // If reserves are low, currency devalues
    if (playerCountry.economy.international_reserves < 20000000.0) {
        cost_push += 0.03; // Devaluation inflation
    }
    
    // 4. Policy Cooling (Central Bank Interest Rates)
    // High interest rates reduce inflation.
    double policy_cooling = playerCountry.economy.interest_rate * 0.5;
    
    // Target Inflation Calculation
    // We'll use the local net_purchasing_power variable from earlier in the function
    double target_inflation = base_inflation + monetary_pull + ( (net_purchasing_power > 1.1) ? (net_purchasing_power - 1.1) * 0.1 : 0.0 ) + cost_push - policy_cooling;
    if (target_inflation < -0.02) target_inflation = -0.02; // Deflation floor
    
    // Drift actual inflation towards Target
    playerCountry.economy.inflation += (target_inflation - playerCountry.economy.inflation) * 0.3;
    
    // Monetary Emission Decays (The one-time printing shock fades)
    playerCountry.economy.monetary_emission *= 0.5;
    
    // --- BALANCE OF PAYMENTS (Reserves Logic) ---
    // 1. Trade Balance (Current Account)
    // Exports depend on Tech/Industry + Global Growth + Exchange Rate (Cost Push)
    // Imports depend on Consumption Power + Import Dependency
    
    double exports = playerCountry.economy.gdp * 0.20 * export_exposure * (1.0 + (global_growth_trend - 0.02) * 2.0);
    double imports = playerCountry.economy.gdp * playerCountry.economy.import_dependency * (net_purchasing_power > 0.8 ? net_purchasing_power : 0.8);
    
    playerCountry.economy.trade_balance = exports - imports;
    
    // 2. Financial Account (FDI and Capital Flow)
    // Determinants: Interest Rate (Carry Trade) + Stability (UN Score/Polarization)
    
    double capital_flow_base = 0.0;
    
    // Carry Trade: High Interest attracts Hot Money
    if (playerCountry.economy.interest_rate > 0.05) {
        capital_flow_base += (playerCountry.economy.interest_rate - 0.05) * 1000000000.0; // $1B per 1% spread
    }
    
    // Stability Factor
    if (playerCountry.welfare.un_score < 0.3 || playerCountry.politics.polarization_index > 0.7) {
        capital_flow_base -= 500000000.0; // Capital Flight ($500M)
        std::cout << "[!] CAPITAL FLIGHT: Investors are fleeing instability." << std::endl;
    }
    
    
    // FDI (Long term)
    // Autonomy is a Multiplier for TRUST.
    double fdi = (playerCountry.welfare.un_score * 0.02) * playerCountry.economy.gdp * (0.5 + playerCountry.economy.central_bank_autonomy); 
    
    // 3. Debt Service
    // We pay interest on external debt.
    double external_debt_interest = playerCountry.economy.gdp * playerCountry.economy.debt_to_gdp_ratio * 0.05; // 5% avg interest
    
    // Net Change in Reserves
    double reserves_change = playerCountry.economy.trade_balance + fdi + capital_flow_base - external_debt_interest;
    
    playerCountry.economy.international_reserves += reserves_change;
    
    // Display BoP
    // std::cout << "[ECON] BoP: Trade $" << playerCountry.economy.trade_balance / 1000000 << "M | Capital $" << (fdi+capital_flow_base)/1000000 << "M | DebtServ -$" << external_debt_interest/1000000 << "M" << std::endl;
    std::cout << "[ECON] Reserves Change: " << (reserves_change >= 0 ? "+" : "") << reserves_change / 1000000.0 << "M USD. Total: $" << playerCountry.economy.international_reserves / 1000000.0 << "M" << std::endl;
    
    // --- CURRENCY CRISIS (Reserves < 0) ---
    if (playerCountry.economy.international_reserves < 0.0) {
        std::cout << "[!!!] CURRENCY CRISIS: The Central Bank has run out of dollars." << std::endl;
        
        // Massive Devaluation (Mitigated by Stability)
        // Base shock: 15% inflation increase.
        // Stability Factor reduces it. Max stability (0.9) -> Shock ~1.5%.
        double devaluation_shock = 0.15 * (1.0 - playerCountry.economy.exchange_rate_stability);
        if (devaluation_shock < 0.02) devaluation_shock = 0.02; // Minimum shock
        
        playerCountry.economy.inflation += devaluation_shock; 
        playerCountry.economy.gdp *= 0.95;       // -5% GDP Shock
        playerCountry.politics.popularity -= 0.10; // Anger
        playerCountry.economy.international_reserves = 0.0; // IMF Bailout (reset to zero, debt increases)
        playerCountry.economy.debt_to_gdp_ratio += 0.05; // Forced borrowing
        
        std::cout << "      Devaluation Shock: +" << devaluation_shock * 100 << "% Inflation." << std::endl;
        if (playerCountry.economy.central_bank_autonomy > 0.7) {
            std::cout << "      (Autonomous Bank mitigated the panic)." << std::endl;
        } else {
             std::cout << "      (Politicized Bank caused a run on the currency)." << std::endl;
        }
    }

    // --- HYPERINFLATION DISASTER ---
    if (playerCountry.economy.inflation > 0.50) { // 50% Inflation
         std::cout << "[!!!] HYPERINFLATION: The currency has collapsed. Economy is in freefall." << std::endl;
         playerCountry.economy.gdp *= 0.90; // -10% GDP per year
         playerCountry.politics.polarization_index += 0.10; // Chaos
         playerCountry.economy.international_reserves *= 0.5; // Capital Flight
         playerCountry.welfare.poverty_rate += 0.05;
    }

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
    if (playerCountry.politics.popularity < 0.0) playerCountry.politics.popularity = 0.0;

    // --- INTERNATIONAL RELATIONS (Sanctions & Investment) ---
    // 1. Sanctions Check
    // Risk depends on UN Score. If Score < 0.3, risk spikes.
    if (playerCountry.welfare.un_score < 0.3) {
        playerCountry.economy.international_sanctions_prob += 0.05;
    } else {
        playerCountry.economy.international_sanctions_prob -= 0.01; // Decays if behaving
    }
    // Clamp
    if (playerCountry.economy.international_sanctions_prob > 1.0) playerCountry.economy.international_sanctions_prob = 1.0;
    if (playerCountry.economy.international_sanctions_prob < 0.0) playerCountry.economy.international_sanctions_prob = 0.0;
    
    // Trigger Sanctions
    std::uniform_real_distribution<> dist(0.0, 1.0);
    double sanctions_roll = dist(rng);
    if (sanctions_roll < playerCountry.economy.international_sanctions_prob) {
        std::cout << "[!!!] SANCTIONS: The International Community has imposed a blockade." << std::endl;
        std::cout << "      (GDP -5%, Inflation +2%)" << std::endl;
        
        playerCountry.economy.gdp *= 0.95; 
        playerCountry.economy.inflation += 0.02; 
    }
    
    // 2. Foreign Investment
    // If UN Score > 0.7 (Stable, respected), capital flows in.
    if (playerCountry.welfare.un_score > 0.7) {
        double investment_boost = (playerCountry.welfare.un_score - 0.7) * 0.05; // Max 1.5% boost
        playerCountry.economy.growth_rate += investment_boost;
        std::cout << "[INFO] FOREIGN INVESTMENT: Global confidence boosts growth by " << investment_boost * 100 << "%" << std::endl;
    }

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
