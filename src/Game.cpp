#include "Game.hpp"
#include "Persistence.hpp"
#include "GameOverChecker.hpp"
#include "ScenarioLoader.hpp"
#include "LeaderLoader.hpp"
#include "Advisor.hpp"
#include "Localization.hpp"
#include <iostream>
#include <thread> // For sleep
#include <chrono> // For time duration
#include <algorithm>
#include <unordered_map>

Game::Game() : isRunning(true), nextTurn(false), turnCount(0), rng(std::random_device{}()) {
    std::cout << "Initializing Game..." << std::endl;
    initialGdp = playerCountry.economy.gdp;
    scriptedEvents = EventLoader::builtin();
    registerCommands();
    std::cout << "Comandos del tutorial: 'tutorial_start' / 'tutorial_skip' / 'tutorial_reset'" << std::endl;
}

void Game::registerCommands() {
    commandHandlers["exit"]         = [this]() { isRunning = false; };
    commandHandlers["next"]         = [this]() { nextTurn = true; };
    commandHandlers["save"]         = [this]() { saveGame("savegame.txt"); };
    commandHandlers["load"]         = [this]() { loadGame("savegame.txt"); };
    commandHandlers["save_slot"]    = [this]() {
        std::string slot;
        std::cin >> slot;
        if (slot.empty()) { std::cout << ">> Usage: save_slot <1-5|quick>" << std::endl; return; }
        saveGame("savegame_" + slot + ".txt");
    };
    commandHandlers["load_slot"]    = [this]() {
        std::string slot;
        std::cin >> slot;
        if (slot.empty()) { std::cout << ">> Usage: load_slot <1-5|quick>" << std::endl; return; }
        loadGame("savegame_" + slot + ".txt");
    };
    commandHandlers["tax+"]         = [this]() {
        double current_tax_level = playerCountry.economy.tax_collection / (playerCountry.economy.gdp * 0.30);
        if (current_tax_level > 1.0) current_tax_level = 1.0;
        double laffer = 1.0 + 0.8 * current_tax_level - 0.6 * current_tax_level * current_tax_level;
        double revenue_multiplier = 1.0 + 0.10 * (laffer - 1.0);
        if (revenue_multiplier < 1.0) revenue_multiplier = 1.0;
        double pre_rev = playerCountry.economy.tax_collection;
        double pre_pop = playerCountry.politics.popularity;
        playerCountry.economy.tax_collection *= revenue_multiplier;
        playerCountry.politics.popularity -= 0.05;
        playerCountry.economy.inflation += 0.01;
        playerCountry.politics.economic_ideology -= 0.02;
        if (playerCountry.politics.economic_ideology < 0.0) playerCountry.politics.economic_ideology = 0.0;
        std::cout << ">> SUBES IMPUESTOS (Laffer x=" << (int)(current_tax_level*100) << "%)" << std::endl;
        std::cout << "   Recaudacion: $" << (long long)(pre_rev/1e6) << "M -> $" << (long long)(playerCountry.economy.tax_collection/1e6) << "M (+" << (int)((revenue_multiplier-1.0)*100) << "%)" << std::endl;
        std::cout << "   Popularidad: " << (int)(pre_pop*100) << "% -> " << (int)(playerCountry.politics.popularity*100) << "%" << std::endl;
        std::cout << "   Inflacion:   +1pt" << std::endl;
        if (current_tax_level > 0.667) std::cout << "   [!] Estas pasado el optimo Laffer: cada suba rinde menos." << std::endl;
    };
    commandHandlers["tax-"]         = [this]() {
        playerCountry.economy.tax_collection *= 0.90;
        playerCountry.politics.popularity += 0.03;
        playerCountry.economy.inflation -= 0.005;
        playerCountry.politics.economic_ideology += 0.02;
        if (playerCountry.politics.economic_ideology > 1.0) playerCountry.politics.economic_ideology = 1.0;
        std::cout << ">> Taxes CUT! Popularity up, revenue down." << std::endl;
    };
    commandHandlers["decisions"] = [this]() {
        if (pendingDecisions.empty()) { std::cout << ">> No hay decisiones pendientes." << std::endl; return; }
        std::cout << "\n=== DECISIONES PENDIENTES (" << pendingDecisions.size() << ") ===" << std::endl;
        for (size_t i = 0; i < pendingDecisions.size(); ++i) {
            std::cout << "[" << i << "] " << pendingDecisions[i].id
                      << " — " << pendingDecisions[i].prompt << std::endl;
        }
        std::cout << "=================================" << std::endl;
    };
    commandHandlers["tutorial_start"] = [this]() { tutorial.start(); };
    commandHandlers["tutorial_skip"]  = [this]() { tutorial.skip(); };
    commandHandlers["tutorial_reset"] = [this]() { tutorial.reset(); };
    commandHandlers["language"] = []() {
        std::string lang;
        std::cin >> lang;
        if (lang.empty()) {
            std::cout << ">> Usage: language <es|en>" << std::endl;
            std::cout << "   Actual: " << Localization::currentLanguage() << std::endl;
            return;
        }
        std::string dir = "/Users/michelcano/Documents/Repositorios/homo-politicus-game/content/locales";
        if (Localization::load(dir, lang)) {
            std::cout << ">> Idioma cambiado a: " << lang << std::endl;
            std::cout << "   " << Localization::tr("ui.welcome") << std::endl;
        } else {
            std::cout << ">> LANGUAGE: no se pudo cargar " << dir << "/" << lang << ".yaml" << std::endl;
        }
    };
    commandHandlers["events_load"] = [this]() {
        std::string dir;
        std::cin >> dir;
        if (dir.empty()) {
            std::cout << ">> Usage: events_load <directory>" << std::endl;
            return;
        }
        auto loaded = EventLoader::loadDir(dir);
        if (loaded.empty()) {
            std::cout << ">> EVENTS_LOAD: no se encontraron eventos en " << dir << std::endl;
            return;
        }
        scriptedEvents = loaded;
        std::cout << ">> EVENTS_LOAD: " << loaded.size() << " eventos cargados desde " << dir << std::endl;
    };
    commandHandlers["events_list"] = [this]() {
        std::cout << "\n=== EVENTOS CARGADOS (" << scriptedEvents.size() << ") ===" << std::endl;
        for (const auto& e : scriptedEvents) {
            std::cout << "  " << e.id << " - " << e.name_es
                      << " (prob " << e.prob_per_turn << ", cd " << e.cooldown_turns << ")" << std::endl;
        }
        std::cout << "==============================" << std::endl;
    };
    commandHandlers["advisors"] = []() {
        std::cout << "\n=== ASESORES DEL GABINETE ===" << std::endl;
        for (const auto& a : Advisors::all())
            std::cout << "  " << a->id() << " - " << a->name_es() << std::endl;
        std::cout << "Uso: ask <id>" << std::endl;
    };
    commandHandlers["ask"] = [this]() {
        std::string id;
        std::cin >> id;
        Advisor* a = Advisors::findById(id);
        if (!a) {
            std::cout << ">> Advisor '" << id << "' no existe. Tipea 'advisors' para listar." << std::endl;
        } else {
            std::cout << "\n[" << a->name_es() << "]" << std::endl;
            std::cout << a->respond(playerCountry, "") << std::endl;
        }
    };
    commandHandlers["wage-"] = [this]() {
        playerCountry.welfare.minimum_wage *= 0.90;
        std::cout << ">> DECREE: Cutting Minimum Wage to $" << playerCountry.welfare.minimum_wage << std::endl;
        std::cout << "   (Competitiveness +, Poverty ++, Popularity --)" << std::endl;
        playerCountry.politics.popularity -= 0.06;
    };
    commandHandlers["retire+"] = [this]() {
        playerCountry.welfare.retirement_age += 1.0;
        std::cout << ">> REFORM: Raising Retirement Age to " << playerCountry.welfare.retirement_age << " years." << std::endl;
        std::cout << "   (Pension Sustainability ++, Popularity ---)" << std::endl;
        playerCountry.politics.popularity -= 0.05;
        std::uniform_int_distribution<> dist(0, 99);
        if (dist(rng) < 30) {
            std::cout << "[!] PROTESTS: Seniors take the streets!" << std::endl;
            playerCountry.welfare.general_strike_prob += 0.05;
        }
    };
    commandHandlers["retire-"] = [this]() {
        playerCountry.welfare.retirement_age -= 1.0;
        std::cout << ">> REFORM: Lowering Retirement Age to " << playerCountry.welfare.retirement_age << " years." << std::endl;
        std::cout << "   (Popularity ++, Pension Sustainability --)" << std::endl;
        playerCountry.politics.popularity += 0.03;
    };
    commandHandlers["torture+"] = [this]() {
        playerCountry.welfare.torture_index += 0.1;
        if (playerCountry.welfare.torture_index > 1.0) playerCountry.welfare.torture_index = 1.0;
        std::cout << ">> SECRET ORDER: Enhanced Interrogation techniques authorized." << std::endl;
        playerCountry.security.attack_detection_prob += 0.15;
        if (playerCountry.security.attack_detection_prob > 1.0) playerCountry.security.attack_detection_prob = 1.0;
        playerCountry.welfare.radicalism_prob += 0.05;
        playerCountry.welfare.un_score -= 0.15;
        if (playerCountry.welfare.un_score < 0.0) playerCountry.welfare.un_score = 0.0;
    };
    commandHandlers["torture-"] = [this]() {
        playerCountry.welfare.torture_index -= 0.1;
        if (playerCountry.welfare.torture_index < 0.0) playerCountry.welfare.torture_index = 0.0;
        std::cout << ">> REFORM: Banning torture and closing secret prisons." << std::endl;
        playerCountry.security.attack_detection_prob -= 0.10;
        if (playerCountry.security.attack_detection_prob < 0.0) playerCountry.security.attack_detection_prob = 0.0;
        playerCountry.welfare.radicalism_prob -= 0.03;
        playerCountry.welfare.un_score += 0.10;
        if (playerCountry.welfare.un_score > 1.0) playerCountry.welfare.un_score = 1.0;
    };
    commandHandlers["disappear+"] = [this]() {
        playerCountry.welfare.forced_disappearances += 0.2;
        if (playerCountry.welfare.forced_disappearances > 1.0) playerCountry.welfare.forced_disappearances = 1.0;
        std::cout << ">> SECRET OPERATION: Night and Fog decree signed." << std::endl;
        playerCountry.politics.mobilizations = 0;
        playerCountry.welfare.general_strike_prob *= 0.5;
        playerCountry.welfare.radicalism_prob += 0.08;
        playerCountry.welfare.un_score -= 0.20;
        if (playerCountry.welfare.un_score < 0.0) playerCountry.welfare.un_score = 0.0;
        playerCountry.politics.popularity -= 0.10;
    };
    commandHandlers["disappear-"] = [this]() {
        playerCountry.welfare.forced_disappearances -= 0.2;
        if (playerCountry.welfare.forced_disappearances < 0.0) playerCountry.welfare.forced_disappearances = 0.0;
        std::cout << ">> JUSTICE: Truth Commission established." << std::endl;
        playerCountry.politics.mobilizations += 2;
        playerCountry.welfare.radicalism_prob -= 0.05;
        if (playerCountry.welfare.radicalism_prob < 0.0) playerCountry.welfare.radicalism_prob = 0.0;
        playerCountry.welfare.un_score += 0.15;
        if (playerCountry.welfare.un_score > 1.0) playerCountry.welfare.un_score = 1.0;
        playerCountry.politics.popularity += 0.05;
    };
    commandHandlers["press+"] = [this]() {
        playerCountry.welfare.freedom_of_expression += 0.1;
        if (playerCountry.welfare.freedom_of_expression > 1.0) playerCountry.welfare.freedom_of_expression = 1.0;
        std::cout << ">> POLICY: Censorship lifted. Press is free." << std::endl;
        playerCountry.infra.innovation_index += 0.05;
        playerCountry.politics.administrative_corruption -= 0.05;
        if (playerCountry.politics.administrative_corruption < 0.0) playerCountry.politics.administrative_corruption = 0.0;
        std::uniform_real_distribution<> dist(0.0, 1.0);
        if (dist(rng) < 0.4) {
            std::cout << "[!] SCANDAL: Free press exposes government corruption!" << std::endl;
            playerCountry.politics.popularity -= 0.08;
            playerCountry.politics.polarization_index += 0.03;
        }
    };
    commandHandlers["press-"] = [this]() {
        playerCountry.welfare.freedom_of_expression -= 0.1;
        if (playerCountry.welfare.freedom_of_expression < 0.0) playerCountry.welfare.freedom_of_expression = 0.0;
        std::cout << ">> POLICY: Media access restricted." << std::endl;
        playerCountry.politics.popularity += 0.02;
        playerCountry.infra.innovation_index -= 0.08;
        if (playerCountry.infra.innovation_index < 0.0) playerCountry.infra.innovation_index = 0.0;
        playerCountry.politics.administrative_corruption += 0.05;
        playerCountry.welfare.radicalism_prob += 0.02;
    };
    commandHandlers["worship+"] = [this]() {
        playerCountry.welfare.freedom_of_worship += 0.1;
        if (playerCountry.welfare.freedom_of_worship > 1.0) playerCountry.welfare.freedom_of_worship = 1.0;
        std::cout << ">> POLICY: Religious Freedom increased to " << playerCountry.welfare.freedom_of_worship << std::endl;
        playerCountry.welfare.clerical_political_influence -= 0.05;
        if (playerCountry.welfare.clerical_political_influence < 0.0) playerCountry.welfare.clerical_political_influence = 0.0;
        playerCountry.welfare.interreligious_tension += 0.05;
    };
    commandHandlers["worship-"] = [this]() {
        playerCountry.welfare.freedom_of_worship -= 0.1;
        if (playerCountry.welfare.freedom_of_worship < 0.0) playerCountry.welfare.freedom_of_worship = 0.0;
        std::cout << ">> POLICY: Religious Freedom decreased to " << playerCountry.welfare.freedom_of_worship << std::endl;
        playerCountry.welfare.clerical_political_influence += 0.05;
        playerCountry.welfare.radicalism_prob += 0.02;
    };
    commandHandlers["autonomy+"] = [this]() {
        playerCountry.economy.central_bank_autonomy += 0.05;
        if (playerCountry.economy.central_bank_autonomy > 1.0) playerCountry.economy.central_bank_autonomy = 1.0;
        std::cout << ">> MONETARY: Central Bank Autonomy raised to " << playerCountry.economy.central_bank_autonomy << std::endl;
    };
    commandHandlers["autonomy-"] = [this]() {
        playerCountry.economy.central_bank_autonomy -= 0.05;
        if (playerCountry.economy.central_bank_autonomy < 0.0) playerCountry.economy.central_bank_autonomy = 0.0;
        std::cout << ">> MONETARY: Central Bank Autonomy reduced to " << playerCountry.economy.central_bank_autonomy << std::endl;
        std::cout << "   (Capital flight risk +, inflation risk +)" << std::endl;
    };
    commandHandlers["interest+"] = [this]() {
        playerCountry.economy.interest_rate += 0.01;
        std::cout << ">> MONETARY: Interest Rate raised. Inflation tamed, growth slowed." << std::endl;
    };
    commandHandlers["interest-"] = [this]() {
        playerCountry.economy.interest_rate -= 0.01;
        if (playerCountry.economy.interest_rate < 0.0) playerCountry.economy.interest_rate = 0.0;
        std::cout << ">> MONETARY: Interest Rate lowered. Borrowing is cheap!" << std::endl;
    };
    commandHandlers["minority+"] = [this]() {
        playerCountry.welfare.minority_protection += 0.1;
        if (playerCountry.welfare.minority_protection > 1.0) playerCountry.welfare.minority_protection = 1.0;
        playerCountry.welfare.un_score += 0.05;
        if (playerCountry.welfare.un_score > 1.0) playerCountry.welfare.un_score = 1.0;
        std::cout << ">> RIGHTS: Minority Protection increased to " << playerCountry.welfare.minority_protection << std::endl;
    };
    commandHandlers["minority-"] = [this]() {
        playerCountry.welfare.minority_protection -= 0.1;
        if (playerCountry.welfare.minority_protection < 0.0) playerCountry.welfare.minority_protection = 0.0;
        playerCountry.welfare.un_score -= 0.05;
        if (playerCountry.welfare.un_score < 0.0) playerCountry.welfare.un_score = 0.0;
        playerCountry.welfare.interreligious_tension += 0.05;
        std::cout << ">> RIGHTS: Minority Protection reduced to " << playerCountry.welfare.minority_protection << std::endl;
    };
    commandHandlers["diplomacy+"] = [this]() {
        playerCountry.security.diplomatic_prestige += 0.05;
        if (playerCountry.security.diplomatic_prestige > 1.0) playerCountry.security.diplomatic_prestige = 1.0;
        std::cout << ">> DIPLOMACY: Prestige raised to " << playerCountry.security.diplomatic_prestige << std::endl;
    };
    commandHandlers["diplomacy-"] = [this]() {
        playerCountry.security.diplomatic_prestige -= 0.05;
        if (playerCountry.security.diplomatic_prestige < 0.0) playerCountry.security.diplomatic_prestige = 0.0;
        std::cout << ">> DIPLOMACY: Prestige reduced to " << playerCountry.security.diplomatic_prestige << std::endl;
    };
    commandHandlers["achievements"] = [this]() {
        auto list = achievements.unlockedList();
        if (list.empty()) {
            std::cout << ">> Sin logros desbloqueados aun. Catalogo: " << AchievementTracker::catalog().size() << " logros disponibles." << std::endl;
            return;
        }
        std::cout << "\n=== LOGROS DESBLOQUEADOS (" << list.size() << "/" << AchievementTracker::catalog().size() << ") ===" << std::endl;
        for (const auto& def : AchievementTracker::catalog()) {
            if (achievements.isUnlocked(def.id))
                std::cout << "  [X] " << def.id << " - " << def.name_es << std::endl;
        }
        std::cout << "============================================" << std::endl;
    };
    commandHandlers["skip_decision"] = [this]() {
        if (pendingDecisions.empty()) { std::cout << ">> No hay decisión que saltar." << std::endl; return; }
        PendingDecision d = pendingDecisions.front();
        pendingDecisions.erase(pendingDecisions.begin());
        pendingDecisions.push_back(d);
        playerCountry.politics.popular_pressure += 0.02;
        std::cout << ">> Decisión '" << d.id << "' pospuesta (-credibilidad)." << std::endl;
    };
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
    if (!pendingDecisions.empty()) {
        const auto& d = pendingDecisions.front();
        std::cout << "\n>> Pending decision: " << d.prompt << std::endl;
        std::cout << "Choose (or 'skip_decision' / 'decisions' / 'status_brief'): ";
        for (size_t i = 0; i < d.options.size(); ++i) {
            std::cout << d.options[i];
            if (i + 1 < d.options.size()) std::cout << " | ";
        }
        std::cout << "\n> ";
        std::string choice;
        std::cin >> choice;
        if (choice == "skip_decision" || choice == "decisions" || choice == "status_brief"
            || choice == "help" || choice == "save" || choice == "exit") {
            if (choice == "exit") { isRunning = false; return; }
            std::string command = choice;
            // fallthrough to command handler below
            // duplicate the small subset here to avoid reentering
            if (command == "skip_decision") {
                PendingDecision d2 = pendingDecisions.front();
                pendingDecisions.erase(pendingDecisions.begin());
                pendingDecisions.push_back(d2);
                playerCountry.politics.popular_pressure += 0.02;
                std::cout << ">> Decisión '" << d2.id << "' pospuesta (-credibilidad)." << std::endl;
            } else if (command == "decisions") {
                std::cout << "\n=== DECISIONES PENDIENTES (" << pendingDecisions.size() << ") ===" << std::endl;
                for (size_t i = 0; i < pendingDecisions.size(); ++i) {
                    std::cout << "[" << i << "] " << pendingDecisions[i].id
                              << " — " << pendingDecisions[i].prompt << std::endl;
                }
                std::cout << "=================================" << std::endl;
            } else if (command == "save") {
                saveGame("savegame.txt");
            } else if (command == "help") {
                std::cout << ">> Type the option text shown, or 'skip_decision' to defer." << std::endl;
            } else if (command == "status_brief") {
                auto& pol = playerCountry.politics;
                auto& eco = playerCountry.economy;
                std::cout << "Pop: " << (int)(pol.popularity * 100) << "% | GDP: $"
                          << (long long)(eco.gdp / 1000000) << "M | Pendientes: "
                          << pendingDecisions.size() << std::endl;
            }
            return;
        }
        resolveDecision(choice);
        return;
    }
    std::cout << "\nCommand (next/exit/help/status_brief/decisions/...): ";
    std::string command;
    std::cin >> command;

    auto it = commandHandlers.find(command);
    if (it != commandHandlers.end()) {
        it->second();
        return;
    }

    if (command == "wage-") {
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
    // --- MINING COMMANDS ---
    else if (command == "mining+") {
        if (playerCountry.economy.resource_depletion > 0.9) {
            std::cout << ">> DENIED: Geological surveys show reserves nearly exhausted. No viable new concessions." << std::endl;
        } else {
            playerCountry.economy.mining_concessions += 1;
            playerCountry.economy.community_conflicts += 0.05;
            if (playerCountry.economy.community_conflicts > 1.0) playerCountry.economy.community_conflicts = 1.0;
            playerCountry.infra.co2_emissions         += 100.0;
            playerCountry.welfare.un_score            -= 0.02;
            playerCountry.politics.popularity         += 0.02; // Job creation
            playerCountry.politics.industrial_power   += 0.03; // Industry lobby satisfied
            std::cout << ">> CONCESSION GRANTED: Mining operation #" << playerCountry.economy.mining_concessions << " authorized." << std::endl;
            std::cout << "   (Royalties +, Jobs +, Pollution +, UN -, Community Conflict +)" << std::endl;
        }
    }
    else if (command == "mining-") {
        if (playerCountry.economy.mining_concessions <= 0) {
            std::cout << ">> No active mining concessions to revoke." << std::endl;
        } else {
            playerCountry.economy.mining_concessions -= 1;
            playerCountry.economy.community_conflicts -= 0.08;
            if (playerCountry.economy.community_conflicts < 0.0) playerCountry.economy.community_conflicts = 0.0;
            playerCountry.politics.popularity  -= 0.02; // Job losses
            playerCountry.welfare.un_score     += 0.03; // Environmental praise
            playerCountry.politics.industrial_power -= 0.03;
            std::cout << ">> CONCESSION REVOKED: Mining operation shut down." << std::endl;
            std::cout << "   (Royalties -, Jobs -, Pollution -, UN +, Community Conflict -)" << std::endl;
        }
    }
    else if (command == "mining_reform") {
        if (playerCountry.economy.mining_concessions <= 0) {
            std::cout << ">> No active mining operations to reform." << std::endl;
        } else {
            // Revenue-sharing fund: 20% of royalties redirected to affected communities.
            double cost = playerCountry.economy.state_royalties * 0.20;
            playerCountry.economy.tax_collection      -= cost;
            playerCountry.economy.community_conflicts -= 0.15;
            if (playerCountry.economy.community_conflicts < 0.0) playerCountry.economy.community_conflicts = 0.0;
            playerCountry.welfare.poverty_rate        -= 0.02;
            if (playerCountry.welfare.poverty_rate < 0.0) playerCountry.welfare.poverty_rate = 0.0;
            playerCountry.politics.popularity         += 0.03;
            playerCountry.welfare.un_score            += 0.04;
            playerCountry.politics.industrial_power   -= 0.05;
            std::cout << ">> REFORM: Revenue-sharing fund established for mining communities." << std::endl;
            std::cout << "   (Conflict --, Poverty -, Popularity +, UN +, Royalties -20%, Industry Lobby -)" << std::endl;
        }
    }
    else if (command == "royalty+") {
        if (playerCountry.economy.royalty_rate >= 0.50) {
            std::cout << ">> Royalty rate is already at maximum (50%)." << std::endl;
        } else {
            playerCountry.economy.royalty_rate        += 0.05;
            playerCountry.politics.industrial_power   -= 0.04; // Companies furious
            playerCountry.politics.popularity         += 0.02; // "Making them pay their share"
            playerCountry.welfare.un_score            += 0.01;
            // High rates also reduce conflict slightly (communities see a fairer deal)
            playerCountry.economy.community_conflicts -= 0.03;
            if (playerCountry.economy.community_conflicts < 0.0) playerCountry.economy.community_conflicts = 0.0;
            std::cout << ">> DECREE: Mining royalty rate raised to "
                      << playerCountry.economy.royalty_rate * 100 << "%" << std::endl;
            std::cout << "   (Revenue +, Industry Lobby -, FDI Risk, Popularity +, Conflict -)" << std::endl;
        }
    }
    else if (command == "royalty-") {
        if (playerCountry.economy.royalty_rate <= 0.05) {
            std::cout << ">> Royalty rate is already at minimum (5%)." << std::endl;
        } else {
            playerCountry.economy.royalty_rate        -= 0.05;
            playerCountry.politics.industrial_power   += 0.05; // Industry grateful
            playerCountry.politics.popularity         -= 0.02; // "Giving away our resources"
            // Lower rate → communities feel cheated
            playerCountry.economy.community_conflicts += 0.03;
            if (playerCountry.economy.community_conflicts > 1.0) playerCountry.economy.community_conflicts = 1.0;
            std::cout << ">> DECREE: Mining royalty rate cut to "
                      << playerCountry.economy.royalty_rate * 100 << "%" << std::endl;
            std::cout << "   (Revenue -, Industry Lobby +, FDI +, Popularity -, Conflict +)" << std::endl;
        }
    }
    // --- SOVEREIGN WEALTH FUND COMMANDS ---
    else if (command == "swf_save") {
        if (playerCountry.economy.mining_concessions <= 0) {
            std::cout << ">> No mining royalties to save. Activate concessions first." << std::endl;
        } else if (playerCountry.economy.swf_deposit_rate >= 0.5) {
            std::cout << ">> Already saving 50% of royalties. Maximum deposit rate reached." << std::endl;
        } else {
            playerCountry.economy.swf_deposit_rate += 0.10;
            playerCountry.politics.popularity -= 0.01;
            playerCountry.welfare.un_score    += 0.02;
            std::cout << ">> SWF: Saving " << playerCountry.economy.swf_deposit_rate * 100
                      << "% of annual royalties. Balance: $"
                      << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M" << std::endl;
            std::cout << "   (Future security +, Short-term revenue -, Fiscal resilience +)" << std::endl;
        }
    }
    else if (command == "swf_cancel") {
        if (playerCountry.economy.swf_deposit_rate <= 0.0) {
            std::cout << ">> No automatic SWF deposits active." << std::endl;
        } else if (playerCountry.economy.swf_rule_active) {
            // Constitutional rule blocks full cancellation — floor is 20%
            if (playerCountry.economy.swf_deposit_rate <= 0.20) {
                std::cout << ">> BLOCKED: Constitutional fiscal rule mandates minimum 20% deposit rate." << std::endl;
                std::cout << "   The rule cannot be suspended without legislative supermajority." << std::endl;
            } else {
                playerCountry.economy.swf_deposit_rate = 0.20; // Floor at constitutional minimum
                playerCountry.politics.popularity += 0.01;
                std::cout << ">> SWF: Deposits reduced to constitutional minimum of 20%." << std::endl;
                std::cout << "   Full cancellation blocked by fiscal rule. Balance: $"
                          << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M" << std::endl;
            }
        } else {
            playerCountry.economy.swf_deposit_rate = 0.0;
            playerCountry.politics.popularity += 0.01;
            std::cout << ">> SWF: Deposits suspended. Balance preserved: $"
                      << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M" << std::endl;
        }
    }
    else if (command == "swf_rate-") {
        double floor = playerCountry.economy.swf_rule_active ? 0.20 : 0.0;
        if (playerCountry.economy.swf_deposit_rate <= floor) {
            if (playerCountry.economy.swf_rule_active)
                std::cout << ">> BLOCKED: Constitutional rule sets 20% as the minimum deposit rate." << std::endl;
            else
                std::cout << ">> SWF deposit rate is already at 0%. Use swf_cancel to confirm." << std::endl;
        } else {
            playerCountry.economy.swf_deposit_rate -= 0.05;
            if (playerCountry.economy.swf_deposit_rate < floor) playerCountry.economy.swf_deposit_rate = floor;
            playerCountry.politics.popularity += 0.005; // Slight relief from fiscal pressure
            std::cout << ">> SWF: Deposit rate reduced to " << playerCountry.economy.swf_deposit_rate * 100
                      << "%. Balance: $" << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M" << std::endl;
        }
    }
    else if (command == "swf_rule") {
        if (playerCountry.economy.swf_rule_active) {
            std::cout << ">> Constitutional fiscal rule already in force." << std::endl;
            std::cout << "   Minimum deposit rate: 20% of royalties during commodity booms." << std::endl;
        } else if (playerCountry.economy.sovereign_wealth_fund < 100000000.0) {
            std::cout << ">> DENIED: Constitutional fiscal rule requires a seed fund of at least $100M." << std::endl;
            std::cout << "   Build the SWF first to demonstrate fiscal seriousness." << std::endl;
        } else if (playerCountry.politics.congressional_support < 0.55) {
            std::cout << ">> BLOCKED: Insufficient congressional support ("
                      << (int)(playerCountry.politics.congressional_support * 100) << "%)." << std::endl;
            std::cout << "   Constitutional amendment requires >55% supermajority in Congress." << std::endl;
        } else {
            playerCountry.economy.swf_rule_active = true;
            // Enforce minimum deposit rate immediately
            if (playerCountry.economy.swf_deposit_rate < 0.20)
                playerCountry.economy.swf_deposit_rate = 0.20;
            // Major credibility signal
            playerCountry.economy.debt_to_gdp_ratio     -= 0.015; // Market confidence
            playerCountry.economy.international_reserves += 20000000.0; // Capital inflows
            playerCountry.politics.administrative_corruption -= 0.04; // Rule-bound institutions
            playerCountry.welfare.un_score               += 0.06;
            playerCountry.politics.popularity            -= 0.04; // Short-term pain — future-locking
            playerCountry.politics.congressional_support -= 0.05; // Ruling party loses flexibility
            std::cout << ">> CONSTITUTIONAL FISCAL RULE ENACTED." << std::endl;
            std::cout << "   Minimum 20% of royalties saved automatically during commodity booms." << std::endl;
            std::cout << "   This cannot be cancelled — only amended by future supermajority." << std::endl;
            std::cout << "   (Credibility ++, Debt -, FDI +, Corruption -, Popularity -, Flexibility -)" << std::endl;
        }
    }
    // Investment mandate commands
    else if (command == "swf_conservative") {
        playerCountry.economy.swf_mandate = 0;
        std::cout << ">> SWF MANDATE: Conservative (bonds/cash). Return: ~2.5%, low volatility." << std::endl;
        std::cout << "   Best when: markets volatile, fund small, fiscal crisis risk high." << std::endl;
    }
    else if (command == "swf_balanced") {
        playerCountry.economy.swf_mandate = 1;
        std::cout << ">> SWF MANDATE: Balanced (mixed assets). Return: ~4.5%, moderate volatility." << std::endl;
        std::cout << "   Default strategy for medium-sized funds." << std::endl;
    }
    else if (command == "swf_growth") {
        if (playerCountry.economy.sovereign_wealth_fund < 200000000.0) {
            std::cout << ">> DENIED: Growth mandate requires fund > $200M to absorb market swings." << std::endl;
        } else {
            playerCountry.economy.swf_mandate = 2;
            std::cout << ">> SWF MANDATE: Growth (equities/alternatives). Return: ~6.5%, high volatility." << std::endl;
            std::cout << "   Best when: fund large, long time horizon, fiscal position stable." << std::endl;
        }
    }
    // Governance / transparency
    else if (command == "swf_transparency") {
        playerCountry.economy.swf_transparent = !playerCountry.economy.swf_transparent;
        if (playerCountry.economy.swf_transparent) {
            playerCountry.welfare.un_score                  += 0.04;
            playerCountry.politics.administrative_corruption -= 0.03;
            playerCountry.politics.financial_power          += 0.04; // Markets trust it
            std::cout << ">> SWF REFORM: Independent board established. Annual reports published." << std::endl;
            std::cout << "   (FDI +, Corruption -, UN +, Leakage stops)" << std::endl;
        } else {
            playerCountry.politics.administrative_corruption += 0.02;
            std::cout << ">> SWF: Oversight suspended. Fund returns to executive control." << std::endl;
            std::cout << "   (Control +, Corruption +, FDI -, Leakage resumes)" << std::endl;
        }
    }
    // --- COMMODITY PRICE COMMANDS ---
    else if (command == "hedge_prices") {
        if (playerCountry.economy.mining_concessions <= 0) {
            std::cout << ">> No mining operations to hedge." << std::endl;
        } else if (playerCountry.economy.commodity_hedge_turns > 0) {
            std::cout << ">> Price hedge already active for "
                      << playerCountry.economy.commodity_hedge_turns << " more year(s)." << std::endl;
        } else if (playerCountry.economy.sovereign_wealth_fund < 50000000.0
                && playerCountry.politics.financial_power < 0.6) {
            std::cout << ">> DENIED: Insufficient financial capacity to enter derivatives market." << std::endl;
            std::cout << "   Requirement: SWF ≥ $50M OR Financial Sector power > 60%." << std::endl;
        } else {
            // Premium: 8% of last year's royalties — the cost of certainty
            double premium = playerCountry.economy.state_royalties * 0.08;
            playerCountry.economy.gdp -= premium;
            playerCountry.economy.commodity_hedge_turns = 2;
            std::cout << ">> HEDGE: Commodity prices locked at "
                      << playerCountry.economy.commodity_prices << "x for 2 years." << std::endl;
            std::cout << "   Premium paid: $" << premium / 1000000.0 << "M"
                      << "   (Upside capped, downside protected)" << std::endl;
        }
    }
    // --- RESOURCE MANAGEMENT COMMANDS ---
    else if (command == "geo_survey") {
        playerCountry.economy.gdp -= 30000000; // $30M survey cost
        std::uniform_real_distribution<> dist(0.0, 1.0);
        double roll = dist(rng);
        if (playerCountry.economy.resource_depletion > 0.95) {
            std::cout << ">> GEO SURVEY: Extensive surveys confirm reserves are geologically exhausted." << std::endl;
            std::cout << "   $30M spent. No viable new deposits identified." << std::endl;
        } else if (roll < 0.50) {
            // Major find
            double recovery = 0.08 + dist(rng) * 0.10; // 8%–18% reserve extension
            playerCountry.economy.resource_depletion -= recovery;
            if (playerCountry.economy.resource_depletion < 0.0) playerCountry.economy.resource_depletion = 0.0;
            playerCountry.politics.industrial_power += 0.04;
            playerCountry.politics.popularity       += 0.02;
            std::cout << ">> GEO SURVEY: Major deposit discovered! Reserves extended by "
                      << (int)(recovery * 100) << "%." << std::endl;
            std::cout << "   (Depletion -, FDI +, Popularity +)" << std::endl;
        } else if (roll < 0.80) {
            // Minor find
            double recovery = 0.03 + dist(rng) * 0.04; // 3%–7%
            playerCountry.economy.resource_depletion -= recovery;
            if (playerCountry.economy.resource_depletion < 0.0) playerCountry.economy.resource_depletion = 0.0;
            std::cout << ">> GEO SURVEY: Small secondary deposit found. Reserves extended by "
                      << (int)(recovery * 100) << "%." << std::endl;
        } else {
            // Nothing found
            std::cout << ">> GEO SURVEY: No significant new deposits. Existing reserve estimates confirmed." << std::endl;
            std::cout << "   $30M spent with no material change." << std::endl;
        }
    }
    else if (command == "mine_rehab") {
        double legacy = playerCountry.economy.mining_legacy_damage;
        if (playerCountry.economy.resource_depletion < 0.5 && legacy < 0.1) {
            std::cout << ">> MINE REHAB: No significant contamination yet. Rehabilitation applies to damaged areas." << std::endl;
        } else {
            // Cost and cleanup scale with severity — deeper contamination is harder to remediate
            double base_cost, legacy_reduction;
            std::string tier_label;
            if (legacy < 0.25) {
                base_cost = 30000000.0;  legacy_reduction = 0.12; tier_label = "Early-stage";
            } else if (legacy < 0.50) {
                base_cost = 60000000.0;  legacy_reduction = 0.15; tier_label = "Moderate";
            } else if (legacy < 0.75) {
                base_cost = 100000000.0; legacy_reduction = 0.18; tier_label = "Critical";
            } else {
                base_cost = 150000000.0; legacy_reduction = 0.20; tier_label = "Superfund";
            }
            // International co-funding if transparent SWF + high UN score
            double cofinance = 0.0;
            if (playerCountry.economy.swf_transparent && playerCountry.welfare.un_score > 0.65) {
                cofinance = base_cost * 0.30; // IADB/World Bank co-funding
                std::cout << "   [+] International co-financing: $" << cofinance / 1000000.0
                          << "M from multilateral partners." << std::endl;
            }
            double net_cost = base_cost - cofinance;
            playerCountry.economy.gdp -= net_cost;
            double prev_legacy = playerCountry.economy.mining_legacy_damage;
            playerCountry.economy.mining_legacy_damage -= legacy_reduction;
            if (playerCountry.economy.mining_legacy_damage < 0.0) playerCountry.economy.mining_legacy_damage = 0.0;
            // Spatial cleanup: area reduced proportionally to legacy reduction
            if (prev_legacy > 0.0) {
                double area_fraction = legacy_reduction / prev_legacy;
                playerCountry.economy.contaminated_area_km2 *= (1.0 - area_fraction);
            }
            playerCountry.economy.remediation_cost = playerCountry.economy.contaminated_area_km2 * 1800000.0;
            if (playerCountry.economy.mining_legacy_damage >= 0.50)
                playerCountry.economy.superfund_sites = (int)((playerCountry.economy.mining_legacy_damage - 0.50) / 0.25) + 1;
            else
                playerCountry.economy.superfund_sites = 0;
            playerCountry.economy.remediation_active = true;
            playerCountry.infra.potable_water_access += 0.04;
            if (playerCountry.infra.potable_water_access > 1.0) playerCountry.infra.potable_water_access = 1.0;
            playerCountry.infra.pollution_prob -= 0.06;
            if (playerCountry.infra.pollution_prob < 0.0) playerCountry.infra.pollution_prob = 0.0;
            playerCountry.welfare.un_score += 0.04;
            playerCountry.welfare.mental_health_index += 0.02;
            if (playerCountry.welfare.mental_health_index > 1.0) playerCountry.welfare.mental_health_index = 1.0;
            playerCountry.economy.community_conflicts -= 0.10;
            if (playerCountry.economy.community_conflicts < 0.0) playerCountry.economy.community_conflicts = 0.0;
            playerCountry.politics.popularity += 0.02;
            // Restored land recovers some agricultural value
            playerCountry.politics.agricultural_power += 0.015;
            if (playerCountry.politics.agricultural_power > 1.0) playerCountry.politics.agricultural_power = 1.0;
            std::cout << ">> MINE REHAB [" << tier_label << "]: Environmental remediation program launched." << std::endl;
            std::cout << "   Tailings sealed, water treatment installed, land restored." << std::endl;
            std::cout << "   Net cost: $" << net_cost / 1000000.0 << "M | Legacy reduced by "
                      << (int)(legacy_reduction * 100) << "%" << std::endl;
            std::cout << "   (Legacy -, Water +, Pollution -, Mental Health +, Agri +, Conflict -, UN +)" << std::endl;
        }
    }
    else if (command == "env_bond") {
        // Issue an international green/environmental remediation bond
        // Brings in external capital for large-scale cleanup at the cost of sovereign debt
        double legacy = playerCountry.economy.mining_legacy_damage;
        if (legacy < 0.20) {
            std::cout << ">> ENV BOND: Contamination level too low for international green bond issuance." << std::endl;
            std::cout << "   Bond markets require documented environmental emergency (>20% damage)." << std::endl;
        } else {
            double bond_size = 120000000.0; // $120M green bond
            double gdp = playerCountry.economy.gdp;
            playerCountry.economy.debt_to_gdp_ratio += bond_size / gdp; // New sovereign debt
            double prev_legacy_bond = playerCountry.economy.mining_legacy_damage;
            playerCountry.economy.mining_legacy_damage -= 0.28;
            if (playerCountry.economy.mining_legacy_damage < 0.0) playerCountry.economy.mining_legacy_damage = 0.0;
            if (prev_legacy_bond > 0.0) {
                double area_fraction_bond = 0.28 / prev_legacy_bond;
                playerCountry.economy.contaminated_area_km2 *= (1.0 - area_fraction_bond);
            }
            playerCountry.economy.remediation_cost = playerCountry.economy.contaminated_area_km2 * 1800000.0;
            if (playerCountry.economy.mining_legacy_damage >= 0.50)
                playerCountry.economy.superfund_sites = (int)((playerCountry.economy.mining_legacy_damage - 0.50) / 0.25) + 1;
            else
                playerCountry.economy.superfund_sites = 0;
            playerCountry.economy.remediation_active = true;
            playerCountry.infra.potable_water_access += 0.06;
            if (playerCountry.infra.potable_water_access > 1.0) playerCountry.infra.potable_water_access = 1.0;
            playerCountry.infra.pollution_prob -= 0.10;
            if (playerCountry.infra.pollution_prob < 0.0) playerCountry.infra.pollution_prob = 0.0;
            playerCountry.economy.community_conflicts -= 0.12;
            if (playerCountry.economy.community_conflicts < 0.0) playerCountry.economy.community_conflicts = 0.0;
            playerCountry.welfare.un_score += 0.07;
            playerCountry.welfare.health_coverage += 0.03;
            if (playerCountry.welfare.health_coverage > 1.0) playerCountry.welfare.health_coverage = 1.0;
            playerCountry.economy.tourist_safety += 0.04;
            if (playerCountry.economy.tourist_safety > 1.0) playerCountry.economy.tourist_safety = 1.0;
            // Green bond attracts ESG investors → FDI premium
            playerCountry.economy.international_reserves += bond_size * 0.15;
            playerCountry.politics.popularity += 0.03;
            // Sanctions risk drops — visible international commitment
            playerCountry.economy.international_sanctions_prob -= 0.02;
            if (playerCountry.economy.international_sanctions_prob < 0.0)
                playerCountry.economy.international_sanctions_prob = 0.0;
            std::cout << ">> GREEN BOND ISSUED: $120M environmental remediation bond placed on international markets." << std::endl;
            std::cout << "   Debt/GDP +$" << bond_size / 1000000.0 << "M | Legacy -28% | Major cleanup underway." << std::endl;
            std::cout << "   ESG investors signal confidence: reserves +" << bond_size * 0.15 / 1000000.0 << "M inflow." << std::endl;
            std::cout << "   (Legacy --, Water +, Pollution --, Conflict -, Health +, Sanctions -, Debt +)" << std::endl;
        }
    }
    else if (command == "env_audit") {
        // Commission independent environmental audit — accountability signal
        double legacy = playerCountry.economy.mining_legacy_damage;
        double audit_cost = 15000000.0; // $15M
        playerCountry.economy.gdp -= audit_cost;
        // Audit reveals truth → communities trust process → conflict falls
        playerCountry.economy.community_conflicts -= 0.07;
        if (playerCountry.economy.community_conflicts < 0.0) playerCountry.economy.community_conflicts = 0.0;
        playerCountry.welfare.un_score += 0.03;
        playerCountry.security.diplomatic_prestige += 0.02;
        // Audit applies regulatory pressure on mining companies → slower future accumulation
        // (Simulated by slightly improved judicial_independence signal)
        playerCountry.politics.judicial_independence += 0.02;
        if (playerCountry.politics.judicial_independence > 1.0) playerCountry.politics.judicial_independence = 1.0;
        playerCountry.politics.popularity += 0.01;
        // Print detailed contamination assessment
        std::string severity;
        if      (legacy < 0.10) severity = "Minimal — no remediation required";
        else if (legacy < 0.25) severity = "Latent — early-stage groundwater leaching";
        else if (legacy < 0.50) severity = "Moderate — health and agricultural impacts active";
        else if (legacy < 0.75) severity = "Critical — international liability exposure";
        else                    severity = "CATASTROPHIC — urgent superfund intervention required";
        std::cout << ">> ENVIRONMENTAL AUDIT COMPLETED (cost: $15M)" << std::endl;
        std::cout << "   Legacy Damage Index: " << (int)(legacy * 100) << "% — " << severity << std::endl;
        std::cout << "   Recommendations: ";
        if      (legacy < 0.10) std::cout << "Routine monitoring only.";
        else if (legacy < 0.25) std::cout << "mine_rehab sufficient.";
        else if (legacy < 0.50) std::cout << "mine_rehab (x2) or env_bond recommended.";
        else if (legacy < 0.75) std::cout << "env_bond URGENT. mine_rehab alone insufficient at this scale.";
        else                    std::cout << "env_bond + mine_rehab + international pressure required IMMEDIATELY.";
        std::cout << std::endl;
        std::cout << "   (Conflict -, UN +, Prestige +, Oversight +, Popularity +, Cost $15M)" << std::endl;
    }
    // --- COMMUNITY CONFLICT RESOLUTION COMMANDS ---
    else if (command == "consult") {
        if (playerCountry.economy.mining_concessions <= 0) {
            std::cout << ">> No active mining operations to consult about." << std::endl;
        } else {
            // Prior consultation: legally required under ILO 169 for indigenous communities.
            // Expensive and slow but builds durable social license.
            playerCountry.economy.gdp -= 20000000; // $20M process cost
            playerCountry.economy.community_conflicts -= 0.12;
            if (playerCountry.economy.community_conflicts < 0.0) playerCountry.economy.community_conflicts = 0.0;
            playerCountry.welfare.minority_protection += 0.04; // Rights formally recognized
            if (playerCountry.welfare.minority_protection > 1.0) playerCountry.welfare.minority_protection = 1.0;
            playerCountry.welfare.un_score    += 0.05;
            playerCountry.politics.popularity += 0.01; // Looks responsible internationally
            std::cout << ">> CONSULTATION: Prior consent process launched with affected communities." << std::endl;
            std::cout << "   (Conflict --, Rights +, UN +, Cost $20M)" << std::endl;
        }
    }
    else if (command == "mediate") {
        if (playerCountry.economy.community_conflicts < 0.2) {
            std::cout << ">> Conflict level too low. Communities are not in formal dispute." << std::endl;
        } else if (playerCountry.politics.judicial_independence < 0.4) {
            // Weak courts: companies ignore arbitration rulings — mediation collapses
            playerCountry.economy.community_conflicts += 0.03;
            std::cout << ">> MEDIATION FAILED: Judiciary too weak to enforce agreements. Communities enraged." << std::endl;
            std::cout << "   (Conflict +, rebuild judicial independence first)" << std::endl;
        } else {
            playerCountry.economy.gdp -= 10000000; // $10M
            playerCountry.economy.community_conflicts -= 0.08;
            if (playerCountry.economy.community_conflicts < 0.0) playerCountry.economy.community_conflicts = 0.0;
            playerCountry.politics.trust_in_justice += 0.03;
            playerCountry.politics.popularity       += 0.01;
            std::cout << ">> MEDIATION: State arbitration panel established between companies and communities." << std::endl;
            std::cout << "   (Conflict -, Justice +, Cost $10M)" << std::endl;
        }
    }
    else if (command == "suppress_conflict") {
        if (playerCountry.economy.community_conflicts < 0.3) {
            std::cout << ">> Conflict level too low to justify security deployment." << std::endl;
        } else {
            // Short-term suppression — works immediately but poisons the long-term
            playerCountry.economy.community_conflicts -= 0.15;
            if (playerCountry.economy.community_conflicts < 0.0) playerCountry.economy.community_conflicts = 0.0;
            playerCountry.welfare.radicalism_prob        += 0.05; // Martyrs recruit
            playerCountry.welfare.un_score               -= 0.10;
            playerCountry.welfare.forced_disappearances  += 0.05;
            if (playerCountry.welfare.forced_disappearances > 1.0) playerCountry.welfare.forced_disappearances = 1.0;
            playerCountry.security.homicide_rate         += 2.0;
            playerCountry.politics.popularity            -= 0.03;
            playerCountry.politics.polarization_index    += 0.03;
            std::cout << ">> CRACKDOWN: Security forces deployed to mining zones. Blockades broken." << std::endl;
            std::cout << "   (Conflict - now, Radicalism +, Disappearances +, UN --, Popularity -, Polarization +)" << std::endl;
        }
    }
    else if (command == "swf_spend") {
        // Countercyclical withdrawal: only justified in economic downturns
        double withdrawal = 150000000.0; // $150M
        if (playerCountry.economy.sovereign_wealth_fund < withdrawal) {
            std::cout << ">> INSUFFICIENT FUNDS: SWF balance is only $"
                      << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M." << std::endl;
        } else {
            double corruption_loss = withdrawal * playerCountry.politics.administrative_corruption * 0.15;
            playerCountry.economy.sovereign_wealth_fund -= withdrawal;
            playerCountry.economy.tax_collection        += (withdrawal - corruption_loss);
            playerCountry.politics.popularity           += 0.04;
            playerCountry.welfare.unemployment_rate     -= 0.008;
            if (playerCountry.welfare.unemployment_rate < 0.0) playerCountry.welfare.unemployment_rate = 0.0;
            // Fiscal prudence penalty: raiding fund damages credibility
            double swf_gdp_ratio = playerCountry.economy.sovereign_wealth_fund / playerCountry.economy.gdp;
            if (swf_gdp_ratio < 0.05) {
                playerCountry.economy.debt_to_gdp_ratio += 0.005; // Markets lose confidence
                std::cout << "   [!] SWF nearly depleted — bond markets growing concerned." << std::endl;
            }
            if (corruption_loss > 0) {
                std::cout << "   [!] Corruption siphoned $" << corruption_loss / 1000000.0 << "M of the withdrawal." << std::endl;
            }
            std::cout << ">> COUNTERCYCLICAL SPEND: $150M from SWF deployed as fiscal stimulus." << std::endl;
            std::cout << "   Net injected: $" << (withdrawal - corruption_loss) / 1000000.0 << "M" << std::endl;
            std::cout << "   Remaining SWF: $" << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M" << std::endl;
            std::cout << "   (Popularity +, Unemployment -, Savings -)" << std::endl;
        }
    }
    else if (command == "swf_invest") {
        // Deploy SWF into domestic infrastructure — long-term growth multiplier
        double investment = 200000000.0; // $200M
        if (playerCountry.economy.sovereign_wealth_fund < investment) {
            std::cout << ">> INSUFFICIENT FUNDS: SWF balance is only $"
                      << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M." << std::endl;
        } else {
            playerCountry.economy.sovereign_wealth_fund -= investment;
            // Infrastructure improvements
            playerCountry.infra.road_connectivity   += 0.03;
            playerCountry.infra.port_capacity       += 0.02;
            playerCountry.infra.internet_coverage   += 0.02;
            playerCountry.infra.maintenance_level   += 0.04;
            if (playerCountry.infra.road_connectivity > 1.0) playerCountry.infra.road_connectivity = 1.0;
            if (playerCountry.infra.port_capacity    > 1.0) playerCountry.infra.port_capacity    = 1.0;
            if (playerCountry.infra.internet_coverage > 1.0) playerCountry.infra.internet_coverage = 1.0;
            if (playerCountry.infra.maintenance_level > 1.0) playerCountry.infra.maintenance_level = 1.0;
            // Long-term GDP growth effect
            playerCountry.economy.growth_rate       += 0.004;
            playerCountry.economy.international_reserves += investment * 0.10; // FDI attraction
            playerCountry.politics.popularity       += 0.02;
            // Community conflict reduction — visible state investment
            playerCountry.economy.community_conflicts -= 0.04;
            if (playerCountry.economy.community_conflicts < 0.0) playerCountry.economy.community_conflicts = 0.0;
            std::cout << ">> SWF INFRASTRUCTURE DEPLOY: $200M invested in national infrastructure." << std::endl;
            std::cout << "   Remaining SWF: $" << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M" << std::endl;
            std::cout << "   (Roads +, Ports +, Internet +, Growth +, FDI +, Conflict -)" << std::endl;
        }
    }
    else if (command == "swf_debt") {
        // Pay down sovereign debt from SWF — fiscal discipline signal
        double payment = 250000000.0; // $250M
        if (playerCountry.economy.sovereign_wealth_fund < payment) {
            std::cout << ">> INSUFFICIENT FUNDS: SWF balance is only $"
                      << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M." << std::endl;
        } else {
            playerCountry.economy.sovereign_wealth_fund -= payment;
            double gdp = playerCountry.economy.gdp;
            double debt_reduction = payment / gdp;
            playerCountry.economy.debt_to_gdp_ratio -= debt_reduction;
            if (playerCountry.economy.debt_to_gdp_ratio < 0.0) playerCountry.economy.debt_to_gdp_ratio = 0.0;
            // Recalculate debt interest
            double new_total_debt = gdp * playerCountry.economy.debt_to_gdp_ratio;
            playerCountry.economy.debt_interest = new_total_debt * GetInterestRate(playerCountry.economy.credit_rating);
            // Fiscal discipline signal improves credit rating (one step) if debt < 60%
            if (playerCountry.economy.debt_to_gdp_ratio < 0.60) {
                CreditRating cr = playerCountry.economy.credit_rating;
                if (cr == CreditRating::BB)  playerCountry.economy.credit_rating = CreditRating::BBB;
                else if (cr == CreditRating::BBB) playerCountry.economy.credit_rating = CreditRating::A;
                else if (cr == CreditRating::A)   playerCountry.economy.credit_rating = CreditRating::AA;
                else if (cr == CreditRating::B)   playerCountry.economy.credit_rating = CreditRating::BB;
                if (playerCountry.economy.credit_rating != cr) {
                    std::cout << "   [!] RATING UPGRADE: " << CreditRatingToString(cr)
                              << " → " << CreditRatingToString(playerCountry.economy.credit_rating) << std::endl;
                }
            }
            playerCountry.economy.international_reserves += payment * 0.05; // Confidence inflows
            playerCountry.politics.popularity -= 0.01; // Invisible sacrifice
            std::cout << ">> DEBT PAYDOWN: $250M from SWF used to retire sovereign debt." << std::endl;
            std::cout << "   New Debt/GDP: " << playerCountry.economy.debt_to_gdp_ratio * 100 << "%" << std::endl;
            std::cout << "   New Interest: $" << playerCountry.economy.debt_interest / 1000000.0 << "M/yr" << std::endl;
            std::cout << "   Remaining SWF: $" << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M" << std::endl;
            std::cout << "   (Debt -, Interest -, Rating+, Reserves +, Popularity -)" << std::endl;
        }
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
        if (playerCountry.economy.central_bank_autonomy > 0.5) {
            std::cout << ">> BLOCKED: Central Bank Governor refuses to monetize debt. Autonomy is too high." << std::endl;
            std::cout << "   (Lower Autonomy first with 'autonomy-', but beware capital flight)" << std::endl;
        } else {
            double pre_emission = playerCountry.economy.monetary_emission;
            double pre_inflation = playerCountry.economy.inflation;
            int pre_rating = (int)playerCountry.economy.credit_rating;
            double print_amount = playerCountry.economy.gdp * 0.01;
            playerCountry.economy.monetary_emission += 0.01;
            playerCountry.economy.international_reserves += print_amount;
            double curve_inflation = 0.005 + 0.4 * playerCountry.economy.monetary_emission * playerCountry.economy.monetary_emission;
            playerCountry.economy.inflation += curve_inflation;
            if (playerCountry.economy.monetary_emission > 0.15) {
                int new_rating = (int)playerCountry.economy.credit_rating + 1;
                if (new_rating > (int)CreditRating::D) new_rating = (int)CreditRating::D;
                playerCountry.economy.credit_rating = (CreditRating)new_rating;
            }
            playerCountry.economy.exchange_rate_stability -= 0.05;
            if (playerCountry.economy.exchange_rate_stability < 0.0) playerCountry.economy.exchange_rate_stability = 0.0;
            std::cout << "[!!!] DECREE: emision monetaria autorizada" << std::endl;
            std::cout << "      Reservas:        +$" << print_amount / 1000000 << "M" << std::endl;
            std::cout << "      Emision (M2):    " << (int)(pre_emission*100) << "% -> " << (int)(playerCountry.economy.monetary_emission*100) << "%" << std::endl;
            std::cout << "      Inflacion:       +" << (int)(curve_inflation*1000)/10.0 << "% (de " << (int)(pre_inflation*100) << "% a " << (int)(playerCountry.economy.inflation*100) << "%)" << std::endl;
            if ((int)playerCountry.economy.credit_rating != pre_rating)
                std::cout << "      Credit rating:   DOWNGRADE a " << CreditRatingToString(playerCountry.economy.credit_rating) << std::endl;
            std::cout << "      Estabilidad FX:  -5pts" << std::endl;
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
    // --- TRADE POLICY COMMANDS ---
    else if (command == "tariff+") {
        double old_tariff = playerCountry.economy.average_tariffs;
        if (old_tariff >= 0.40) {
            std::cout << ">> MAXIMUM: Tariff wall at 40%. Any further isolation requires autarky laws." << std::endl;
        } else {
            playerCountry.economy.average_tariffs += 0.05;
            double new_tariff = playerCountry.economy.average_tariffs;
            // Domestic producers sheltered
            playerCountry.politics.industrial_power   += 0.015;
            if (playerCountry.politics.industrial_power  > 1.0) playerCountry.politics.industrial_power  = 1.0;
            playerCountry.politics.agricultural_power += 0.010;
            if (playerCountry.politics.agricultural_power > 1.0) playerCountry.politics.agricultural_power = 1.0;
            // Consumer import-price pass-through
            playerCountry.economy.inflation    += 0.006;
            playerCountry.politics.popularity  -= 0.020;
            // FTA retaliation risk (probability rises with existing tariff level)
            std::uniform_real_distribution<> d(0.0, 1.0);
            double retaliation_prob = 0.25 + old_tariff * 0.8; // More likely already high
            if (playerCountry.economy.free_trade_agreements > 0 && d(rng) < retaliation_prob) {
                playerCountry.economy.free_trade_agreements -= 1;
                std::cout << "   [!] RETALIATION: A trading partner suspended their FTA over new tariffs." << std::endl;
            }
            // Regime-crossing consequences
            if (old_tariff < 0.15 && new_tariff >= 0.15) {
                std::cout << "   [!] REGIME SHIFT: Entering protectionist territory. "
                          << "Smuggling networks will begin forming." << std::endl;
            } else if (old_tariff < 0.25 && new_tariff >= 0.25) {
                playerCountry.economy.international_sanctions_prob += 0.05;
                std::cout << "   [!] REGIME SHIFT: Above 25% — WTO dispute filings expected. "
                          << "Sanctions risk rising. (+5% sanction prob)" << std::endl;
            } else if (old_tariff < 0.35 && new_tariff >= 0.35) {
                playerCountry.economy.international_reserves -= playerCountry.economy.gdp * 0.01;
                std::cout << "   [!!!] REGIME SHIFT: Near-autarky level. "
                          << "FDI fleeing. Black markets will dominate trade flows." << std::endl;
            }
            std::cout << ">> TARIFF INCREASE: Average tariff raised to " << (int)(new_tariff * 100) << "%." << std::endl;
            std::cout << "   (Industry +, Agri +, Customs Revenue +, Inflation +, Popularity -, FTA risk)" << std::endl;
        }
    }
    else if (command == "tariff-") {
        double old_tariff = playerCountry.economy.average_tariffs;
        if (old_tariff <= 0.0) {
            std::cout << ">> Tariffs already at zero. Economy is fully open to world trade." << std::endl;
        } else {
            playerCountry.economy.average_tariffs -= 0.05;
            if (playerCountry.economy.average_tariffs < 0.0) playerCountry.economy.average_tariffs = 0.0;
            double new_tariff = playerCountry.economy.average_tariffs;
            // Open economy signal
            playerCountry.economy.international_reserves += 8000000.0;
            playerCountry.economy.inflation              -= 0.003;
            if (playerCountry.economy.inflation < 0.0) playerCountry.economy.inflation = 0.0;
            playerCountry.politics.popularity            += 0.015;
            playerCountry.welfare.un_score               += 0.020;
            // Domestic producers face import competition
            playerCountry.politics.industrial_power      -= 0.012;
            if (playerCountry.politics.industrial_power  < 0.0) playerCountry.politics.industrial_power  = 0.0;
            playerCountry.politics.agricultural_power    -= 0.008;
            if (playerCountry.politics.agricultural_power < 0.0) playerCountry.politics.agricultural_power = 0.0;
            // Regime-crossing bonuses
            if (old_tariff >= 0.35 && new_tariff < 0.35) {
                playerCountry.economy.international_reserves += 20000000.0; // FDI re-entry signal
                std::cout << "   [+] REGIME SHIFT: Exiting near-autarky. FDI confidence returning." << std::endl;
            } else if (old_tariff >= 0.25 && new_tariff < 0.25) {
                playerCountry.economy.international_sanctions_prob -= 0.04;
                if (playerCountry.economy.international_sanctions_prob < 0.0)
                    playerCountry.economy.international_sanctions_prob = 0.0;
                std::cout << "   [+] REGIME SHIFT: Below 25% — WTO disputes defused. Sanctions risk reduced." << std::endl;
            } else if (old_tariff >= 0.15 && new_tariff < 0.15) {
                // Smuggling networks begin to dissolve
                playerCountry.welfare.labor_informality         -= 0.010;
                if (playerCountry.welfare.labor_informality < 0.0) playerCountry.welfare.labor_informality = 0.0;
                playerCountry.politics.administrative_corruption -= 0.008;
                if (playerCountry.politics.administrative_corruption < 0.0)
                    playerCountry.politics.administrative_corruption = 0.0;
                std::cout << "   [+] REGIME SHIFT: Below 15% — smuggling networks losing profitability. "
                          << "Customs corruption declining." << std::endl;
            } else if (new_tariff < 0.05) {
                // Near-free trade: consumer surplus materializes
                playerCountry.welfare.poverty_rate -= 0.005;
                if (playerCountry.welfare.poverty_rate < 0.0) playerCountry.welfare.poverty_rate = 0.0;
                std::cout << "   [+] NEAR FREE TRADE: Consumer prices falling. Poverty rate improving." << std::endl;
            }
            std::cout << ">> TARIFF REDUCTION: Average tariff lowered to " << (int)(new_tariff * 100) << "%." << std::endl;
            std::cout << "   (FDI +, Inflation -, Popularity +, UN +, Industry -, Agri -)" << std::endl;
        }
    }
    else if (command == "tariff_dumping") {
        // Anti-dumping emergency measure: impose targeted duties on below-cost foreign imports
        // WTO allows this but it's politically costly and invites narrow retaliation
        double commodity_price = playerCountry.economy.commodity_prices;
        if (commodity_price > 0.85) {
            std::cout << ">> ANTI-DUMPING DENIED: No active commodity price crisis to justify emergency duties." << std::endl;
            std::cout << "   Prices are (" << commodity_price << "x) — within normal range." << std::endl;
        } else if (playerCountry.economy.average_tariffs >= 0.35) {
            std::cout << ">> BLOCKED: Existing tariff wall already at "
                      << (int)(playerCountry.economy.average_tariffs * 100) << "%. "
                      << "Anti-dumping duties require room to operate." << std::endl;
        } else {
            // Emergency 10% sectoral duty on competing imports — not a blanket tariff increase
            double duty_revenue = playerCountry.economy.gdp
                                * playerCountry.economy.import_dependency * 0.10 * 0.70;
            playerCountry.economy.tax_collection     += duty_revenue;
            playerCountry.politics.industrial_power  += 0.025; // Protected from dumped goods
            if (playerCountry.politics.industrial_power > 1.0) playerCountry.politics.industrial_power = 1.0;
            playerCountry.politics.agricultural_power += 0.020;
            if (playerCountry.politics.agricultural_power > 1.0) playerCountry.politics.agricultural_power = 1.0;
            playerCountry.welfare.unemployment_rate  -= 0.010; // Jobs saved in affected sectors
            if (playerCountry.welfare.unemployment_rate < 0.0) playerCountry.welfare.unemployment_rate = 0.0;
            // WTO-legal but still invites narrow retaliation
            playerCountry.economy.international_sanctions_prob += 0.015;
            // Narrow measure — doesn't raise general average_tariffs, but some FTA friction
            std::uniform_real_distribution<> d(0.0, 1.0);
            if (playerCountry.economy.free_trade_agreements > 0 && d(rng) < 0.25) {
                playerCountry.economy.free_trade_agreements -= 1;
                std::cout << "   [!] NARROW RETALIATION: One partner targeted our exports in response." << std::endl;
            }
            std::cout << ">> ANTI-DUMPING DUTIES IMPOSED: Emergency 10% sectoral duties on below-cost imports." << std::endl;
            std::cout << "   Customs revenue: $" << duty_revenue / 1000000.0 << "M this year." << std::endl;
            std::cout << "   (Industry +, Agri +, Employment +, Sanctions risk +, FTA risk)" << std::endl;
        }
    }
    else if (command == "fta_sign") {
        double negotiation_cost = 25000000.0; // $25M
        if (playerCountry.economy.international_sanctions_prob > 0.5) {
            std::cout << ">> BLOCKED: No partner willing to sign FTA with a sanctioned state." << std::endl;
        } else if (playerCountry.economy.average_tariffs > 0.25) {
            std::cout << ">> STALLED: Partners unwilling to sign FTA while average tariffs exceed 25%." << std::endl;
            std::cout << "   (Use tariff- to reduce barriers below 25% first)" << std::endl;
        } else if (playerCountry.economy.gdp < negotiation_cost * 3) {
            std::cout << ">> INSUFFICIENT RESOURCES: FTA negotiations require $25M." << std::endl;
        } else {
            playerCountry.economy.gdp -= negotiation_cost;
            playerCountry.economy.free_trade_agreements += 1;
            // Economic effects: new export markets, but import competition rises
            playerCountry.economy.international_reserves += 15000000.0; // Trade confidence signal
            playerCountry.economy.growth_rate            += 0.002;      // Long-run channel
            playerCountry.politics.financial_power       += 0.025;
            playerCountry.welfare.un_score               += 0.02;
            // Losers from trade — lobby resistance materializes as political costs
            playerCountry.politics.industrial_power      -= 0.012;
            if (playerCountry.politics.industrial_power < 0.0) playerCountry.politics.industrial_power = 0.0;
            playerCountry.politics.agricultural_power    -= 0.010;
            if (playerCountry.politics.agricultural_power < 0.0) playerCountry.politics.agricultural_power = 0.0;
            playerCountry.politics.popularity            -= 0.015; // Industry workers and farmers resist
            std::cout << ">> FTA SIGNED: New free trade agreement. Total agreements: "
                      << playerCountry.economy.free_trade_agreements << "." << std::endl;
            std::cout << "   (Growth +, FDI +, UN +, Industry -, Agri -, Popularity -, Cost $25M)" << std::endl;
        }
    }
    // --- DEFENSE COMMANDS ---
    else if (command == "mil_budget+") {
        playerCountry.security.military_spending_gdp += 0.005;
        if (playerCountry.security.military_spending_gdp > 0.10) playerCountry.security.military_spending_gdp = 0.10;
        double cost = playerCountry.economy.gdp * 0.005;
        playerCountry.economy.gdp -= cost;
        playerCountry.security.troop_morale += 0.05;
        if (playerCountry.security.troop_morale > 1.0) playerCountry.security.troop_morale = 1.0;
        playerCountry.security.equipment_modernization += 0.03;
        if (playerCountry.security.equipment_modernization > 1.0) playerCountry.security.equipment_modernization = 1.0;
        playerCountry.politics.popularity -= 0.01; // Guns vs butter
        std::cout << ">> DEFENSE BUDGET INCREASED to " << playerCountry.security.military_spending_gdp * 100
                  << "% GDP. Cost: $" << (int)(cost / 1000000.0) << "M" << std::endl;
        std::cout << "   (Morale +, Equipment +, Budget -, Popularity -)" << std::endl;
    }
    else if (command == "mil_budget-") {
        playerCountry.security.military_spending_gdp -= 0.005;
        if (playerCountry.security.military_spending_gdp < 0.005) playerCountry.security.military_spending_gdp = 0.005;
        playerCountry.security.troop_morale -= 0.05;
        if (playerCountry.security.troop_morale < 0.1) playerCountry.security.troop_morale = 0.1;
        playerCountry.security.military_insubordination_prob += 0.02;
        playerCountry.politics.popularity += 0.01; // Peace dividend
        std::cout << ">> DEFENSE BUDGET CUT to " << playerCountry.security.military_spending_gdp * 100
                  << "% GDP. Peace dividend unlocked." << std::endl;
        std::cout << "   (Morale -, Insubordination +, Popularity +)" << std::endl;
    }
    else if (command == "modernize") {
        double cost = 100000000.0; // $100M
        if (playerCountry.economy.gdp < cost * 5) {
            std::cout << ">> INSUFFICIENT RESOURCES: Equipment modernization requires $100M." << std::endl;
        } else {
            playerCountry.economy.gdp -= cost;
            playerCountry.security.equipment_modernization += 0.1;
            if (playerCountry.security.equipment_modernization > 1.0) playerCountry.security.equipment_modernization = 1.0;
            playerCountry.security.equipment_readiness += 0.05;
            if (playerCountry.security.equipment_readiness > 1.0) playerCountry.security.equipment_readiness = 1.0;
            playerCountry.security.troop_morale += 0.03;
            std::cout << ">> MILITARY MODERNIZATION: $100M invested in new equipment." << std::endl;
            std::cout << "   Equipment: " << (int)(playerCountry.security.equipment_modernization * 100)
                      << "%, Readiness: " << (int)(playerCountry.security.equipment_readiness * 100) << "%" << std::endl;
        }
    }
    else if (command == "deploy") {
        if (playerCountry.security.troop_morale < 0.3) {
            std::cout << ">> REFUSED: Troop morale too low for deployment." << std::endl;
        } else {
            playerCountry.security.conflict_with_groups += 0.1;
            if (playerCountry.security.conflict_with_groups > 1.0) playerCountry.security.conflict_with_groups = 1.0;
            playerCountry.security.troop_morale -= 0.05;
            playerCountry.security.combat_stress_index += 0.05;
            playerCountry.security.conflict_casualties_annual += 20;
            playerCountry.security.homicide_rate -= 1.0; // Temporarily reduces crime
            if (playerCountry.security.homicide_rate < 1.0) playerCountry.security.homicide_rate = 1.0;
            playerCountry.welfare.torture_index += 0.05; // Human rights cost
            std::cout << ">> TROOPS DEPLOYED: Military operations against non-state groups." << std::endl;
            std::cout << "   (Security +, Morale -, Casualties +, Human Rights -)" << std::endl;
        }
    }
    // --- ESPIONAGE COMMANDS ---
    else if (command == "spy_budget+") {
        double cost = playerCountry.economy.gdp * 0.002;
        playerCountry.economy.gdp -= cost;
        playerCountry.security.espionage_budget += cost;
        playerCountry.security.humint_capability += 0.05;
        if (playerCountry.security.humint_capability > 1.0) playerCountry.security.humint_capability = 1.0;
        playerCountry.security.sigint_capability += 0.03;
        if (playerCountry.security.sigint_capability > 1.0) playerCountry.security.sigint_capability = 1.0;
        playerCountry.security.attack_detection_prob += 0.02;
        std::cout << ">> ESPIONAGE BUDGET INCREASED. Intelligence capabilities enhanced." << std::endl;
        std::cout << "   HUMINT: " << (int)(playerCountry.security.humint_capability * 100)
                  << "%, SIGINT: " << (int)(playerCountry.security.sigint_capability * 100) << "%" << std::endl;
    }
    else if (command == "cyber_op") {
        if (playerCountry.security.sigint_capability < 0.3) {
            std::cout << ">> FAILED: SIGINT capability too low for cyber operations." << std::endl;
        } else {
            double success_prob = playerCountry.security.sigint_capability * 0.5
                                + playerCountry.infra.cyber_defense_maturity * 0.3;
            std::uniform_real_distribution<> dist01(0.0, 1.0);
            if (dist01(rng) < success_prob) {
                playerCountry.security.diplomatic_prestige += 0.02; // Secret success
                playerCountry.security.attack_detection_prob += 0.03;
                std::cout << ">> CYBER OPERATION SUCCESS: Target systems compromised. Intel gained." << std::endl;
            } else {
                playerCountry.security.document_leak_prob += 0.05; // Exposure risk
                playerCountry.security.diplomatic_prestige -= 0.05;
                playerCountry.economy.international_sanctions_prob += 0.03;
                std::cout << ">> CYBER OPERATION EXPOSED: Attribution detected. Diplomatic fallout!" << std::endl;
            }
        }
    }
    else if (command == "surveillance+") {
        playerCountry.security.mass_surveillance_active = true;
        playerCountry.security.cyber_surveillance += 0.1;
        if (playerCountry.security.cyber_surveillance > 1.0) playerCountry.security.cyber_surveillance = 1.0;
        playerCountry.security.social_media_monitoring += 0.1;
        if (playerCountry.security.social_media_monitoring > 1.0) playerCountry.security.social_media_monitoring = 1.0;
        playerCountry.security.terrorism_detection_prob += 0.05;
        playerCountry.security.attack_detection_prob += 0.03;
        // Liberty costs
        playerCountry.security.press_freedom -= 0.05;
        if (playerCountry.security.press_freedom < 0.1) playerCountry.security.press_freedom = 0.1;
        playerCountry.welfare.freedom_of_expression -= 0.05;
        if (playerCountry.welfare.freedom_of_expression < 0.1) playerCountry.welfare.freedom_of_expression = 0.1;
        playerCountry.politics.democratic_backsliding_index += 0.02;
        std::cout << ">> MASS SURVEILLANCE ACTIVATED: Detection up, Liberty down." << std::endl;
        std::cout << "   (Terror Detection +, Press Freedom -, Democracy -)" << std::endl;
    }
    else if (command == "surveillance-") {
        playerCountry.security.mass_surveillance_active = false;
        playerCountry.security.cyber_surveillance -= 0.1;
        if (playerCountry.security.cyber_surveillance < 0.0) playerCountry.security.cyber_surveillance = 0.0;
        playerCountry.security.terrorism_detection_prob -= 0.03;
        playerCountry.security.press_freedom += 0.03;
        playerCountry.welfare.freedom_of_expression += 0.03;
        playerCountry.politics.democratic_backsliding_index -= 0.01;
        if (playerCountry.politics.democratic_backsliding_index < 0.0) playerCountry.politics.democratic_backsliding_index = 0.0;
        std::cout << ">> SURVEILLANCE REDUCED: Civil liberties restored. Detection capability down." << std::endl;
    }
    // --- ENERGY POLICY COMMANDS ---
    else if (command == "renewables+") {
        double cost = playerCountry.economy.gdp * 0.01;
        if (playerCountry.economy.gdp < cost * 5) {
            std::cout << ">> INSUFFICIENT RESOURCES for renewable energy investment." << std::endl;
        } else {
            playerCountry.economy.gdp -= cost;
            playerCountry.infra.renewables_percentage += 0.05;
            if (playerCountry.infra.renewables_percentage > 0.95) playerCountry.infra.renewables_percentage = 0.95;
            playerCountry.infra.solar_capacity_gw += 0.5;
            playerCountry.infra.wind_capacity_gw += 0.3;
            playerCountry.infra.fossil_fuel_dependency -= 0.03;
            if (playerCountry.infra.fossil_fuel_dependency < 0.05) playerCountry.infra.fossil_fuel_dependency = 0.05;
            playerCountry.infra.co2_emissions *= 0.97;
            playerCountry.infra.energy_transition_speed += 0.005;
            // Industry backlash
            playerCountry.politics.extractive_sector_power -= 0.02;
            std::cout << ">> RENEWABLE ENERGY INVESTMENT: $" << (int)(cost / 1000000.0) << "M deployed." << std::endl;
            std::cout << "   Renewables: " << (int)(playerCountry.infra.renewables_percentage * 100)
                      << "%, Fossil dependency: " << (int)(playerCountry.infra.fossil_fuel_dependency * 100) << "%" << std::endl;
        }
    }
    else if (command == "carbon_tax") {
        if (playerCountry.infra.carbon_tax_active) {
            playerCountry.infra.carbon_tax_active = false;
            playerCountry.infra.carbon_tax_rate = 0.0;
            playerCountry.politics.popularity += 0.02;
            playerCountry.politics.industrial_power += 0.05;
            std::cout << ">> CARBON TAX REPEALED: Industry celebrates, emissions resume." << std::endl;
        } else {
            playerCountry.infra.carbon_tax_active = true;
            playerCountry.infra.carbon_tax_rate = 30.0; // $30/ton
            playerCountry.infra.co2_emissions *= 0.95;
            playerCountry.economy.tax_collection += playerCountry.infra.co2_emissions * 0.00003;
            playerCountry.politics.popularity -= 0.03;
            playerCountry.politics.industrial_power -= 0.05;
            if (playerCountry.politics.industrial_power < 0.0) playerCountry.politics.industrial_power = 0.0;
            playerCountry.infra.energy_transition_speed += 0.01;
            std::cout << ">> CARBON TAX ENACTED: $30/ton CO2. Emissions reduced, industry unhappy." << std::endl;
            std::cout << "   (Emissions -, Revenue +, Industry -, Popularity -)" << std::endl;
        }
    }
    else if (command == "energy_subsidy") {
        double cost = playerCountry.economy.gdp * 0.005;
        playerCountry.economy.gdp -= cost;
        playerCountry.infra.energy_subsidy_gdp += 0.005;
        playerCountry.infra.energy_affordability += 0.05;
        if (playerCountry.infra.energy_affordability > 1.0) playerCountry.infra.energy_affordability = 1.0;
        playerCountry.infra.kwh_price *= 0.95;
        playerCountry.politics.popularity += 0.02;
        // Distortion: keeps fossil dependency
        playerCountry.infra.fossil_fuel_dependency += 0.01;
        if (playerCountry.infra.fossil_fuel_dependency > 1.0) playerCountry.infra.fossil_fuel_dependency = 1.0;
        std::cout << ">> ENERGY SUBSIDIES INCREASED: Affordable energy, fiscal cost $"
                  << (int)(cost / 1000000.0) << "M." << std::endl;
        std::cout << "   (Affordability +, Popularity +, Fossil Lock-in +, Budget -)" << std::endl;
    }
    // --- SCIENCE & SPACE COMMANDS ---
    else if (command == "rd_budget+") {
        double cost = playerCountry.economy.gdp * 0.005;
        playerCountry.economy.gdp -= cost;
        playerCountry.infra.st_investment_gdp += 0.005;
        if (playerCountry.infra.st_investment_gdp > 0.05) playerCountry.infra.st_investment_gdp = 0.05;
        playerCountry.infra.innovation_index += 0.02;
        if (playerCountry.infra.innovation_index > 1.0) playerCountry.infra.innovation_index = 1.0;
        playerCountry.infra.patent_development += 10;
        playerCountry.infra.researcher_density += 20.0;
        std::cout << ">> R&D BUDGET INCREASED to " << playerCountry.infra.st_investment_gdp * 100
                  << "% GDP. Innovation boosted." << std::endl;
    }
    else if (command == "rd_budget-") {
        playerCountry.infra.st_investment_gdp -= 0.005;
        if (playerCountry.infra.st_investment_gdp < 0.001) playerCountry.infra.st_investment_gdp = 0.001;
        playerCountry.infra.innovation_index -= 0.01;
        if (playerCountry.infra.innovation_index < 0.1) playerCountry.infra.innovation_index = 0.1;
        playerCountry.welfare.brain_drain += 0.01;
        std::cout << ">> R&D BUDGET CUT to " << playerCountry.infra.st_investment_gdp * 100
                  << "% GDP. Brain drain accelerating." << std::endl;
    }
    else if (command == "space+") {
        double cost = 50000000.0;
        if (playerCountry.economy.gdp < cost * 10) {
            std::cout << ">> INSUFFICIENT RESOURCES for space program expansion." << std::endl;
        } else {
            playerCountry.economy.gdp -= cost;
            playerCountry.infra.space_budget += cost;
            playerCountry.infra.space_budget_gdp += 0.001;
            playerCountry.infra.technological_prestige += 0.03;
            if (playerCountry.infra.technological_prestige > 1.0) playerCountry.infra.technological_prestige = 1.0;
            playerCountry.infra.space_prestige += 0.02;
            if (!playerCountry.infra.own_launch_capability && playerCountry.infra.space_budget > 200000000.0) {
                playerCountry.infra.own_launch_capability = true;
                std::cout << "   [!!!] MILESTONE: Domestic launch capability achieved!" << std::endl;
            }
            std::cout << ">> SPACE PROGRAM: $50M invested. Prestige: "
                      << (int)(playerCountry.infra.space_prestige * 100) << "%" << std::endl;
        }
    }
    else if (command == "ai_strategy") {
        if (playerCountry.infra.ai_national_strategy) {
            std::cout << ">> AI strategy already in effect." << std::endl;
        } else {
            double cost = 30000000.0;
            playerCountry.economy.gdp -= cost;
            playerCountry.infra.ai_national_strategy = true;
            playerCountry.infra.state_ai_development += 0.1;
            playerCountry.infra.ai_talent_pool += 0.05;
            playerCountry.infra.employment_automation += 0.02;
            playerCountry.infra.innovation_index += 0.03;
            playerCountry.infra.technological_prestige += 0.02;
            std::cout << ">> NATIONAL AI STRATEGY LAUNCHED: $30M initial investment." << std::endl;
            std::cout << "   (AI Development +, Talent +, Innovation +, Automation Risk +)" << std::endl;
        }
    }
    // --- MEDIA & PROPAGANDA COMMANDS ---
    else if (command == "media_control+") {
        playerCountry.security.media_control += 0.1;
        if (playerCountry.security.media_control > 1.0) playerCountry.security.media_control = 1.0;
        playerCountry.security.press_freedom -= 0.08;
        if (playerCountry.security.press_freedom < 0.05) playerCountry.security.press_freedom = 0.05;
        playerCountry.security.narrative_reach += 0.05;
        if (playerCountry.security.narrative_reach > 1.0) playerCountry.security.narrative_reach = 1.0;
        playerCountry.security.self_censorship_rate += 0.05;
        playerCountry.politics.democratic_backsliding_index += 0.02;
        playerCountry.economy.international_sanctions_prob += 0.01;
        std::cout << ">> MEDIA CONTROL TIGHTENED: State narrative dominates. Press freedom eroded." << std::endl;
        std::cout << "   (Narrative +, Press Freedom -, Democracy -, Sanctions Risk +)" << std::endl;
    }
    else if (command == "propaganda") {
        double cost = 10000000.0;
        playerCountry.economy.gdp -= cost;
        playerCountry.security.narrative_reach += 0.1;
        if (playerCountry.security.narrative_reach > 1.0) playerCountry.security.narrative_reach = 1.0;
        playerCountry.security.social_media_reach += 0.05;
        playerCountry.politics.popularity += 0.03;
        // Risk of backfire if press is free
        if (playerCountry.security.press_freedom > 0.6) {
            playerCountry.security.fake_news_success_prob += 0.05;
            std::cout << ">> PROPAGANDA CAMPAIGN: $10M spent. But free press may expose it." << std::endl;
        } else {
            playerCountry.politics.popularity += 0.02; // Extra boost with controlled media
            std::cout << ">> PROPAGANDA CAMPAIGN: $10M spent. Message reaches "
                      << (int)(playerCountry.security.narrative_reach * 100) << "% of population." << std::endl;
        }
    }
    else if (command == "press_free+") {
        playerCountry.security.press_freedom += 0.1;
        if (playerCountry.security.press_freedom > 1.0) playerCountry.security.press_freedom = 1.0;
        playerCountry.security.media_control -= 0.05;
        if (playerCountry.security.media_control < 0.0) playerCountry.security.media_control = 0.0;
        playerCountry.security.media_pluralism += 0.05;
        if (playerCountry.security.media_pluralism > 1.0) playerCountry.security.media_pluralism = 1.0;
        playerCountry.security.self_censorship_rate -= 0.05;
        if (playerCountry.security.self_censorship_rate < 0.0) playerCountry.security.self_censorship_rate = 0.0;
        playerCountry.politics.democratic_backsliding_index -= 0.01;
        if (playerCountry.politics.democratic_backsliding_index < 0.0) playerCountry.politics.democratic_backsliding_index = 0.0;
        playerCountry.security.diplomatic_prestige += 0.02;
        playerCountry.politics.auth_dem_axis -= 0.02; // Democratic shift
        if (playerCountry.politics.auth_dem_axis < 0.0) playerCountry.politics.auth_dem_axis = 0.0;
        // Risk: free press exposes government scandals
        playerCountry.security.document_leak_prob += 0.02;
        std::cout << ">> PRESS FREEDOM PROTECTED: Media independence strengthened." << std::endl;
        std::cout << "   (Pluralism +, Democracy +, Prestige +, Leak Risk +)" << std::endl;
    }
    // --- EMERGENCY & REPRESSION COMMANDS ---
    else if (command == "emergency") {
        if (playerCountry.politics.state_of_emergency_active) {
            playerCountry.politics.state_of_emergency_active = false;
            playerCountry.politics.emergency_turns_elapsed = 0;
            playerCountry.welfare.freedom_of_expression += 0.05;
            playerCountry.security.press_freedom += 0.03;
            std::cout << ">> STATE OF EMERGENCY LIFTED: Civil liberties restored." << std::endl;
        } else {
            playerCountry.politics.state_of_emergency_active = true;
            playerCountry.politics.emergency_turns_elapsed = 0;
            playerCountry.politics.protest_intensity *= 0.5; // Immediate suppression
            playerCountry.welfare.freedom_of_expression -= 0.1;
            if (playerCountry.welfare.freedom_of_expression < 0.1) playerCountry.welfare.freedom_of_expression = 0.1;
            playerCountry.security.press_freedom -= 0.1;
            if (playerCountry.security.press_freedom < 0.1) playerCountry.security.press_freedom = 0.1;
            playerCountry.politics.democratic_backsliding_index += 0.05;
            playerCountry.politics.authoritarianism_prob += 0.05;
            playerCountry.politics.auth_dem_axis += 0.05; // Authoritarian shift
            if (playerCountry.politics.auth_dem_axis > 1.0) playerCountry.politics.auth_dem_axis = 1.0;
            std::cout << ">> STATE OF EMERGENCY DECLARED: Protests suppressed. Rights suspended." << std::endl;
            std::cout << "   (Stability +, Freedom -, Democracy -, International Scrutiny +)" << std::endl;
        }
    }
    else if (command == "riot_police") {
        if (playerCountry.politics.protest_intensity < 0.1) {
            std::cout << ">> No significant protests to suppress." << std::endl;
        } else {
            playerCountry.politics.protest_intensity *= 0.6;
            playerCountry.welfare.torture_index += 0.1;
            playerCountry.welfare.forced_disappearances += 5;
            playerCountry.welfare.freedom_of_expression -= 0.03;
            playerCountry.welfare.un_score -= 0.03;
            // Can backfire: if violence too high, protests intensify
            std::uniform_real_distribution<> dist01(0.0, 1.0);
            if (dist01(rng) < 0.3) {
                playerCountry.politics.protest_intensity += 0.2;
                playerCountry.politics.polarization_index += 0.05;
                std::cout << ">> RIOT POLICE BACKFIRE: Brutality footage goes viral! Protests intensify." << std::endl;
            } else {
                std::cout << ">> RIOT POLICE DEPLOYED: Protests suppressed through force." << std::endl;
            }
            std::cout << "   (Protest -, Human Rights -, UN Score -, Backfire Risk)" << std::endl;
        }
    }
    else if (command == "curfew") {
        playerCountry.politics.protest_intensity *= 0.7;
        playerCountry.economy.gdp *= 0.995; // Economic cost of restricted movement
        playerCountry.welfare.freedom_of_expression -= 0.02;
        playerCountry.welfare.mental_health_index -= 0.02;
        playerCountry.economy.annual_visitors = (int)(playerCountry.economy.annual_visitors * 0.9);
        std::cout << ">> CURFEW IMPOSED: Streets cleared. Economic activity reduced." << std::endl;
        std::cout << "   (Protest -, GDP -, Freedom -, Tourism -, Mental Health -)" << std::endl;
    }
    // --- NEGOTIATION & DIPLOMACY COMMANDS ---
    else if (command == "negotiate_peace") {
        if (playerCountry.security.conflict_with_groups < 0.2 && !playerCountry.security.war_active) {
            std::cout << ">> No active conflict to negotiate." << std::endl;
        } else {
            double cost = 20000000.0;
            playerCountry.economy.gdp -= cost;
            double peace_chance = playerCountry.security.peace_negotiation_prob
                                + playerCountry.security.diplomatic_prestige * 0.2;
            std::uniform_real_distribution<> dist01(0.0, 1.0);
            if (dist01(rng) < peace_chance) {
                playerCountry.security.ceasefire_active = true;
                playerCountry.security.conflict_with_groups *= 0.5;
                playerCountry.security.conflict_casualties_annual = (int)(playerCountry.security.conflict_casualties_annual * 0.3);
                playerCountry.politics.popularity += 0.05;
                playerCountry.security.diplomatic_prestige += 0.05;
                std::cout << ">> CEASEFIRE ACHIEVED: Peace negotiations successful!" << std::endl;
            } else {
                playerCountry.politics.popularity -= 0.02;
                std::cout << ">> PEACE TALKS COLLAPSED: Groups refuse terms. $20M wasted." << std::endl;
            }
        }
    }
    else if (command == "seek_alliance") {
        double cost = 30000000.0;
        playerCountry.economy.gdp -= cost;
        playerCountry.security.alliance_protection += 0.05;
        if (playerCountry.security.alliance_protection > 1.0) playerCountry.security.alliance_protection = 1.0;
        playerCountry.security.diplomatic_prestige += 0.02;
        playerCountry.security.foreign_intelligence_sharing += 0.05;
        if (playerCountry.security.foreign_intelligence_sharing > 1.0) playerCountry.security.foreign_intelligence_sharing = 1.0;
        // Alignment shift based on current position
        if (playerCountry.security.us_alignment > playerCountry.security.china_alignment) {
            playerCountry.security.us_alignment += 0.05;
            if (playerCountry.security.us_alignment > 1.0) playerCountry.security.us_alignment = 1.0;
            playerCountry.security.china_alignment -= 0.02;
        } else {
            playerCountry.security.china_alignment += 0.05;
            if (playerCountry.security.china_alignment > 1.0) playerCountry.security.china_alignment = 1.0;
            playerCountry.security.us_alignment -= 0.02;
        }
        std::cout << ">> ALLIANCE STRENGTHENED: Military protection enhanced. $30M invested." << std::endl;
        std::cout << "   Alliance: " << (int)(playerCountry.security.alliance_protection * 100)
                  << "%, US align: " << (int)(playerCountry.security.us_alignment * 100)
                  << "%, China align: " << (int)(playerCountry.security.china_alignment * 100) << "%" << std::endl;
    }
    else if (command == "diplomacy+") {
        double cost = 15000000.0;
        playerCountry.economy.gdp -= cost;
        playerCountry.security.diplomatic_prestige += 0.05;
        if (playerCountry.security.diplomatic_prestige > 1.0) playerCountry.security.diplomatic_prestige = 1.0;
        playerCountry.security.soft_power_index += 0.03;
        if (playerCountry.security.soft_power_index > 1.0) playerCountry.security.soft_power_index = 1.0;
        playerCountry.security.multilateral_leadership += 0.03;
        if (playerCountry.security.multilateral_leadership > 1.0) playerCountry.security.multilateral_leadership = 1.0;
        playerCountry.economy.international_sanctions_prob -= 0.02;
        if (playerCountry.economy.international_sanctions_prob < 0.0) playerCountry.economy.international_sanctions_prob = 0.0;
        std::cout << ">> DIPLOMATIC OFFENSIVE: Prestige restored. Sanctions risk reduced." << std::endl;
        std::cout << "   Prestige: " << (int)(playerCountry.security.diplomatic_prestige * 100) << "%" << std::endl;
    }
    // --- ELECTORAL & CONSTITUTIONAL COMMANDS ---
    else if (command == "referendum") {
        double cost = 20000000.0;
        playerCountry.economy.gdp -= cost;
        std::cout << ">> NATIONAL REFERENDUM CALLED on constitutional reform." << std::endl;
        // Result depends on popularity and polarization
        double approval = playerCountry.politics.popularity * 0.6
                        + (1.0 - playerCountry.politics.polarization_index) * 0.3;
        std::uniform_real_distribution<> noise(-0.1, 0.1);
        approval += noise(rng);
        if (approval > 0.5) {
            playerCountry.politics.popularity += 0.05;
            playerCountry.politics.congressional_support += 0.1;
            if (playerCountry.politics.congressional_support > 1.0) playerCountry.politics.congressional_support = 1.0;
            std::cout << "   APPROVED with " << (int)(approval * 100) << "% support. Mandate strengthened." << std::endl;
        } else {
            playerCountry.politics.popularity -= 0.05;
            playerCountry.politics.polarization_index += 0.05;
            std::cout << "   REJECTED with only " << (int)(approval * 100) << "% support. Political setback." << std::endl;
        }
    }
    else if (command == "reform_constitution") {
        if (playerCountry.politics.congressional_support < 0.67) {
            std::cout << ">> BLOCKED: Constitutional reform requires 2/3 congressional support ("
                      << (int)(playerCountry.politics.congressional_support * 100) << "% current)." << std::endl;
        } else {
            playerCountry.politics.constitutional_reforms_pending--;
            if (playerCountry.politics.term_limit_active) {
                playerCountry.politics.term_limit_active = false;
                playerCountry.politics.democratic_backsliding_index += 0.1;
                playerCountry.economy.international_sanctions_prob += 0.05;
                std::cout << ">> CONSTITUTIONAL REFORM: Term limits abolished! Democracy watchdogs alarmed." << std::endl;
            } else {
                playerCountry.politics.judicial_independence += 0.05;
                playerCountry.politics.legislative_efficiency += 0.05;
                std::cout << ">> CONSTITUTIONAL REFORM: Institutional strengthening enacted." << std::endl;
            }
        }
    }
    else if (command == "snap_election") {
        std::cout << ">> SNAP ELECTION CALLED!" << std::endl;
        double effective_vote = playerCountry.politics.popularity
                              + playerCountry.politics.incumbent_advantage
                              + playerCountry.politics.electoral_manipulation_capacity * 0.3;
        std::uniform_real_distribution<> noise2(-0.05, 0.05);
        effective_vote += noise2(rng);
        if (effective_vote > 0.50) {
            playerCountry.politics.popularity += 0.05;
            playerCountry.politics.honeymoon_turns_remaining = 4;
            playerCountry.politics.congressional_support += 0.1;
            if (playerCountry.politics.congressional_support > 1.0) playerCountry.politics.congressional_support = 1.0;
            std::cout << "   VICTORY: " << (int)(effective_vote * 100) << "%. Fresh mandate secured." << std::endl;
        } else {
            std::cout << "   DEFEAT: " << (int)(effective_vote * 100) << "%. You gambled and lost." << std::endl;
            std::cout << "GAME OVER." << std::endl;
            exit(0);
        }
    }
    // --- ADDITIONAL TRADE COMMANDS ---
    else if (command == "capital_controls") {
        if (playerCountry.economy.central_bank_autonomy > 0.8) {
            std::cout << ">> CENTRAL BANK REFUSES: Autonomous central bank blocks capital controls." << std::endl;
        } else {
            playerCountry.infra.capital_flight_risk -= 0.1;
            if (playerCountry.infra.capital_flight_risk < 0.0) playerCountry.infra.capital_flight_risk = 0.0;
            playerCountry.infra.fdi_inflow_gdp -= 0.01;
            if (playerCountry.infra.fdi_inflow_gdp < 0.0) playerCountry.infra.fdi_inflow_gdp = 0.0;
            playerCountry.economy.exchange_rate_stability += 0.05;
            if (playerCountry.economy.exchange_rate_stability > 1.0) playerCountry.economy.exchange_rate_stability = 1.0;
            playerCountry.economy.trade_openness -= 0.05;
            if (playerCountry.economy.trade_openness < 0.1) playerCountry.economy.trade_openness = 0.1;
            playerCountry.infra.investment_climate_index -= 0.05;
            if (playerCountry.infra.investment_climate_index < 0.1) playerCountry.infra.investment_climate_index = 0.1;
            std::cout << ">> CAPITAL CONTROLS IMPOSED: Outflows restricted. FDI chilled." << std::endl;
            std::cout << "   (Capital Flight -, FDI -, Exchange Stability +, Investment Climate -)" << std::endl;
        }
    }
    else if (command == "embargo") {
        playerCountry.economy.trade_openness -= 0.1;
        if (playerCountry.economy.trade_openness < 0.05) playerCountry.economy.trade_openness = 0.05;
        playerCountry.economy.average_tariffs += 0.1;
        playerCountry.economy.import_dependency -= 0.05;
        if (playerCountry.economy.import_dependency < 0.05) playerCountry.economy.import_dependency = 0.05;
        playerCountry.economy.inflation += 0.02;
        playerCountry.security.diplomatic_prestige -= 0.05;
        std::cout << ">> TRADE EMBARGO: Imports restricted. Self-sufficiency push at inflation cost." << std::endl;
        std::cout << "   (Imports -, Inflation +, Dependency -, Prestige -)" << std::endl;
    }
    else if (command == "devalue") {
        if (playerCountry.economy.central_bank_autonomy > 0.7) {
            std::cout << ">> CENTRAL BANK REFUSES: Independent bank won't devalue on political orders." << std::endl;
        } else {
            playerCountry.economy.exchange_rate_stability -= 0.1;
            if (playerCountry.economy.exchange_rate_stability < 0.1) playerCountry.economy.exchange_rate_stability = 0.1;
            playerCountry.economy.inflation += 0.03;
            playerCountry.economy.trade_balance += playerCountry.economy.gdp * 0.01; // Export boost
            playerCountry.economy.growth_rate += 0.005;
            playerCountry.politics.popularity -= 0.02;
            std::cout << ">> CURRENCY DEVALUATION: Exports competitive, imports expensive." << std::endl;
            std::cout << "   (Exports +, Growth +, Inflation +, Popularity -, Stability -)" << std::endl;
        }
    }
    // --- WELFARE & LABOR COMMANDS ---
    else if (command == "wage+") {
        playerCountry.welfare.minimum_wage *= 1.10;
        playerCountry.economy.inflation += 0.005;
        playerCountry.welfare.poverty_rate -= 0.01;
        if (playerCountry.welfare.poverty_rate < 0.05) playerCountry.welfare.poverty_rate = 0.05;
        playerCountry.welfare.union_strength += 0.03;
        playerCountry.politics.popularity += 0.03;
        // Can increase unemployment if wage too high
        if (playerCountry.welfare.minimum_wage > 2000.0) {
            playerCountry.welfare.unemployment_rate += 0.005;
        }
        std::cout << ">> MINIMUM WAGE RAISED to $" << (int)playerCountry.welfare.minimum_wage << std::endl;
        std::cout << "   (Poverty -, Popularity +, Inflation +, Unions +)" << std::endl;
    }
    else if (command == "healthcare+") {
        double cost = playerCountry.economy.gdp * 0.01;
        playerCountry.economy.gdp -= cost;
        playerCountry.welfare.health_coverage += 0.05;
        if (playerCountry.welfare.health_coverage > 1.0) playerCountry.welfare.health_coverage = 1.0;
        playerCountry.welfare.hospitals += 5;
        playerCountry.welfare.mental_health_index += 0.02;
        playerCountry.politics.popularity += 0.03;
        playerCountry.politics.economic_ideology -= 0.02; // Left shift (state welfare)
        if (playerCountry.politics.economic_ideology < 0.0) playerCountry.politics.economic_ideology = 0.0;
        std::cout << ">> HEALTHCARE EXPANDED: Coverage now " << (int)(playerCountry.welfare.health_coverage * 100)
                  << "%. Cost: $" << (int)(cost / 1000000.0) << "M" << std::endl;
    }
    else if (command == "healthcare-") {
        playerCountry.welfare.health_coverage -= 0.05;
        if (playerCountry.welfare.health_coverage < 0.2) playerCountry.welfare.health_coverage = 0.2;
        playerCountry.welfare.hospitals -= 3;
        if (playerCountry.welfare.hospitals < 10) playerCountry.welfare.hospitals = 10;
        playerCountry.welfare.mental_health_index -= 0.02;
        playerCountry.politics.popularity -= 0.04;
        std::cout << ">> HEALTHCARE CUT: Austerity saves money but hospitals close." << std::endl;
    }
    else if (command == "pension_reform") {
        playerCountry.welfare.pension_sustainability += 0.1;
        if (playerCountry.welfare.pension_sustainability > 1.0) playerCountry.welfare.pension_sustainability = 1.0;
        playerCountry.welfare.retirement_age += 2.0;
        playerCountry.politics.popularity -= 0.08;
        playerCountry.welfare.general_strike_prob += 0.1;
        if (playerCountry.welfare.general_strike_prob > 0.5) playerCountry.welfare.general_strike_prob = 0.5;
        std::cout << ">> PENSION REFORM: Retirement age raised to " << playerCountry.welfare.retirement_age
                  << ". Sustainability improved but massive backlash." << std::endl;
        std::cout << "   (Pension +, Popularity ---, Strike Risk +++)" << std::endl;
    }
    else if (command == "education+") {
        double cost = playerCountry.economy.gdp * 0.005;
        playerCountry.economy.gdp -= cost;
        playerCountry.welfare.educational_quality += 0.03;
        if (playerCountry.welfare.educational_quality > 1.0) playerCountry.welfare.educational_quality = 1.0;
        playerCountry.welfare.university_enrollment += 0.02;
        if (playerCountry.welfare.university_enrollment > 0.8) playerCountry.welfare.university_enrollment = 0.8;
        playerCountry.welfare.brain_drain -= 0.01;
        if (playerCountry.welfare.brain_drain < 0.01) playerCountry.welfare.brain_drain = 0.01;
        playerCountry.infra.innovation_index += 0.01;
        std::cout << ">> EDUCATION INVESTMENT: Quality improved to " << (int)(playerCountry.welfare.educational_quality * 100)
                  << "%. Cost: $" << (int)(cost / 1000000.0) << "M" << std::endl;
    }
    // --- ANTI-CORRUPTION COMMANDS ---
    else if (command == "anticorruption") {
        double cost = 15000000.0;
        playerCountry.economy.gdp -= cost;
        playerCountry.politics.anticorruption_enforcement += 0.1;
        if (playerCountry.politics.anticorruption_enforcement > 1.0) playerCountry.politics.anticorruption_enforcement = 1.0;
        playerCountry.politics.administrative_corruption -= 0.03;
        if (playerCountry.politics.administrative_corruption < 0.05) playerCountry.politics.administrative_corruption = 0.05;
        playerCountry.politics.corruption_perception_index += 3.0;
        if (playerCountry.politics.corruption_perception_index > 100.0) playerCountry.politics.corruption_perception_index = 100.0;
        playerCountry.politics.popularity += 0.03;
        // Backlash from corrupt elites
        playerCountry.politics.congressional_support -= 0.05;
        if (playerCountry.politics.congressional_support < 0.1) playerCountry.politics.congressional_support = 0.1;
        std::cout << ">> ANTI-CORRUPTION DRIVE: Enforcement strengthened. Corrupt officials nervous." << std::endl;
        std::cout << "   (Corruption -, CPI +, Popularity +, Congressional Support -)" << std::endl;
    }
    else if (command == "purge") {
        playerCountry.politics.administrative_corruption -= 0.1;
        if (playerCountry.politics.administrative_corruption < 0.05) playerCountry.politics.administrative_corruption = 0.05;
        playerCountry.politics.petty_corruption -= 0.05;
        if (playerCountry.politics.petty_corruption < 0.02) playerCountry.politics.petty_corruption = 0.02;
        playerCountry.politics.grand_corruption -= 0.03;
        if (playerCountry.politics.grand_corruption < 0.01) playerCountry.politics.grand_corruption = 0.01;
        // Institutional disruption
        playerCountry.politics.legislative_efficiency -= 0.1;
        if (playerCountry.politics.legislative_efficiency < 0.1) playerCountry.politics.legislative_efficiency = 0.1;
        playerCountry.politics.coup_d_etat_prob += 0.02; // Purged officials may plot
        playerCountry.politics.popularity += 0.05; // Public loves purges
        std::cout << ">> OFFICIAL PURGE: Corrupt officials removed. Institutional disruption." << std::endl;
        std::cout << "   (Corruption --, Efficiency -, Coup Risk +, Popularity +)" << std::endl;
    }
    else if (command == "transparency") {
        playerCountry.politics.corruption_perception_index += 5.0;
        if (playerCountry.politics.corruption_perception_index > 100.0) playerCountry.politics.corruption_perception_index = 100.0;
        playerCountry.politics.administrative_corruption -= 0.02;
        if (playerCountry.politics.administrative_corruption < 0.05) playerCountry.politics.administrative_corruption = 0.05;
        playerCountry.infra.investment_climate_index += 0.03;
        if (playerCountry.infra.investment_climate_index > 1.0) playerCountry.infra.investment_climate_index = 1.0;
        playerCountry.security.diplomatic_prestige += 0.02;
        playerCountry.economy.sovereign_wealth_fund *= 1.01; // Better SWF governance
        std::cout << ">> TRANSPARENCY REFORM: Open government, public procurement, FOI laws enacted." << std::endl;
        std::cout << "   (CPI +, Investment Climate +, Prestige +, SWF Governance +)" << std::endl;
    }
    // --- CAMPAIGN COMMAND ---
    else if (command == "campaign_spend") {
        if (!playerCountry.politics.campaign_active) {
            std::cout << ">> No active campaign. Wait for campaign season." << std::endl;
        } else {
            double cost = 20000000.0; // $20M per spending round
            if (playerCountry.economy.gdp < cost * 10) {
                std::cout << ">> INSUFFICIENT FUNDS for campaign spending." << std::endl;
            } else {
                playerCountry.economy.gdp -= cost;
                playerCountry.politics.campaign_spending += cost;
                std::cout << ">> CAMPAIGN SPENDING: $20M invested. Total: $"
                          << (int)(playerCountry.politics.campaign_spending / 1000000.0) << "M" << std::endl;
                std::cout << "   Current poll margin: " << (playerCountry.politics.poll_margin >= 0 ? "+" : "")
                          << (int)(playerCountry.politics.poll_margin * 100) << " points" << std::endl;
            }
        }
    }
    // --- AUTHORITARIAN COMMANDS ---
    else if (command == "pack_court") {
        if (playerCountry.politics.court_packing_executed) {
            std::cout << ">> Court already packed. No further changes possible." << std::endl;
        } else if (playerCountry.politics.judicial_independence > 0.7) {
            std::cout << ">> BLOCKED: Independent judiciary resists court packing attempt." << std::endl;
            playerCountry.politics.popularity -= 0.03;
            playerCountry.politics.judicial_pressure += 0.1;
        } else {
            playerCountry.politics.court_packing_executed = true;
            playerCountry.politics.judicial_independence -= 0.3;
            if (playerCountry.politics.judicial_independence < 0.1) playerCountry.politics.judicial_independence = 0.1;
            playerCountry.politics.ruling_against_state_prob -= 0.3;
            if (playerCountry.politics.ruling_against_state_prob < 0.05) playerCountry.politics.ruling_against_state_prob = 0.05;
            playerCountry.politics.democratic_backsliding_index += 0.15;
            playerCountry.politics.auth_dem_axis += 0.1;
            if (playerCountry.politics.auth_dem_axis > 1.0) playerCountry.politics.auth_dem_axis = 1.0;
            playerCountry.politics.regime_legitimacy -= 0.1;
            playerCountry.politics.authoritarian_actions_count++;
            playerCountry.economy.international_sanctions_prob += 0.05;
            std::cout << ">> COURT PACKED: Loyalist judges installed. Judicial independence destroyed." << std::endl;
            std::cout << "   (Courts -, Democracy -, Sanctions Risk +, Legitimacy -)" << std::endl;
        }
    }
    else if (command == "nationalize_media") {
        if (playerCountry.politics.media_nationalized) {
            std::cout << ">> Media already under state control." << std::endl;
        } else {
            playerCountry.politics.media_nationalized = true;
            playerCountry.security.media_control = 0.9;
            playerCountry.security.press_freedom -= 0.5;
            if (playerCountry.security.press_freedom < 0.05) playerCountry.security.press_freedom = 0.05;
            playerCountry.security.narrative_reach = 0.9;
            playerCountry.security.media_pluralism = 0.1;
            playerCountry.politics.democratic_backsliding_index += 0.15;
            playerCountry.politics.auth_dem_axis += 0.1;
            playerCountry.politics.regime_legitimacy -= 0.1;
            playerCountry.politics.authoritarian_actions_count++;
            playerCountry.economy.international_sanctions_prob += 0.05;
            std::cout << ">> MEDIA NATIONALIZED: State controls all major outlets. Dissent silenced." << std::endl;
            std::cout << "   (Control +++, Freedom ---, Democracy ---, International Outcry)" << std::endl;
        }
    }
    else if (command == "ban_opposition") {
        if (playerCountry.politics.opposition_banned) {
            std::cout << ">> Opposition already banned." << std::endl;
        } else if (playerCountry.security.press_freedom > 0.5) {
            std::cout << ">> RISKY: Free press makes opposition ban politically costly." << std::endl;
            playerCountry.politics.popularity -= 0.1;
            playerCountry.politics.opposition_banned = true;
            playerCountry.politics.opposition_seats = 0;
            playerCountry.politics.congressional_support = 1.0;
        } else {
            playerCountry.politics.opposition_banned = true;
            playerCountry.politics.opposition_seats = 0;
            playerCountry.politics.congressional_support = 1.0;
            playerCountry.politics.coalition_cohesion = 1.0;
            playerCountry.politics.democratic_backsliding_index += 0.2;
            playerCountry.politics.auth_dem_axis += 0.15;
            playerCountry.politics.regime_legitimacy -= 0.2;
            playerCountry.politics.authoritarian_actions_count++;
            playerCountry.economy.international_sanctions_prob += 0.1;
            std::cout << ">> OPPOSITION BANNED: One-party state established. No legislative resistance." << std::endl;
            std::cout << "   (Congress = 100%, Democracy -, Legitimacy --, Sanctions +++)" << std::endl;
        }
    }
    else if (command == "rig_election") {
        playerCountry.politics.election_rigged = true;
        playerCountry.politics.popularity += 0.1; // Fake boost
        playerCountry.politics.regime_legitimacy -= 0.15;
        playerCountry.politics.democratic_backsliding_index += 0.1;
        playerCountry.politics.authoritarian_actions_count++;
        // Risk of detection by international observers
        std::uniform_real_distribution<> obs_dist(0.0, 1.0);
        if (obs_dist(rng) < (1.0 - playerCountry.security.media_control) * 0.5) {
            playerCountry.economy.international_sanctions_prob += 0.1;
            playerCountry.security.diplomatic_prestige -= 0.1;
            std::cout << ">> ELECTION RIGGED: But international observers detected fraud! Massive backlash." << std::endl;
        } else {
            std::cout << ">> ELECTION RIGGED: Results manipulated. Guaranteed victory at legitimacy cost." << std::endl;
        }
    }
    // --- ENVIRONMENT COMMANDS ---
    else if (command == "protect_area") {
        double cost = 10000000.0;
        playerCountry.economy.gdp -= cost;
        playerCountry.infra.protected_area_coverage += 0.03;
        if (playerCountry.infra.protected_area_coverage > 0.5) playerCountry.infra.protected_area_coverage = 0.5;
        playerCountry.infra.biodiversity_index += 0.02;
        if (playerCountry.infra.biodiversity_index > 1.0) playerCountry.infra.biodiversity_index = 1.0;
        playerCountry.infra.deforestation_rate -= 0.002;
        if (playerCountry.infra.deforestation_rate < 0.0) playerCountry.infra.deforestation_rate = 0.0;
        // Extractive sector conflict
        playerCountry.economy.community_conflicts += 0.05;
        playerCountry.politics.extractive_sector_power -= 0.03;
        playerCountry.security.diplomatic_prestige += 0.02;
        std::cout << ">> PROTECTED AREA CREATED: " << (int)(playerCountry.infra.protected_area_coverage * 100)
                  << "% of territory under protection." << std::endl;
        std::cout << "   (Biodiversity +, Prestige +, Mining Lobby -, Cost $10M)" << std::endl;
    }
    else if (command == "pollution_control") {
        double cost = 20000000.0;
        playerCountry.economy.gdp -= cost;
        playerCountry.infra.pollution_prob -= 0.05;
        if (playerCountry.infra.pollution_prob < 0.05) playerCountry.infra.pollution_prob = 0.05;
        playerCountry.infra.air_quality_index += 0.05;
        if (playerCountry.infra.air_quality_index > 1.0) playerCountry.infra.air_quality_index = 1.0;
        playerCountry.infra.water_quality_index += 0.03;
        if (playerCountry.infra.water_quality_index > 1.0) playerCountry.infra.water_quality_index = 1.0;
        playerCountry.welfare.health_coverage += 0.01; // Less pollution = less disease
        // Industry cost
        playerCountry.politics.industrial_power -= 0.03;
        if (playerCountry.politics.industrial_power < 0.0) playerCountry.politics.industrial_power = 0.0;
        playerCountry.economy.growth_rate -= 0.002; // Short-term growth cost
        std::cout << ">> POLLUTION CONTROLS ENFORCED: Air quality " << (int)(playerCountry.infra.air_quality_index * 100)
                  << "%, Water: " << (int)(playerCountry.infra.water_quality_index * 100) << "%" << std::endl;
        std::cout << "   (Health +, Industry -, Growth -, Prestige +)" << std::endl;
    }
    else if (command == "reforest") {
        double cost = 15000000.0;
        playerCountry.economy.gdp -= cost;
        playerCountry.infra.deforestation_rate -= 0.005;
        if (playerCountry.infra.deforestation_rate < -0.01) playerCountry.infra.deforestation_rate = -0.01; // Net reforestation
        playerCountry.infra.co2_emissions *= 0.98;
        playerCountry.infra.biodiversity_index += 0.01;
        playerCountry.infra.climate_vulnerability_index -= 0.01;
        if (playerCountry.infra.climate_vulnerability_index < 0.05) playerCountry.infra.climate_vulnerability_index = 0.05;
        playerCountry.security.diplomatic_prestige += 0.01;
        std::cout << ">> REFORESTATION PROGRAM: $15M invested in forest recovery." << std::endl;
        std::cout << "   Deforestation rate: " << playerCountry.infra.deforestation_rate * 100 << "% per year." << std::endl;
        std::cout << "   (CO2 -, Biodiversity +, Climate Resilience +)" << std::endl;
    }
    // --- NEIGHBOR INTERACTION COMMANDS ---
    else if (command.substr(0, 17) == "improve_relations") {
        // improve_relations <index 0-2>
        int idx = -1;
        if (command.size() > 18) idx = std::stoi(command.substr(18));
        if (idx >= 0 && idx < (int)playerCountry.neighbors.size()) {
            auto& n = playerCountry.neighbors[idx];
            n.diplomatic_relations += 0.1;
            if (n.diplomatic_relations > 1.0) n.diplomatic_relations = 1.0;
            playerCountry.economy.gdp -= 5000000.0; // Diplomatic cost $5M
            std::cout << ">> DIPLOMACY: Relations with " << n.name << " improved to "
                      << (int)(n.diplomatic_relations * 100) << "%." << std::endl;
        } else {
            std::cout << ">> Usage: improve_relations <0-2> (0=Northland, 1=Easteria, 2=Southaven)" << std::endl;
        }
    }
    else if (command.substr(0, 16) == "worsen_relations") {
        int idx = -1;
        if (command.size() > 17) idx = std::stoi(command.substr(17));
        if (idx >= 0 && idx < (int)playerCountry.neighbors.size()) {
            auto& n = playerCountry.neighbors[idx];
            n.diplomatic_relations -= 0.15;
            if (n.diplomatic_relations < -1.0) n.diplomatic_relations = -1.0;
            std::cout << ">> HOSTILITY: Relations with " << n.name << " worsened to "
                      << (int)(n.diplomatic_relations * 100) << "%." << std::endl;
        } else {
            std::cout << ">> Usage: worsen_relations <0-2>" << std::endl;
        }
    }
    else if (command.substr(0, 10) == "trade_deal") {
        int idx = -1;
        if (command.size() > 11) idx = std::stoi(command.substr(11));
        if (idx >= 0 && idx < (int)playerCountry.neighbors.size()) {
            auto& n = playerCountry.neighbors[idx];
            if (n.diplomatic_relations > 0.0) {
                double boost = n.gdp * 0.02 * n.diplomatic_relations;
                n.trade_volume += boost;
                n.diplomatic_relations += 0.05;
                if (n.diplomatic_relations > 1.0) n.diplomatic_relations = 1.0;
                std::cout << ">> TRADE DEAL with " << n.name << ": +$" << boost / 1000000.0
                          << "M bilateral trade." << std::endl;
            } else {
                std::cout << ">> Cannot sign trade deal with hostile nation " << n.name << "." << std::endl;
            }
        } else {
            std::cout << ">> Usage: trade_deal <0-2>" << std::endl;
        }
    }
    else if (command.substr(0, 8) == "threaten") {
        int idx = -1;
        if (command.size() > 9) idx = std::stoi(command.substr(9));
        if (idx >= 0 && idx < (int)playerCountry.neighbors.size()) {
            auto& n = playerCountry.neighbors[idx];
            n.diplomatic_relations -= 0.2;
            if (n.diplomatic_relations < -1.0) n.diplomatic_relations = -1.0;
            if (playerCountry.security.military_spending_gdp > 0.03) {
                n.military_strength -= 0.02;
                if (n.military_strength < 0.1) n.military_strength = 0.1;
                std::cout << ">> THREAT: Military posturing against " << n.name
                          << ". They back down slightly." << std::endl;
            } else {
                std::cout << ">> THREAT: " << n.name << " is unimpressed by our weak military." << std::endl;
            }
            playerCountry.security.diplomatic_prestige -= 0.03;
            playerCountry.politics.threaten_streak_count++;
            if (playerCountry.politics.threaten_streak_count >= 3) {
                playerCountry.security.invasion_prob += 0.15;
                playerCountry.security.diplomatic_prestige -= 0.10;
                std::cout << "   [!] CREDIBILIDAD ROTA: amenazaste " << playerCountry.politics.threaten_streak_count
                          << " veces sin actuar. invasion_prob +15%, prestigio -10pts." << std::endl;
            }
        } else {
            std::cout << ">> Usage: threaten <0-2>" << std::endl;
        }
    }
    else if (command == "decisions") {
        if (pendingDecisions.empty()) {
            std::cout << ">> No hay decisiones pendientes." << std::endl;
        } else {
            std::cout << "\n=== DECISIONES PENDIENTES (" << pendingDecisions.size() << ") ===" << std::endl;
            for (size_t i = 0; i < pendingDecisions.size(); ++i) {
                std::cout << "[" << i << "] " << pendingDecisions[i].id
                          << " — " << pendingDecisions[i].prompt << std::endl;
                std::cout << "    Opciones: ";
                for (size_t j = 0; j < pendingDecisions[i].options.size(); ++j) {
                    std::cout << pendingDecisions[i].options[j];
                    if (j + 1 < pendingDecisions[i].options.size()) std::cout << " | ";
                }
                std::cout << std::endl;
            }
            std::cout << "=================================" << std::endl;
        }
    }
    else if (command == "skip_decision") {
        if (pendingDecisions.empty()) {
            std::cout << ">> No hay decisión que saltar." << std::endl;
        } else {
            PendingDecision d = pendingDecisions.front();
            pendingDecisions.erase(pendingDecisions.begin());
            pendingDecisions.push_back(d);
            playerCountry.politics.popular_pressure += 0.02;
            std::cout << ">> Decisión '" << d.id << "' pospuesta al final de la cola (-credibilidad)." << std::endl;
        }
    }
    else if (command == "save") {
        saveGame("savegame.txt");
    }
    else if (command == "load") {
        loadGame("savegame.txt");
    }
    else if (command == "help") {
        std::cout << "\n=== FAMILIAS DE COMANDOS ===" << std::endl;
        std::cout << "Turno:        next, exit" << std::endl;
        std::cout << "Resumen:      help, status_brief" << std::endl;
        std::cout << "Fiscal:       tax+, tax-, wage-, retire+, retire-, invest_health/security/infra/education" << std::endl;
        std::cout << "Recursos:     mining+, mining-, mining_reform, royalty+, royalty-, geo_survey, mine_rehab" << std::endl;
        std::cout << "SWF:          swf_save, swf_rate-, swf_cancel, swf_rule, swf_spend, swf_invest, swf_debt," << std::endl;
        std::cout << "              swf_conservative, swf_balanced, swf_growth, swf_transparency" << std::endl;
        std::cout << "Comercio:     tariff+, tariff-, tariff_dumping, fta_sign, hedge_prices" << std::endl;
        std::cout << "Ambiental:    env_bond, env_audit, consult, mediate, suppress_conflict" << std::endl;
        std::cout << "Monetario:    autonomy+, autonomy-, interest+, interest-, print+, reform_currency" << std::endl;
        std::cout << "Política:     worship+/-, torture+/-, disappear+/-, press+/-, minority+/-" << std::endl;
        std::cout << "Diplomacia:   diplomacy+, diplomacy-" << std::endl;
        std::cout << "Vecinos:      improve_relations/worsen_relations/trade_deal/threaten <0-2>, negotiate_peace_neighbor" << std::endl;
        std::cout << "Escándalos:   cover_up, scapegoat, counter_narrative, apologize" << std::endl;
        std::cout << "============================" << std::endl;
    }
    else if (command == "status_brief") {
        auto& pol = playerCountry.politics;
        auto& eco = playerCountry.economy;
        std::cout << "\n--- STATUS BRIEF (Turno " << turnCount << ") ---" << std::endl;
        std::cout << "Popularidad: " << (int)(pol.popularity * 100) << "%"
                  << " | GDP: $" << (long long)(eco.gdp / 1000000) << "M"
                  << " | Inflación: " << (int)(eco.inflation * 100) << "%" << std::endl;
        std::cout << "Presiones — Cong: " << (int)(pol.congressional_pressure * 100) << "%"
                  << " Jud: " << (int)(pol.judicial_pressure * 100) << "%"
                  << " Mil: " << (int)(pol.military_pressure * 100) << "%"
                  << " Pop: " << (int)(pol.popular_pressure * 100) << "%"
                  << " Intl: " << (int)(pol.international_pressure * 100) << "%" << std::endl;
        std::cout << "Escándalos activos: " << pol.active_scandals
                  << " | Legitimidad: " << (int)(pol.regime_legitimacy * 100) << "%"
                  << " | Backsliding: " << (int)(pol.democratic_backsliding_index * 100) << "%" << std::endl;
        std::string events;
        if (playerCountry.welfare.pandemic_active) events += "pandemia ";
        if (playerCountry.welfare.general_strike_active) events += "huelga ";
        if (eco.in_recession) events += "recesión ";
        if (playerCountry.security.war_active) events += "guerra ";
        if (playerCountry.security.migration_crisis_active) events += "migración ";
        if (pol.civil_war_active) events += "guerra-civil ";
        if (playerCountry.infra.blackout_active) events += "apagón ";
        if (events.empty()) events = "ninguno";
        std::cout << "Eventos activos: " << events << std::endl;
        std::cout << "Decisiones pendientes: " << pendingDecisions.size() << std::endl;
        std::cout << "----------------------------------" << std::endl;
    }
    else if (command == "cover_up") {
        double cost = playerCountry.politics.lobbying_cost * 10.0;
        if (playerCountry.economy.tax_collection < cost) {
            std::cout << ">> COVER-UP: presupuesto insuficiente." << std::endl;
        } else {
            playerCountry.economy.tax_collection -= cost;
            std::uniform_real_distribution<double> roll(0.0, 1.0);
            if (roll(rng) < playerCountry.politics.cover_up_probability) {
                playerCountry.politics.scandal_corruption_severity *= 0.4;
                playerCountry.politics.scandal_sex_severity *= 0.4;
                playerCountry.politics.scandal_financial_severity *= 0.4;
                playerCountry.security.media_control += 0.05;
                std::cout << ">> COVER-UP exitoso: severidades reducidas, control mediático sube." << std::endl;
            } else {
                playerCountry.politics.judicial_pressure += 0.15;
                playerCountry.politics.media_exposure_intensity += 0.2;
                std::cout << ">> COVER-UP fracasa: la justicia y la prensa intensifican." << std::endl;
            }
        }
    }
    else if (command == "scapegoat") {
        auto& pol = playerCountry.politics;
        double* worst = &pol.scandal_corruption_severity;
        if (pol.scandal_sex_severity > *worst) worst = &pol.scandal_sex_severity;
        if (pol.scandal_financial_severity > *worst) worst = &pol.scandal_financial_severity;
        if (pol.scandal_violence_severity > *worst) worst = &pol.scandal_violence_severity;
        if (pol.scandal_substance_severity > *worst) worst = &pol.scandal_substance_severity;
        if (*worst > 0.0) {
            *worst = 0.0;
            if (pol.active_scandals > 0) pol.active_scandals--;
            pol.popularity -= 0.05;
            pol.polarization_index += 0.1;
            std::cout << ">> CHIVO EXPIATORIO: un escándalo desactivado, popularidad y polarización pagan el precio." << std::endl;
        } else {
            std::cout << ">> No hay escándalos activos." << std::endl;
        }
    }
    else if (command == "counter_narrative") {
        double effect = playerCountry.security.narrative_reach * 0.5;
        playerCountry.politics.media_exposure_intensity -= effect;
        if (playerCountry.politics.media_exposure_intensity < 0.0) playerCountry.politics.media_exposure_intensity = 0.0;
        playerCountry.security.narrative_reach -= 0.02;
        std::cout << ">> CONTRA-NARRATIVA: la agenda mediática se desvía." << std::endl;
    }
    else if (command == "apologize") {
        auto& pol = playerCountry.politics;
        if (pol.apologize_cooldown_turns > 0) {
            std::cout << ">> APOLOGIZE bloqueado: tu ultima disculpa fue demasiado reciente." << std::endl;
            std::cout << "   Espera " << pol.apologize_cooldown_turns << " turnos para que el publico vuelva a creerte." << std::endl;
        } else {
            pol.popularity -= 0.05;
            pol.scandal_corruption_severity *= 0.8;
            pol.scandal_sex_severity *= 0.8;
            pol.scandal_financial_severity *= 0.8;
            pol.scandal_violence_severity *= 0.8;
            pol.scandal_substance_severity *= 0.8;
            pol.polarization_index -= 0.1;
            if (pol.polarization_index < 0.0) pol.polarization_index = 0.0;
            pol.regime_legitimacy += 0.03;
            pol.apologize_cooldown_turns = 3;
            std::cout << ">> DISCULPAS PUBLICAS: costo de imagen, ganancia de legitimidad. Cooldown 3 turnos." << std::endl;
        }
    }
    else if (command == "negotiate_peace_neighbor") {
        bool any_war = false;
        for (auto& n : playerCountry.neighbors) {
            if (n.at_war) {
                n.at_war = false;
                n.diplomatic_relations = -0.4;
                n.trade_volume = n.gdp * 0.03;
                n.sanctions_against_us = false;
                playerCountry.security.war_active = false;
                playerCountry.security.diplomatic_prestige -= 0.05;
                std::cout << ">> PEACE NEGOTIATION: Ceasefire with " << n.name << ". Relations remain tense." << std::endl;
                any_war = true;
            }
        }
        if (!any_war) std::cout << ">> No active wars with neighbors." << std::endl;
    }
    else if (command == "leader") {
        std::string path;
        std::cin >> path;
        if (path.empty()) {
            std::cout << ">> Usage: leader <path-to-yaml>" << std::endl;
            std::cout << "   Ej: leader ../homo-politicus-game/content/leaders/peron.yaml" << std::endl;
            return;
        }
        LeaderLoader::Metadata meta;
        if (LeaderLoader::load(path, playerCountry, meta)) {
            std::cout << ">> LIDER CARGADO: " << meta.name_es
                      << " (" << meta.country_origin << ")" << std::endl;
            std::cout << "   Modificadores aplicados al estado actual." << std::endl;
        } else {
            std::cout << ">> LEADER: no se pudo cargar " << path << std::endl;
        }
    }
    else if (command == "scenario") {
        std::string path;
        std::cin >> path;
        if (path.empty()) {
            std::cout << ">> Usage: scenario <path-to-yaml>" << std::endl;
            std::cout << "   Ej: scenario ../homo-politicus-game/content/scenarios/argentina_1976.yaml" << std::endl;
            return;
        }
        ScenarioLoader::Metadata meta;
        if (ScenarioLoader::load(path, playerCountry, meta)) {
            std::cout << ">> ESCENARIO CARGADO: " << meta.name_es
                      << " (año " << meta.start_year << ", dificultad " << meta.difficulty << ")" << std::endl;
            std::cout << "   Estado inicial aplicado. Comenzá tu mandato con 'next'." << std::endl;
            turnCount = 0;
            popularitySum = 0.0;
            endCondition = EndCondition::NONE;
            initialGdp = playerCountry.economy.gdp;
        } else {
            std::cout << ">> SCENARIO: no se pudo cargar " << path << std::endl;
        }
    }
    else {
        std::cout << ">> Unknown command. Tipea 'help' para ver familias." << std::endl;
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

    // 1. Labor Factor: workforce_participation, human_capital y labor_factor
    //    removidos hasta que el growth model los consuma.

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
    // internal_market_strength removido (no consumido).

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

    // --- RECESSION STATE ---
    if (playerCountry.economy.growth_rate < 0) {
        playerCountry.economy.recession_quarters++;
        if (playerCountry.economy.recession_quarters >= 2 && !playerCountry.economy.in_recession) {
            // Formal recession declaration (2+ consecutive negative turns)
            playerCountry.economy.in_recession = true;
            playerCountry.economy.pre_recession_gdp = playerCountry.economy.gdp / (1.0 + playerCountry.economy.growth_rate);
            playerCountry.economy.recession_depth = 0.0;
            std::cout << "[!!!] RECESSION DECLARED: Economy contracting for " << playerCountry.economy.recession_quarters
                      << " consecutive turns." << std::endl;
        }
        if (playerCountry.economy.in_recession) {
            playerCountry.economy.recession_depth += std::abs(playerCountry.economy.growth_rate);

            // Credit rating pressure
            // (handled by existing credit rating logic, but add extra pressure)
            playerCountry.economy.debt_to_gdp_ratio += 0.005; // Deficit spending

            // Capital flight
            playerCountry.economy.international_reserves -= playerCountry.economy.international_reserves * 0.02;

            // Unemployment spikes
            playerCountry.welfare.unemployment_rate += 0.01;
            if (playerCountry.welfare.unemployment_rate > 0.30)
                playerCountry.welfare.unemployment_rate = 0.30;

            // Pension system stress
            playerCountry.welfare.pension_sustainability -= 0.02;
            if (playerCountry.welfare.pension_sustainability < 0.0)
                playerCountry.welfare.pension_sustainability = 0.0;

            // Popularity tanks
            playerCountry.politics.popularity -= 0.03;

            std::cout << "[!!!] RECESSION (Quarter " << playerCountry.economy.recession_quarters
                      << "): Depth " << (int)(playerCountry.economy.recession_depth * 100)
                      << "%. Unemployment: " << (int)(playerCountry.welfare.unemployment_rate * 100)
                      << "%" << std::endl;
        }
    } else {
        if (playerCountry.economy.in_recession) {
            // Need 2 consecutive positive turns to exit recession
            if (playerCountry.economy.recession_quarters <= -1) {
                playerCountry.economy.in_recession = false;
                playerCountry.economy.recession_quarters = 0;
                playerCountry.economy.recession_depth = 0.0;
                std::cout << "[INFO] RECESSION OVER: Economy officially in recovery." << std::endl;
            } else {
                playerCountry.economy.recession_quarters = -1; // First positive turn
            }
        } else {
            playerCountry.economy.recession_quarters = 0;
        }
    }

    // --- CROSS-SYSTEM FEEDBACK: ECONOMIC → SOCIAL ---
    {
        // GDP decline drives social deterioration
        if (playerCountry.economy.growth_rate < 0) {
            double decline_mag = std::abs(playerCountry.economy.growth_rate);
            // Unemployment rises proportional to GDP decline (Okun's law feedback)
            playerCountry.welfare.unemployment_rate += decline_mag * 0.3;
            // Poverty worsens
            playerCountry.welfare.poverty_rate += decline_mag * 0.2;
            if (playerCountry.welfare.poverty_rate > 0.8) playerCountry.welfare.poverty_rate = 0.8;
            // Mental health deteriorates
            playerCountry.welfare.mental_health_index -= decline_mag * 0.1;
            if (playerCountry.welfare.mental_health_index < 0.1) playerCountry.welfare.mental_health_index = 0.1;
            // Suicide rate rises
            playerCountry.welfare.suicide_rate += decline_mag * 0.00005;
        } else if (playerCountry.economy.growth_rate > 0.03) {
            // Growth boom: social improvement
            double boom_mag = playerCountry.economy.growth_rate - 0.03;
            playerCountry.welfare.unemployment_rate -= boom_mag * 0.2;
            if (playerCountry.welfare.unemployment_rate < 0.02) playerCountry.welfare.unemployment_rate = 0.02;
            playerCountry.welfare.poverty_rate -= boom_mag * 0.1;
            if (playerCountry.welfare.poverty_rate < 0.05) playerCountry.welfare.poverty_rate = 0.05;
            playerCountry.economy.tax_collection *= (1.0 + boom_mag * 0.5);
            playerCountry.politics.popularity += boom_mag * 0.5;
        }

        // High inflation erodes living standards
        if (playerCountry.economy.inflation > 0.1) {
            double infl_excess = playerCountry.economy.inflation - 0.1;
            playerCountry.welfare.poverty_rate += infl_excess * 0.15;
            if (playerCountry.welfare.poverty_rate > 0.8) playerCountry.welfare.poverty_rate = 0.8;
            playerCountry.politics.protest_intensity += infl_excess * 0.1;
            playerCountry.politics.popularity -= infl_excess * 0.3;
        }

        // Brain drain accelerates during economic hardship
        if (playerCountry.welfare.unemployment_rate > 0.15 || playerCountry.economy.in_recession) {
            playerCountry.welfare.brain_drain += 0.005;
            if (playerCountry.welfare.brain_drain > 0.3) playerCountry.welfare.brain_drain = 0.3;
        }
    }

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

    // --- MINING & NATURAL RESOURCES (Resource Curse vs. Fiscal Windfall) ---

    // === COMMODITY PRICE MODEL (4 layers) ===

    // Layer 1 — SHORT CYCLE (~21 years, offset from GDP sine to avoid lock-step)
    double short_cycle = 0.28 * sin(turnCount * 0.3 + 1.5);

    // Layer 2 — SUPER-CYCLE (Markov regime model, 30-40 year structural waves)
    // Two regimes: Bull (demand-driven expansion) and Bear (oversupply contraction).
    // Regimes are persistent but not permanent — transition probability increases with age.
    double prev_supercycle = playerCountry.economy.commodity_supercycle;

    {
        std::uniform_real_distribution<> roll(0.0, 1.0);
        playerCountry.economy.supercycle_years++;

        if (playerCountry.economy.supercycle_bull) {
            // Bull → Bear: base 8%, rises after 15 years (nothing lasts forever)
            double p_turn = 0.08 + (playerCountry.economy.supercycle_years > 15 ? 0.06 : 0.0);
            if (roll(rng) < p_turn) {
                playerCountry.economy.supercycle_bull = false;
                playerCountry.economy.supercycle_years = 0;
                std::cout << "[SUPERCYCLE] REGIME SHIFT: Global commodity bull cycle ending. "
                          << "Oversupply and demand contraction begin." << std::endl;
            }
        } else {
            // Bear → Bull: base 12%, rises after 10 years (lows sow the seeds of recovery)
            double p_turn = 0.12 + (playerCountry.economy.supercycle_years > 10 ? 0.07 : 0.0);
            if (roll(rng) < p_turn) {
                playerCountry.economy.supercycle_bull = true;
                playerCountry.economy.supercycle_years = 0;
                std::cout << "[SUPERCYCLE] REGIME SHIFT: New commodity bull cycle starting. "
                          << "Global demand surge expected." << std::endl;
            }
        }

        // Regime-directed drift: trend follows current phase with noise
        std::normal_distribution<> noise(0.0, 0.018);
        double regime_drift = playerCountry.economy.supercycle_bull ? +0.014 : -0.014;
        playerCountry.economy.commodity_supercycle += regime_drift + noise(rng);

        // --- CAUSAL DRIVERS ---
        // Strong global growth reinforces bull demand
        if (global_growth_trend > 0.035 && playerCountry.economy.supercycle_bull)
            playerCountry.economy.commodity_supercycle += 0.010;

        // Renewables penetration creates structural bear pressure on fossil/traditional commodities
        if (playerCountry.infra.renewables_percentage > 0.35)
            playerCountry.economy.commodity_supercycle -= 0.015;

        // Geopolitical tension → supply fear → bull signal (regardless of demand)
        if (playerCountry.economy.international_sanctions_prob > 0.30)
            playerCountry.economy.commodity_supercycle += 0.008;

        // Tech disruption (high innovation → new materials displace traditional ore)
        if (playerCountry.infra.innovation_index > 0.70 && !playerCountry.economy.supercycle_bull)
            playerCountry.economy.commodity_supercycle -= 0.010;

        // Gentle mean reversion prevents runaway extremes
        playerCountry.economy.commodity_supercycle *= 0.97;

        // Hard bounds
        if (playerCountry.economy.commodity_supercycle >  0.45) playerCountry.economy.commodity_supercycle =  0.45;
        if (playerCountry.economy.commodity_supercycle < -0.45) playerCountry.economy.commodity_supercycle = -0.45;
    }

    // --- THRESHOLD CROSSING EVENTS ---
    if (prev_supercycle <  0.15 && playerCountry.economy.commodity_supercycle >=  0.15)
        std::cout << "[SUPERCYCLE] Industrial demand surge: commodity markets entering sustained bull territory." << std::endl;
    if (prev_supercycle <  0.30 && playerCountry.economy.commodity_supercycle >=  0.30)
        std::cout << "[SUPERCYCLE] SUPER-BOOM: Resource nationalism rising worldwide. Exceptional revenues likely." << std::endl;
    if (prev_supercycle > -0.15 && playerCountry.economy.commodity_supercycle <= -0.15)
        std::cout << "[SUPERCYCLE] Global industrial slowdown: commodity markets entering bear territory." << std::endl;
    if (prev_supercycle > -0.30 && playerCountry.economy.commodity_supercycle <= -0.30)
        std::cout << "[SUPERCYCLE] SUPER-BUST: Resource economies entering structural fiscal crisis." << std::endl;

    // Layer 3 — GLOBAL DEMAND (coupled to the existing GDP cycle)
    // Commodity demand follows global growth: recessions crush prices, booms inflate them.
    double global_demand_effect = (global_growth_trend - 0.025) * 3.0; // ±~6% effect

    // Layer 4 — RANDOM SHOCKS (stochastic events, independent each year)
    double price_shock = 0.0;
    {
        std::uniform_real_distribution<> roll(0.0, 1.0);
        std::uniform_real_distribution<> mag(0.0, 1.0);

        if (roll(rng) < 0.08) {
            // Geopolitical supply disruption (war, embargo, cartel cut)
            double spike = 0.15 + mag(rng) * 0.20;
            price_shock += spike;
            std::cout << "[COMMODITY] SUPPLY SHOCK: Geopolitical disruption sends prices up +"
                      << (int)(spike * 100) << "%." << std::endl;
        }
        if (roll(rng) < 0.05) {
            // Major deposit discovered elsewhere → global glut
            double crash = 0.10 + mag(rng) * 0.15;
            price_shock -= crash;
            std::cout << "[COMMODITY] GLUT: Large deposits found elsewhere. Prices fall -"
                      << (int)(crash * 100) << "%." << std::endl;
        }
        if (roll(rng) < 0.04 && playerCountry.infra.innovation_index > 0.55) {
            // Tech substitution: synthetic materials or green energy displaces traditional ore
            double disruption = 0.12 + mag(rng) * 0.13;
            price_shock -= disruption;
            std::cout << "[COMMODITY] TECH DISRUPTION: New materials substitute traditional mining outputs. -"
                      << (int)(disruption * 100) << "%." << std::endl;
        }
    }

    // Sanctions discount: isolated sellers must accept lower prices from fewer buyers
    double sanctions_discount = 0.0;
    if (playerCountry.economy.international_sanctions_prob > 0.5)
        sanctions_discount = -0.12;

    // Compose raw target price
    double price_target = 1.0
                        + short_cycle
                        + playerCountry.economy.commodity_supercycle
                        + global_demand_effect
                        + price_shock
                        + sanctions_discount;

    if (price_target < 0.30) price_target = 0.30;
    if (price_target > 2.20) price_target = 2.20;

    // Price momentum: markets adjust gradually, not in a single jump (40/60 smoothing)
    if (playerCountry.economy.commodity_hedge_turns > 0) {
        playerCountry.economy.commodity_hedge_turns--;
        std::cout << "[HEDGE] Prices locked at " << playerCountry.economy.commodity_prices
                  << "x (" << playerCountry.economy.commodity_hedge_turns << " year(s) remaining)." << std::endl;
        // price stays unchanged — hedge absorbs the shock
    } else {
        playerCountry.economy.commodity_prices = 0.55 * playerCountry.economy.commodity_prices
                                               + 0.45 * price_target;
    }

    // --- RESOURCE DEPLETION (Dynamic Rate) ---
    // Base: 0.4%/concession/year. Multiple factors accelerate or slow consumption.
    double depletion_rate = playerCountry.economy.mining_concessions * 0.004;

    // Price boom → companies rush to extract while margins are wide
    if (playerCountry.economy.commodity_prices > 1.2)
        depletion_rate += playerCountry.economy.mining_concessions * 0.002;

    // Overcrowding (> 8 concessions): competitive over-extraction depletes shared reserves faster
    if (playerCountry.economy.mining_concessions > 8)
        depletion_rate += (playerCountry.economy.mining_concessions - 8) * 0.003;

    // Technology improves recovery rates and reduces waste
    if (playerCountry.welfare.research_spending_gdp > 0.02)
        depletion_rate *= 0.80;

    playerCountry.economy.resource_depletion += depletion_rate;
    if (playerCountry.economy.resource_depletion > 1.0) playerCountry.economy.resource_depletion = 1.0;

    // --- EXTRACTION LIFECYCLE (Phase-Based Yield) ---
    // Real extraction follows a bell curve: ramp-up → peak → decline.
    // Yield factor replaces the naive (1 - depletion) formula.
    double dep = playerCountry.economy.resource_depletion;
    double yield_factor;
    if (dep < 0.30) {
        // Phase 1 — RAMP-UP: infrastructure building, improving ore access
        yield_factor = 0.85 + (dep / 0.30) * 0.15;   // 0.85 → 1.0
    } else if (dep < 0.65) {
        // Phase 2 — PEAK: optimal grade, established operations
        yield_factor = 1.0;
    } else {
        // Phase 3 — DECLINE: ore grade falls, costs soar (deeper mines, lower purity)
        // Quadratic collapse: small early decline, rapid fall near exhaustion
        double past_peak = (dep - 0.65) / 0.35;       // 0 → 1 as dep goes 65% → 100%
        yield_factor = 1.0 - (past_peak * past_peak);  // 1.0 → 0.0 (quadratic)
    }
    if (yield_factor < 0.0) yield_factor = 0.0;

    // --- ROYALTY SYSTEM ---
    // Gross extraction value: what the ground is worth before state share.
    // Base $3.33M/concession (at 15% royalty → $500k to state).
    double extraction_value = playerCountry.economy.mining_concessions
                            * 3333333.0
                            * playerCountry.economy.commodity_prices
                            * yield_factor;

    // --- SUPERCYCLE SYSTEMIC EFFECTS ---
    // The supercycle phase reshapes the broader economy beyond price levels.
    if (playerCountry.economy.mining_concessions > 0) {
        double sc = playerCountry.economy.commodity_supercycle;

        if (sc > 0.15) {
            // Bull: foreign capital floods into the resource sector
            double fdi_bonus = sc * extraction_value * 0.015;
            playerCountry.economy.international_reserves += fdi_bonus;
            if (sc > 0.28) {
                // Deep bull: mining lobby crowds out manufacturing investment
                playerCountry.politics.industrial_power -= 0.012; // net Dutch Disease effect
            }
        }

        if (sc < -0.15) {
            // Bear: companies cut corners — environmental standards slip
            playerCountry.infra.pollution_prob += 0.004;
            if (playerCountry.infra.pollution_prob > 1.0) playerCountry.infra.pollution_prob = 1.0;

            if (sc < -0.28) {
                // Deep bear: structural unemployment in mining towns
                playerCountry.welfare.unemployment_rate  += 0.004;
                playerCountry.politics.polarization_index += 0.008;
                // Desperate companies bribe officials more aggressively
                playerCountry.politics.administrative_corruption += 0.005;
                if (playerCountry.economy.supercycle_years > 5)
                    std::cout << "[SUPERCYCLE] Extended bear: mining towns in distress. "
                              << "Unemployment and corruption rising." << std::endl;
            }
        }
    }

    // Corruption leakage: officials pocket part of what companies owe the state.
    // At 50% corruption, state collects only 70% of its due (30% evaded via bribes).
    double effective_royalty_rate = playerCountry.economy.royalty_rate
                                  * (1.0 - playerCountry.politics.administrative_corruption * 0.6);
    if (effective_royalty_rate < 0.0) effective_royalty_rate = 0.0;

    double royalty_yield     = extraction_value * effective_royalty_rate;
    double royalty_evasion   = extraction_value
                             * (playerCountry.economy.royalty_rate - effective_royalty_rate);
    playerCountry.economy.state_royalties = royalty_yield;

    // Windfall tax: during commodity booms the state captures extra surplus automatically.
    // Proportional to how far above 1.3x the price has gone. Angers industry lobby.
    double windfall_tax = 0.0;
    if (playerCountry.economy.commodity_prices > 1.3 && playerCountry.economy.mining_concessions > 0) {
        windfall_tax = extraction_value * 0.10 * (playerCountry.economy.commodity_prices - 1.3);
        royalty_yield                        += windfall_tax;
        playerCountry.economy.state_royalties += windfall_tax;
        playerCountry.politics.industrial_power -= 0.02;
        std::cout << "[INFO] WINDFALL TAX: Commodity boom surplus captured. +$"
                  << windfall_tax / 1000000.0 << "M (Industry lobby angered)." << std::endl;
    }

    // Royalties flow directly into government revenue (non-tax fiscal income).
    playerCountry.economy.tax_collection += royalty_yield;

    // Investment attractiveness: high royalty rates deter new mining FDI.
    // > 35% → companies threaten capital flight. < 10% → companies over-extract.
    if (playerCountry.economy.royalty_rate > 0.35) {
        playerCountry.politics.industrial_power -= 0.02;
        if (playerCountry.economy.mining_concessions > 0) {
            std::cout << "[!] MINING CAPITAL FLIGHT: High royalty rate deters foreign investment in extraction." << std::endl;
        }
    } else if (playerCountry.economy.royalty_rate < 0.10) {
        playerCountry.economy.resource_depletion += 0.01; // Companies accelerate extraction
        if (playerCountry.economy.mining_concessions > 0) {
            std::cout << "[INFO] LOW ROYALTIES: Companies accelerating extraction. Depletion increases faster." << std::endl;
        }
    }

    // Mining adds industrial CO2 and pollution.
    playerCountry.infra.co2_emissions += playerCountry.economy.mining_concessions * 50.0;
    if (playerCountry.economy.mining_concessions > 5) {
        playerCountry.infra.pollution_prob += 0.01 * (playerCountry.economy.mining_concessions - 5);
        if (playerCountry.infra.pollution_prob > 1.0) playerCountry.infra.pollution_prob = 1.0;
    }

    // --- COMMUNITY CONFLICTS (Escalation Model) ---

    // DRIVERS (what fuels conflict):
    double conflict_pressure = 0.0;
    // Raw extraction intensity
    conflict_pressure += playerCountry.economy.mining_concessions * 0.07;
    // Environmental damage: pollution and CO2 contaminate land and water
    conflict_pressure += playerCountry.infra.pollution_prob * 0.25;
    if (playerCountry.infra.co2_emissions > 2000.0) conflict_pressure += 0.04;
    // Poverty makes communities desperate — they have nothing to lose
    conflict_pressure += playerCountry.welfare.poverty_rate * 0.25;
    // Fast depletion signals companies over-extracting (abuse of resources)
    conflict_pressure += playerCountry.economy.resource_depletion * 0.12;
    // Corruption kills trust in any negotiated settlement
    conflict_pressure += playerCountry.politics.administrative_corruption * 0.20;

    // MITIGATORS (what keeps communities at the table):
    // Minority/indigenous rights recognition → communities have legal channels
    conflict_pressure -= playerCountry.welfare.minority_protection * 0.20;
    // Fair royalty rate → communities see tangible benefit
    conflict_pressure -= playerCountry.economy.royalty_rate * 0.10;
    // Strong courts → disputes resolved legally, not on the road
    conflict_pressure -= playerCountry.politics.judicial_independence * 0.15;
    // Revenue sharing already active
    if (playerCountry.economy.swf_deposit_rate > 0) conflict_pressure -= 0.05;

    double target_conflicts = conflict_pressure;
    if (target_conflicts < 0.0) target_conflicts = 0.0;
    if (target_conflicts > 1.0) target_conflicts = 1.0;
    playerCountry.economy.community_conflicts +=
        (target_conflicts - playerCountry.economy.community_conflicts) * 0.2;

    // --- ESCALATION STAGES ---
    {
        std::uniform_real_distribution<> dist(0.0, 1.0);
        double cc = playerCountry.economy.community_conflicts;

        if (cc > 0.8) {
            // STAGE 4: INSURGENCY — armed self-defense groups form
            std::cout << "[!!!] MINING WAR: Armed self-defense groups forming in extraction zones." << std::endl;
            playerCountry.economy.gdp                 *= 0.97;
            playerCountry.politics.popularity         -= 0.05;
            playerCountry.politics.polarization_index += 0.05;
            playerCountry.welfare.radicalism_prob     += 0.04;
            playerCountry.security.homicide_rate      += 3.0;
            playerCountry.welfare.un_score            -= 0.05;
            // Armed groups may form or grow
            if (dist(rng) < 0.35) {
                playerCountry.security.non_state_groups += 1;
                std::cout << "         New armed self-defense group registered in extraction region." << std::endl;
            }
            // Companies may abandon some operations under threat
            if (dist(rng) < 0.40 && playerCountry.economy.mining_concessions > 0) {
                int halted = playerCountry.economy.mining_concessions / 3 + 1;
                playerCountry.economy.mining_concessions -= halted;
                if (playerCountry.economy.mining_concessions < 0) playerCountry.economy.mining_concessions = 0;
                std::cout << "         " << halted << " concession(s) ABANDONED by companies fleeing violence." << std::endl;
            }
        } else if (cc > 0.6) {
            // STAGE 3: OPEN CONFRONTATION — state forces deployed
            std::cout << "[!] MINING CONFRONTATION: State forces deployed. Operations disrupted." << std::endl;
            playerCountry.economy.gdp                 *= 0.99;
            playerCountry.politics.popularity         -= 0.03;
            playerCountry.politics.polarization_index += 0.03;
            playerCountry.welfare.radicalism_prob     += 0.02;
            playerCountry.security.homicide_rate      += 2.0;
            playerCountry.welfare.un_score            -= 0.03;
            playerCountry.infra.potable_water_access  -= 0.005; // Contamination during clashes
            if (playerCountry.infra.potable_water_access < 0.0) playerCountry.infra.potable_water_access = 0.0;
            playerCountry.politics.blockades          += 2;
        } else if (cc > 0.4) {
            // STAGE 2: ACTIVE CONFLICT — blockades, supply route disruptions
            std::cout << "[!] MINING CONFLICT: Communities blockading access roads and supply routes." << std::endl;
            playerCountry.politics.popularity     -= 0.02;
            playerCountry.security.homicide_rate  += 1.0;
            playerCountry.welfare.un_score        -= 0.01;
            playerCountry.politics.marches        += 2;
            playerCountry.politics.blockades      += 1;
        } else if (cc > 0.2) {
            // STAGE 1: LATENT TENSION — organizing and legal challenges
            std::cout << "[!] MINING TENSIONS: Communities organizing against extraction operations." << std::endl;
            playerCountry.politics.popularity -= 0.01;
            playerCountry.politics.marches    += 1;
        }
    }

    // --- EXTRACTION PHASE MESSAGES ---
    if (playerCountry.economy.mining_concessions > 0) {
        if (dep >= 0.65 && dep < 0.75) {
            std::cout << "[!] MINING DECLINE: Reserves past peak. Ore grade dropping, costs rising." << std::endl;
        } else if (dep >= 0.75 && dep < 0.90) {
            std::cout << "[!] LATE-STAGE MINING: Only marginal deposits remain. Consider geo_survey or diversification." << std::endl;
        } else if (dep >= 0.90) {
            std::cout << "[!!!] RESERVES EXHAUSTED: Extraction economically marginal. Revenues in freefall." << std::endl;
            // Companies begin spontaneous withdrawal — unprofitable to operate
            std::uniform_real_distribution<> d(0.0, 1.0);
            if (d(rng) < 0.20 && playerCountry.economy.mining_concessions > 0) {
                playerCountry.economy.mining_concessions -= 1;
                std::cout << "         One company abandoned its concession (uneconomic to continue)." << std::endl;
            }
        }
    }

    // --- ENVIRONMENTAL LEGACY (Tailings & Contamination) ---
    // Tailings accumulate from late-stage extraction. Contamination persists for decades
    // after mines close — affecting water, health, diplomacy, and social stability.
    if (dep > 0.5 && playerCountry.economy.mining_concessions > 0) {
        // More concessions + deeper depletion = faster contamination
        double tailings = (dep - 0.5) * playerCountry.economy.mining_concessions * 0.003;
        // Weak environmental regulation (low judicial independence) → faster accumulation
        double oversight_factor = 1.0 + (1.0 - playerCountry.politics.judicial_independence) * 0.4;
        tailings *= oversight_factor;
        playerCountry.economy.mining_legacy_damage += tailings;
        if (playerCountry.economy.mining_legacy_damage > 1.0) playerCountry.economy.mining_legacy_damage = 1.0;
        // Spatial and financial dimensions grow with each tailings pulse
        playerCountry.economy.contaminated_area_km2 += tailings * playerCountry.economy.mining_concessions * 40.0;
        playerCountry.economy.remediation_cost = playerCountry.economy.contaminated_area_km2 * 1800000.0; // ~$1.8M/km²
        if (playerCountry.economy.mining_legacy_damage >= 0.50)
            playerCountry.economy.superfund_sites = (int)((playerCountry.economy.mining_legacy_damage - 0.50) / 0.25) + 1;
        else
            playerCountry.economy.superfund_sites = 0;
    }

    {
        double legacy = playerCountry.economy.mining_legacy_damage;

        // Natural slow decay only when: mines are closed AND damage is low
        if (legacy > 0.0 && legacy < 0.15 && playerCountry.economy.mining_concessions == 0) {
            playerCountry.economy.mining_legacy_damage -= 0.002; // Bioremediation / rainfall flushing
            if (playerCountry.economy.mining_legacy_damage < 0.0) playerCountry.economy.mining_legacy_damage = 0.0;
            playerCountry.economy.contaminated_area_km2 *= 0.97; // Natural area recovery ~3%/yr
            playerCountry.economy.remediation_cost = playerCountry.economy.contaminated_area_km2 * 1800000.0;
        }

        // --- TIER 1: LATENT CONTAMINATION (0.10 – 0.25) ---
        if (legacy > 0.10) {
            playerCountry.infra.potable_water_access -= legacy * 0.002;
            if (playerCountry.infra.potable_water_access < 0.0) playerCountry.infra.potable_water_access = 0.0;
            playerCountry.infra.pollution_prob       += legacy * 0.001;
            if (playerCountry.infra.pollution_prob   > 1.0) playerCountry.infra.pollution_prob = 1.0;
            playerCountry.economy.community_conflicts += legacy * 0.003;
        }

        // --- TIER 2: MODERATE CONTAMINATION (0.25 – 0.50) ---
        if (legacy > 0.25) {
            // Health system under pressure from chronic exposure
            playerCountry.welfare.health_coverage    -= 0.003;
            if (playerCountry.welfare.health_coverage < 0.0) playerCountry.welfare.health_coverage = 0.0;
            playerCountry.welfare.mental_health_index -= 0.002; // Living near poisoned land is psychologically corrosive
            if (playerCountry.welfare.mental_health_index < 0.0) playerCountry.welfare.mental_health_index = 0.0;
            // Tourism collapses — visitors avoid contaminated regions
            playerCountry.economy.annual_visitors -= (int)(playerCountry.economy.annual_visitors * legacy * 0.04);
            if (playerCountry.economy.annual_visitors < 0) playerCountry.economy.annual_visitors = 0;
            // Farmland contaminated → food imports rise
            playerCountry.politics.agricultural_power -= 0.002;
            if (playerCountry.politics.agricultural_power < 0.0) playerCountry.politics.agricultural_power = 0.0;
            playerCountry.economy.import_dependency   += 0.002;
            if (playerCountry.economy.import_dependency > 1.0) playerCountry.economy.import_dependency = 1.0;
            if (legacy < 0.35) { // Only print once per tier crossing
                std::cout << "[!] CONTAMINATION SPREADING: Agricultural land affected. "
                          << "Tourist alerts issued. Health system under strain." << std::endl;
            }
        }

        // --- TIER 3: CRITICAL CONTAMINATION (0.50 – 0.75) ---
        if (legacy > 0.50) {
            // International scrutiny — UN and press pick up the story
            playerCountry.welfare.un_score               -= 0.010;
            if (playerCountry.welfare.un_score < 0.0) playerCountry.welfare.un_score = 0.0;
            playerCountry.security.diplomatic_prestige   -= 0.005;
            if (playerCountry.security.diplomatic_prestige < 0.0) playerCountry.security.diplomatic_prestige = 0.0;
            playerCountry.economy.tourist_safety         -= 0.005;
            if (playerCountry.economy.tourist_safety < 0.0) playerCountry.economy.tourist_safety = 0.0;
            // Heavy metal contamination suppresses birth rates (documented: lead, arsenic exposure)
            playerCountry.welfare.birth_rate             -= 0.0001;
            if (playerCountry.welfare.birth_rate < 0.0) playerCountry.welfare.birth_rate = 0.0;
            // Credit markets price in environmental liability
            playerCountry.economy.debt_to_gdp_ratio     += 0.003;
            std::cout << "[!!] ENVIRONMENTAL CRISIS: UN Special Rapporteur investigating mining contamination. "
                      << "Birth rates falling, credit outlook deteriorating." << std::endl;
        }

        // --- TIER 4: CATASTROPHIC CONTAMINATION (> 0.75) ---
        if (legacy > 0.75) {
            // International sanctions risk — investors and governments react
            playerCountry.economy.international_sanctions_prob += 0.015;
            if (playerCountry.economy.international_sanctions_prob > 1.0)
                playerCountry.economy.international_sanctions_prob = 1.0;
            // Population flees contaminated zones
            playerCountry.security.mass_migration_prob += 0.010;
            if (playerCountry.security.mass_migration_prob > 1.0)
                playerCountry.security.mass_migration_prob = 1.0;
            // Infant mortality spike — irreversible generational harm
            std::uniform_real_distribution<> d(0.0, 1.0);
            if (d(rng) < legacy - 0.70) {
                playerCountry.welfare.population -= (int)(playerCountry.welfare.population * 0.0008);
                playerCountry.politics.popularity -= 0.04;
                std::cout << "[!!!] ENVIRONMENTAL CATASTROPHE: Infant mortality spiking in mining regions. "
                          << "International media coverage. Population displacement begins." << std::endl;
            } else {
                std::cout << "[!!!] TOXIC ZONE: Regions around old mines uninhabitable. "
                          << "Sanctions risk rising. Displacement accelerating." << std::endl;
            }
        }
    }

    // Commodity price bust.
    if (playerCountry.economy.commodity_prices < 0.7 && playerCountry.economy.mining_concessions > 0) {
        std::cout << "[!] COMMODITY BUST: Global prices collapsed. Mining revenues plummeted." << std::endl;
        playerCountry.politics.popularity -= 0.01;
    }

    // --- DUTCH DISEASE (The Resource Curse: Manufacturing Hollowing-Out) ---
    // When mining revenues are a large share of GDP, foreign exchange floods in.
    // The currency appreciates → non-mining exports become uncompetitive.
    // Manufacturing quietly dies while the treasury looks full.
    if (playerCountry.economy.mining_concessions > 0 && playerCountry.economy.gdp > 0) {
        double mining_share_gdp = extraction_value / playerCountry.economy.gdp;
        if (mining_share_gdp > 0.08) {
            double dutch_severity = (mining_share_gdp - 0.08) * 0.5; // Scales with over-reliance
            // Currency gets artificially strong (seems good, actually harmful)
            playerCountry.economy.exchange_rate_stability += dutch_severity * 0.08;
            if (playerCountry.economy.exchange_rate_stability > 0.95)
                playerCountry.economy.exchange_rate_stability = 0.95;
            // Manufacturing withers — less competitive, investment flees to extraction
            playerCountry.politics.industrial_power -= dutch_severity * 0.06;
            if (playerCountry.politics.industrial_power < 0.0) playerCountry.politics.industrial_power = 0.0;
            // Non-mining workers get displaced → unemployment rises slightly
            playerCountry.welfare.unemployment_rate += dutch_severity * 0.005;
            if (dutch_severity > 0.08) {
                std::cout << "[!] DUTCH DISEASE: Mining boom crowding out manufacturing. "
                          << "Industry weakening, structural unemployment rising." << std::endl;
            }
        }
    }

    // --- SOVEREIGN WEALTH FUND ---
    if (playerCountry.economy.sovereign_wealth_fund > 0) {

        // Investment return depends on mandate (risk/return tradeoff)
        double base_return, ret_vol;
        std::string mandate_label;
        switch (playerCountry.economy.swf_mandate) {
            case 0: base_return = 0.025; ret_vol = 0.007; mandate_label = "Conservative"; break;
            case 2: base_return = 0.065; ret_vol = 0.050; mandate_label = "Growth";       break;
            default: base_return = 0.045; ret_vol = 0.022; mandate_label = "Balanced";   break;
        }
        std::normal_distribution<> ret_dist(base_return, ret_vol);
        double actual_return = ret_dist(rng);
        if (actual_return < -0.15) actual_return = -0.15; // Floor: catastrophic loss
        if (actual_return >  0.25) actual_return =  0.25; // Cap: exceptional year

        double swf_income = playerCountry.economy.sovereign_wealth_fund * actual_return;
        playerCountry.economy.sovereign_wealth_fund += swf_income;
        // Half of returns flow to reserves (liquidity buffer), half compounds in fund
        if (swf_income > 0)
            playerCountry.economy.international_reserves += swf_income * 0.5;

        // Governance: opaque funds suffer corruption leakage — officials skim returns
        if (!playerCountry.economy.swf_transparent) {
            double leakage = playerCountry.economy.sovereign_wealth_fund
                           * playerCountry.politics.administrative_corruption * 0.025;
            playerCountry.economy.sovereign_wealth_fund -= leakage;
            // Leakage enriches corrupt networks silently — no visibility to player
        } else {
            // Transparent SWF: independent board signals fiscal discipline → FDI premium
            playerCountry.economy.international_reserves +=
                playerCountry.economy.sovereign_wealth_fund * 0.004;
        }

        // Notify on notable returns
        if (actual_return < 0.0) {
            std::cout << "[SWF] INVESTMENT LOSS: " << mandate_label << " mandate returned "
                      << (int)(actual_return * 100) << "%. Balance: $"
                      << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M" << std::endl;
        } else if (actual_return > base_return + ret_vol) {
            std::cout << "[SWF] Exceptional return +" << (int)(actual_return * 100)
                      << "% (" << mandate_label << "). Balance: $"
                      << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M" << std::endl;
        }

        // --- MACRO EFFECTS BY FUND SIZE ---
        double swf_gdp_ratio = playerCountry.economy.sovereign_wealth_fund / playerCountry.economy.gdp;

        if (swf_gdp_ratio > 0.05) {
            // Fund > 5% GDP: bond market recognizes fiscal discipline → lower debt costs
            // Implemented as a partial offset to the credit interest penalty
            // (accessed later via debt_service_cost — we signal here for awareness)
            std::cout << "[SWF] Fund >" << (int)(swf_gdp_ratio * 100) << "% GDP: "
                      << "fiscal credibility reduces sovereign borrowing costs." << std::endl;
            // Tangible effect: debt growth slows slightly
            playerCountry.economy.debt_to_gdp_ratio -= 0.002;
        }

        if (swf_gdp_ratio > 0.15) {
            // Fund > 15% GDP: autonomous stabilizer — SWF income partially replaces taxes
            double stabilizer_boost = playerCountry.economy.sovereign_wealth_fund * 0.008;
            playerCountry.economy.international_reserves += stabilizer_boost;
            std::cout << "[SWF] Fund >15% GDP: autonomous macroeconomic stabilizer active." << std::endl;
        }

        if (swf_gdp_ratio > 0.30) {
            // Fund > 30% GDP (Norway-tier): generational wealth — rating improvement signal
            playerCountry.economy.credit_rating = CreditRating::AA; // Floor upgrade signal
            std::cout << "[SWF] NORWAY TIER: Sovereign fund exceeds 30% GDP. "
                      << "Country achieves investment-grade floor regardless of deficits." << std::endl;
        }
    }

    // --- DEPOSIT RATE: CYCLE AWARENESS ---
    {
        double sc = playerCountry.economy.commodity_supercycle;
        double dr = playerCountry.economy.swf_deposit_rate;

        // Boom with no savings: fiscal irresponsibility warning
        if (sc > 0.20 && dr < 0.15 && royalty_yield > 0.0) {
            std::cout << "[!] BOOM WINDFALL ALERT: Commodity supercycle is bullish but only "
                      << (int)(dr * 100) << "% of royalties are being saved. "
                      << "Consider swf_save or swf_rule to lock in gains." << std::endl;
        }

        // Constitutional rule: enforce minimum 20% during bull markets
        if (playerCountry.economy.swf_rule_active && sc > 0.0 && dr < 0.20 && royalty_yield > 0.0) {
            playerCountry.economy.swf_deposit_rate = 0.20;
            std::cout << "[SWF RULE] Deposit rate auto-corrected to constitutional minimum (20%)." << std::endl;
        }

        // Bear supercycle with high dependency: auto-stabilizer draws from SWF even before crisis
        if (sc < -0.20 && playerCountry.economy.sovereign_wealth_fund > 0.0) {
            double stabilizer = playerCountry.economy.sovereign_wealth_fund * 0.012; // 1.2% auto-release
            if (stabilizer > playerCountry.economy.sovereign_wealth_fund)
                stabilizer = playerCountry.economy.sovereign_wealth_fund;
            playerCountry.economy.sovereign_wealth_fund -= stabilizer;
            playerCountry.economy.tax_collection        += stabilizer;
            playerCountry.politics.popularity           += 0.01; // Visible cushion
            std::cout << "[SWF] Bear-cycle stabilizer: $" << stabilizer / 1000000.0
                      << "M auto-released to buffer commodity downturn." << std::endl;
        }

        // High consistent saving rate (>=30%): long-term debt credibility bonus
        if (dr >= 0.30 && royalty_yield > 0.0) {
            playerCountry.economy.debt_to_gdp_ratio -= 0.001; // Markets reward fiscal discipline
            playerCountry.economy.international_reserves +=
                playerCountry.economy.sovereign_wealth_fund * 0.002; // Confidence premium
        }
    }

    // Auto-deposit: redirect swf_deposit_rate fraction of royalties to fund.
    if (playerCountry.economy.swf_deposit_rate > 0.0 && royalty_yield > 0.0) {
        double deposit = royalty_yield * playerCountry.economy.swf_deposit_rate;
        playerCountry.economy.sovereign_wealth_fund += deposit;
        playerCountry.economy.tax_collection        -= deposit;
        // Transparent SWF: public report each year
        if (playerCountry.economy.swf_transparent) {
            std::cout << "[SWF] Audited deposit: $" << deposit / 1000000.0 << "M. "
                      << "Balance: $" << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M" << std::endl;
        } else if (playerCountry.economy.swf_deposit_rate >= 0.20) {
            // Only announce if rate is meaningful — avoid noise for tiny deposits
            std::cout << "[SWF] Deposited $" << deposit / 1000000.0 << "M (internal). "
                      << "Balance: $" << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M" << std::endl;
        }
    }

    // --- FISCAL DEPENDENCY RISK ---
    // If royalties exceed 25% of non-royalty fiscal revenue → fragile budget.
    // During a commodity bust, this triggers a sudden fiscal crisis.
    double base_tax_revenue = playerCountry.economy.tax_collection - royalty_yield;
    if (base_tax_revenue > 0.0 && royalty_yield > 0.0) {
        double royalty_dependency = royalty_yield / base_tax_revenue;
        if (royalty_dependency > 0.25) {
            if (playerCountry.economy.commodity_prices < 0.8) {
                // Bust + high dependency = fiscal crisis
                double shortfall = royalty_yield * 0.35; // Revenues collapse 35% in bust
                playerCountry.economy.tax_collection -= shortfall;
                playerCountry.politics.popularity    -= 0.04;
                playerCountry.economy.debt_to_gdp_ratio += shortfall / playerCountry.economy.gdp;
                std::cout << "[!!!] FISCAL CRISIS: Budget over-reliant on commodities ("
                          << (int)(royalty_dependency * 100) << "% dependency). "
                          << "Price bust creates $" << shortfall / 1000000.0 << "M shortfall!" << std::endl;
                // SWF absorbs the shock — rule-governed funds absorb more (better governance)
                if (playerCountry.economy.sovereign_wealth_fund > 0.0) {
                    double absorb_rate = playerCountry.economy.swf_rule_active ? 0.75 : 0.50;
                    double absorbed = shortfall * absorb_rate;
                    if (absorbed > playerCountry.economy.sovereign_wealth_fund)
                        absorbed = playerCountry.economy.sovereign_wealth_fund;
                    playerCountry.economy.sovereign_wealth_fund -= absorbed;
                    playerCountry.economy.tax_collection        += absorbed;
                    std::string rule_note = playerCountry.economy.swf_rule_active ? " (constitutional rule buffer)" : "";
                    std::cout << "         [SWF BUFFER" << rule_note << "] Fund absorbed $"
                              << absorbed / 1000000.0 << "M of the shock." << std::endl;
                }
            } else {
                std::cout << "[!] FISCAL WARNING: " << (int)(royalty_dependency * 100)
                          << "% of budget depends on commodity prices. Build the SWF as buffer." << std::endl;
            }
        }
    }

    if (playerCountry.economy.mining_concessions > 0 || playerCountry.economy.mining_legacy_damage > 0.05) {
        std::string phase_label    = dep < 0.30 ? "Ramp-up" : dep < 0.65 ? "Peak" : dep < 0.85 ? "Decline" : "Exhausted";
        std::string price_label    = playerCountry.economy.commodity_prices > 1.25 ? "BOOM"
                                   : playerCountry.economy.commodity_prices > 0.85 ? "Normal" : "BUST";
        std::string supercycle_regime = playerCountry.economy.supercycle_bull ? "Bull" : "Bear";
        std::string supercycle_risk   = (playerCountry.economy.supercycle_bull  && playerCountry.economy.supercycle_years > 15)
                                     || (!playerCountry.economy.supercycle_bull && playerCountry.economy.supercycle_years > 10)
                                      ? "TURNING?" : "Stable";
        std::string supercycle_dir = playerCountry.economy.commodity_supercycle >  0.10 ? "Rising"
                                   : playerCountry.economy.commodity_supercycle < -0.10 ? "Falling" : "Flat";
        std::cout << "[MINING] Concessions: " << playerCountry.economy.mining_concessions
                  << " | Phase: " << phase_label
                  << " | Rate: " << playerCountry.economy.royalty_rate * 100 << "%"
                  << " | Extraction: $" << extraction_value / 1000000.0 << "M"
                  << " | Royalties: $" << royalty_yield / 1000000.0 << "M" << std::endl;
        std::cout << "         Price: " << playerCountry.economy.commodity_prices << "x [" << price_label << "]"
                  << " | Supercycle: " << supercycle_regime
                  << " yr" << playerCountry.economy.supercycle_years
                  << " [" << supercycle_dir << "] [" << supercycle_risk << "]"
                  << " | Depletion: " << dep * 100 << "%"
                  << " | Conflict: " << playerCountry.economy.community_conflicts * 100 << "%"
                  << " | Legacy: " << playerCountry.economy.mining_legacy_damage * 100 << "%" << std::endl;
        if (playerCountry.economy.contaminated_area_km2 > 0.1 || playerCountry.economy.superfund_sites > 0) {
            std::cout << "         Contaminated: " << (int)playerCountry.economy.contaminated_area_km2 << " km²"
                      << " | Cleanup Liability: $" << playerCountry.economy.remediation_cost / 1000000.0 << "M"
                      << " | Superfund Zones: " << playerCountry.economy.superfund_sites;
            if (playerCountry.economy.remediation_active)
                std::cout << " | [REMEDIATION ACTIVE]";
            std::cout << std::endl;
        }
        std::cout << "         Corruption Loss: $" << royalty_evasion / 1000000.0 << "M";
        if (playerCountry.economy.commodity_hedge_turns > 0)
            std::cout << " | HEDGE ACTIVE (" << playerCountry.economy.commodity_hedge_turns << " yr)";
        std::cout << std::endl;
        if (playerCountry.economy.sovereign_wealth_fund > 0)
            std::cout << "         SWF Balance: $" << playerCountry.economy.sovereign_wealth_fund / 1000000.0 << "M"
                      << " (saving " << playerCountry.economy.swf_deposit_rate * 100 << "% of royalties)" << std::endl;
    }

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
    
    // --- NEW: SOVEREIGN CREDIT RATING ---
    // Calculate economic health score (0.0 theoretically horrible to 1.0+ great)
    // - High debt hurts.
    // - Growth helps.
    // - High inflation hurts.
    // - Stability (autonomy) helps.
    double economic_health_score = 
        (1.0 - playerCountry.economy.debt_to_gdp_ratio) * 0.4 + 
        (playerCountry.economy.growth_rate * 10.0) * 0.3 + 
        (1.0 - playerCountry.economy.inflation * 5.0) * 0.2 +
        playerCountry.economy.central_bank_autonomy * 0.1;

    // Default prob logic tied to rating
    playerCountry.economy.default_prob = 0.01; // Base

    // Assign Enum and Penalty based on score bounds
    double credit_interest_penalty = 0.0;
    if (economic_health_score > 0.8) {
        playerCountry.economy.credit_rating = CreditRating::AAA;
        credit_interest_penalty = -0.01; // Reward for great rating
    } else if (economic_health_score > 0.7) {
        playerCountry.economy.credit_rating = CreditRating::AA;
    } else if (economic_health_score > 0.6) {
        playerCountry.economy.credit_rating = CreditRating::A;
        credit_interest_penalty = 0.01;
    } else if (economic_health_score > 0.5) {
        playerCountry.economy.credit_rating = CreditRating::BBB;
        credit_interest_penalty = 0.02;
    } else if (economic_health_score > 0.4) {
        playerCountry.economy.credit_rating = CreditRating::BB; // Junk tier begins
        credit_interest_penalty = 0.04;
        playerCountry.economy.default_prob = 0.05;
    } else if (economic_health_score > 0.3) {
        playerCountry.economy.credit_rating = CreditRating::B;
        credit_interest_penalty = 0.08;
        playerCountry.economy.default_prob = 0.15;
    } else if (economic_health_score > 0.15) {
        playerCountry.economy.credit_rating = CreditRating::CCC;
        credit_interest_penalty = 0.15;
        playerCountry.economy.default_prob = 0.30;
    } else if (economic_health_score > 0.0) {
        playerCountry.economy.credit_rating = CreditRating::CC;
        credit_interest_penalty = 0.25;
        playerCountry.economy.default_prob = 0.50;
    } else if (playerCountry.economy.debt_to_gdp_ratio > 1.2 && playerCountry.economy.international_reserves < 0) {
        playerCountry.economy.credit_rating = CreditRating::D;
        credit_interest_penalty = 0.50; // Nobody lends, punitive default rates
        playerCountry.economy.default_prob = 1.0;
    } else {
        playerCountry.economy.credit_rating = CreditRating::C;
        credit_interest_penalty = 0.35;
        playerCountry.economy.default_prob = 0.80;
    }

    // Debt Service with RISK PREMIUM and CREDIT RATING PENALTY
    // If Debt > 60%, the market charges a risk premium.
    double risk_premium = (playerCountry.economy.debt_to_gdp_ratio > 0.6) ? (playerCountry.economy.debt_to_gdp_ratio - 0.6) * 0.2 : 0.0;
    double effective_debt_interest = playerCountry.economy.interest_rate + risk_premium + credit_interest_penalty;
    
    // Ensure interest doesn't go below 0
    if (effective_debt_interest < 0.0) effective_debt_interest = 0.0;
    
    // Calculate total service cost
    double debt_service_cost = playerCountry.economy.gdp * playerCountry.economy.debt_to_gdp_ratio * effective_debt_interest;
    
    // Track it in the country struct
    playerCountry.economy.debt_interest = debt_service_cost;

    
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
    // demand_pull pendiente de implementar cuando se acceda a net_purchasing_power
    
    // 3. Cost Push (Devaluation / Pass-through)
    double cost_push = 0.0;
    
    // Chronic Pass-through: Low stability causes internal price increases (imported goods)
    cost_push += 0.05 * (1.0 - playerCountry.economy.exchange_rate_stability);
    
    // Shock: If reserves are low, currency devalues sharply
    if (playerCountry.economy.international_reserves < 20000000.0) {
        cost_push += 0.03; 
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
    
    // --- BALANCE OF PAYMENTS (Current Account) ---
    // Export components: goods, tourism (invisible exports), agriculture, mining
    // Each is affected by: tariffs, FTAs, exchange rate, global demand

    // Tariff retaliation: high tariffs → partners restrict our exports
    double tariff_export_penalty = playerCountry.economy.average_tariffs * 0.35;
    // FTA bonus: each agreement opens new markets for our producers
    double fta_export_bonus = playerCountry.economy.free_trade_agreements * 0.020;
    // Exchange rate: strong currency (high stability) → expensive exports, less competitive
    double er_factor = 1.0 - (playerCountry.economy.exchange_rate_stability - 0.50) * 0.25;
    if (er_factor < 0.50) er_factor = 0.50;
    if (er_factor > 1.50) er_factor = 1.50;

    // (a) Goods exports — driven by industry + tech capacity
    double exports_goods = playerCountry.economy.gdp * 0.20
                         * export_exposure
                         * (1.0 + (global_growth_trend - 0.02) * 2.0)
                         * er_factor
                         * (1.0 - tariff_export_penalty)
                         * (1.0 + fta_export_bonus);

    // (b) Tourism — invisible exports: foreign visitors spending domestic FX
    // Tourist safety derives from underlying risk vectors
    playerCountry.economy.tourist_crime_rate = (1.0 - playerCountry.economy.tourist_safety) * 0.08
                                             + playerCountry.security.homicide_rate * 0.001;
    playerCountry.economy.kidnapping_tourism_risk = playerCountry.economy.tourist_crime_rate * 0.05
                                                  + playerCountry.security.conflict_with_groups * 0.02;
    playerCountry.economy.health_risk_tourists = (1.0 - playerCountry.welfare.health_coverage) * 0.03;
    // Risks erode international arrivals — tourists avoid dangerous destinations
    double risk_dampening = 1.0 - (playerCountry.economy.tourist_crime_rate + playerCountry.economy.kidnapping_tourism_risk) * 2.0;
    if (risk_dampening < 0.5) risk_dampening = 0.5;

    // Visa restrictiveness dampens international arrivals; domestic unaffected
    double visa_dampening = (1.0 - playerCountry.economy.visa_restrictiveness * 0.4) * risk_dampening;
    playerCountry.economy.visitors_international = (int)(playerCountry.economy.visitors_international * visa_dampening);
    if (playerCountry.economy.visitors_international < 0) playerCountry.economy.visitors_international = 0;
    playerCountry.economy.annual_visitors = playerCountry.economy.visitors_international
                                          + playerCountry.economy.visitors_domestic;
    // Compute weighted average spending from tier mix
    double mid_share = 1.0 - playerCountry.economy.luxury_tourism_share - 0.3; // 30% budget tier baseline
    if (mid_share < 0.1) mid_share = 0.1;
    playerCountry.economy.average_tourist_spending = playerCountry.economy.spending_luxury * playerCountry.economy.luxury_tourism_share
                                                   + 1000.0 * mid_share
                                                   + playerCountry.economy.spending_budget * 0.3;
    // Travel warning level: driven by safety, conflict, and sanctions
    double alert_score = (1.0 - playerCountry.economy.tourist_safety)
                       + playerCountry.security.conflict_with_groups * 0.5
                       + (playerCountry.economy.sanctions_active ? 0.3 : 0.0);
    if (alert_score > 0.8) playerCountry.economy.travel_warning_level = 3;
    else if (alert_score > 0.5) playerCountry.economy.travel_warning_level = 2;
    else if (alert_score > 0.25) playerCountry.economy.travel_warning_level = 1;
    else playerCountry.economy.travel_warning_level = 0;
    // Warning level accumulates reputational damage — tourists remember bad press
    if (playerCountry.economy.travel_warning_level >= 2) {
        playerCountry.economy.reputational_tourism_damage += 0.02 * playerCountry.economy.travel_warning_level;
        if (playerCountry.economy.reputational_tourism_damage > 1.0) playerCountry.economy.reputational_tourism_damage = 1.0;
    } else {
        playerCountry.economy.reputational_tourism_damage -= 0.01; // Slow recovery
        if (playerCountry.economy.reputational_tourism_damage < 0.0) playerCountry.economy.reputational_tourism_damage = 0.0;
    }
    // Reputational damage directly reduces international visitors
    playerCountry.economy.visitors_international = (int)(playerCountry.economy.visitors_international
                                                       * (1.0 - playerCountry.economy.reputational_tourism_damage * 0.3));

    // Heritage & UNESCO boost luxury tourism share and attract more international visitors
    double heritage_tourism_bonus = 1.0 + playerCountry.economy.unesco_sites * 0.03
                                       + playerCountry.economy.heritage_preservation * 0.1;
    playerCountry.economy.luxury_tourism_share += playerCountry.economy.heritage_funding_gdp * 0.5;
    if (playerCountry.economy.luxury_tourism_share > 0.5) playerCountry.economy.luxury_tourism_share = 0.5;
    // Cultural commodification: too much tourism degrades heritage
    if (playerCountry.economy.annual_visitors > 1000000 && playerCountry.economy.heritage_funding_gdp < 0.002)
        playerCountry.economy.cultural_commodification += 0.003;
    if (playerCountry.economy.cultural_commodification > 0.7)
        playerCountry.economy.heritage_preservation -= 0.005;
    if (playerCountry.economy.heritage_preservation < 0.0) playerCountry.economy.heritage_preservation = 0.0;
    // Seasonality: high seasonality means revenue is volatile — effective spending drops
    double seasonality_factor = 1.0 - playerCountry.economy.tourism_seasonality * 0.15;
    double tourism_exports = (double)playerCountry.economy.annual_visitors
                           * playerCountry.economy.average_tourist_spending
                           * playerCountry.economy.tourist_safety
                           * seasonality_factor * heritage_tourism_bonus;

    // (c) Agricultural exports — boosted by FTAs, dampened by strong currency
    double agri_exports = playerCountry.economy.gdp * 0.04
                        * playerCountry.politics.agricultural_power
                        * er_factor
                        * (1.0 + fta_export_bonus * 0.5);

    // (d) Mining/commodity exports — FX earnings from extraction (net of domestic consumption)
    double mining_exports = playerCountry.economy.state_royalties * 0.6;

    // Low RoO compliance means exporters can't fully use FTA preferences → reduced export benefit
    double fta_export_penalty = 1.0 - (1.0 - playerCountry.economy.rules_of_origin_compliance) * 0.3;
    double total_exports = (exports_goods + tourism_exports + agri_exports + mining_exports) * fta_export_penalty;

    // Imports: tariffs + NTBs reduce volume but FTAs re-open them; purchasing power drives demand
    double total_trade_friction = (playerCountry.economy.average_tariffs
                                 + playerCountry.economy.non_tariff_barriers * 0.4) * 0.25;
    double tariff_import_reduction = total_trade_friction;
    // FTA boost scales with number of agreements AND their depth (shallow=goods, deep=services+investment)
    double fta_import_boost = playerCountry.economy.free_trade_agreements * 0.008
                            * (0.5 + playerCountry.economy.fta_depth_index * 0.5);
    double imports = playerCountry.economy.gdp
                   * playerCountry.economy.import_dependency
                   * (net_purchasing_power > 0.8 ? net_purchasing_power : 0.8)
                   * (1.0 - tariff_import_reduction)
                   * (1.0 + fta_import_boost);

    playerCountry.economy.trade_balance = total_exports - imports;

    // Services balance: tourism net + financial services + IP royalties (correlated with innovation)
    double services_exports = tourism_exports
                            + playerCountry.economy.gdp * playerCountry.infra.innovation_index * 0.005
                            + playerCountry.economy.gdp * playerCountry.politics.financial_power * 0.003;
    double services_imports = playerCountry.economy.gdp * 0.02; // Consulting, licenses, insurance
    playerCountry.economy.services_balance = services_exports - services_imports;

    // Current account = trade + services + remittances (already in economy struct)
    playerCountry.economy.current_account_balance = playerCountry.economy.trade_balance
                                                  + playerCountry.economy.services_balance
                                                  + playerCountry.economy.remittances;

    // Trade openness: structural measure of economic integration
    double total_trade_volume = total_exports + imports;
    if (playerCountry.economy.gdp > 0.0)
        playerCountry.economy.trade_openness = total_trade_volume / playerCountry.economy.gdp;
    if (playerCountry.economy.trade_openness > 2.0) playerCountry.economy.trade_openness = 2.0;

    // --- TARIFF REGIME EFFECTS ---
    {
        // Compute weighted average tariff from sector-specific rates
        // Weights: manufactured ~50%, agricultural ~30%, strategic ~20% of import basket
        playerCountry.economy.average_tariffs = playerCountry.economy.tariff_manufactured * 0.50
                                              + playerCountry.economy.tariff_agricultural * 0.30
                                              + playerCountry.economy.tariff_strategic * 0.20;

        // Anti-dumping cases: cada caso activo erosiona prestigio diplomático
        if (playerCountry.economy.antidumping_cases > 0) {
            playerCountry.security.diplomatic_prestige -= playerCountry.economy.antidumping_cases * 0.003;
            if (playerCountry.security.diplomatic_prestige < 0.0) playerCountry.security.diplomatic_prestige = 0.0;
        }

        double tariffs = playerCountry.economy.average_tariffs;

        // 1. Customs revenue: tariffs collect duties on import flow
        //    Evasion scales with tariff level — high walls incentivize smuggling
        double base_evasion  = 0.15; // 15% baseline leakage (mis-invoicing, corruption)
        double extra_evasion = (tariffs > 0.20) ? (tariffs - 0.20) * 1.2 : 0.0;
        double collection_efficiency = 1.0 - base_evasion - extra_evasion;
        if (collection_efficiency < 0.20) collection_efficiency = 0.20; // Floor: some always collected
        double customs_revenue = imports * tariffs * collection_efficiency;
        playerCountry.economy.tax_collection += customs_revenue;

        // 2. REGIME TIER: FREE TRADE (< 5%)
        //    Open economy bonus: cheap imports → real purchasing power rises
        if (tariffs < 0.05) {
            playerCountry.welfare.poverty_rate -= 0.002; // Affordable imported goods
            if (playerCountry.welfare.poverty_rate < 0.0) playerCountry.welfare.poverty_rate = 0.0;
            playerCountry.economy.growth_rate  += 0.0008; // Consumption-led micro-boost
        }

        // 3. REGIME TIER: MODERATE (5–15%) — standard developing-nation range, no side effects

        // 4. REGIME TIER: PROTECTIONIST (15–25%)
        //    Industry benefits but smuggling begins, consumer prices creep up
        if (tariffs > 0.15) {
            playerCountry.welfare.labor_informality     += 0.003; // Informal traders fill the gap
            if (playerCountry.welfare.labor_informality > 1.0) playerCountry.welfare.labor_informality = 1.0;
            playerCountry.politics.administrative_corruption += 0.002; // Customs bribery
            if (playerCountry.politics.administrative_corruption > 1.0)
                playerCountry.politics.administrative_corruption = 1.0;
        }

        // 5. REGIME TIER: HEAVY (> 25%)
        //    WTO dispute risk, partners may file complaints
        if (tariffs > 0.25) {
            playerCountry.economy.international_sanctions_prob += 0.008;
            if (playerCountry.economy.international_sanctions_prob > 1.0)
                playerCountry.economy.international_sanctions_prob = 1.0;
            // Smuggling undermines the very protection tariffs were meant to provide
            playerCountry.welfare.labor_informality     += 0.004;
            if (playerCountry.welfare.labor_informality > 1.0) playerCountry.welfare.labor_informality = 1.0;
            if (tariffs > 0.28) {
                std::cout << "[!] TARIFF WALL: WTO members filing dispute complaints. "
                          << "Sanctions risk rising. Smuggling networks expanding." << std::endl;
            }
        }

        // 6. REGIME TIER: NEAR-AUTARKY (> 35%)
        //    Extreme isolation: FDI collapses, black markets dominate
        if (tariffs > 0.35) {
            playerCountry.economy.international_reserves -= playerCountry.economy.gdp * 0.005;
            playerCountry.economy.growth_rate            -= 0.004; // Structural long-run damage
            playerCountry.welfare.labor_informality      += 0.006;
            if (playerCountry.welfare.labor_informality   > 1.0) playerCountry.welfare.labor_informality = 1.0;
            std::cout << "[!!] NEAR-AUTARKY: Trade walls suffocating economic dynamism. "
                      << "FDI fleeing. Black markets flourishing." << std::endl;
        }

        // Trade diversion from FTAs: deep agreements with many partners can hollow out domestic industry
        if (playerCountry.economy.free_trade_agreements > 8 && playerCountry.economy.fta_depth_index > 0.6) {
            playerCountry.economy.trade_diversion_risk += 0.005;
        } else if (playerCountry.economy.free_trade_agreements < 3) {
            playerCountry.economy.trade_diversion_risk -= 0.002;
        }
        if (playerCountry.economy.trade_diversion_risk > 1.0) playerCountry.economy.trade_diversion_risk = 1.0;
        if (playerCountry.economy.trade_diversion_risk < 0.0) playerCountry.economy.trade_diversion_risk = 0.0;
        // High trade diversion erodes domestic industry
        if (playerCountry.economy.trade_diversion_risk > 0.3) {
            playerCountry.politics.industrial_power -= playerCountry.economy.trade_diversion_risk * 0.005;
            if (playerCountry.politics.industrial_power < 0.0) playerCountry.politics.industrial_power = 0.0;
        }
    }

    // --- TRADE BALANCE FEEDBACK EFFECTS ---
    double gdp_now = playerCountry.economy.gdp;

    // Persistent current account deficit → currency depreciation pressure + import-price inflation
    if (playerCountry.economy.current_account_balance < -(gdp_now * 0.03)) {
        playerCountry.economy.exchange_rate_stability -= 0.012;
        playerCountry.economy.inflation              += 0.004;
        std::cout << "[!] CURRENT ACCOUNT DEFICIT: Persistent external gap weakening currency." << std::endl;
    }
    // Large surplus → FX reserve accumulation bonus + mild demand-pull inflation
    if (playerCountry.economy.current_account_balance > gdp_now * 0.04) {
        playerCountry.economy.international_reserves += playerCountry.economy.current_account_balance * 0.08;
        playerCountry.economy.inflation              += 0.002;
    }
    // Import substitution: strong industry → domestic production displaces imports over time
    if (playerCountry.politics.industrial_power > 0.60
        && playerCountry.economy.import_dependency > 0.15) {
        playerCountry.economy.import_dependency -= 0.002;
    }
    // Deindustrialization trap: low industry + high import dependence → industry further erodes
    if (playerCountry.politics.industrial_power < 0.25
        && playerCountry.economy.import_dependency > 0.45) {
        playerCountry.politics.industrial_power -= 0.003;
        if (playerCountry.politics.industrial_power < 0.0) playerCountry.politics.industrial_power = 0.0;
    }

    // --- STRATEGIC IMPORT DEPENDENCY DYNAMICS ---
    // Agricultural power reduces food import dependency; weak agro increases it
    if (playerCountry.politics.agricultural_power > 0.5)
        playerCountry.economy.food_import_dependency -= 0.002;
    else if (playerCountry.politics.agricultural_power < 0.25)
        playerCountry.economy.food_import_dependency += 0.001;
    if (playerCountry.economy.food_import_dependency < 0.0) playerCountry.economy.food_import_dependency = 0.0;
    if (playerCountry.economy.food_import_dependency > 1.0) playerCountry.economy.food_import_dependency = 1.0;

    // Domestic energy production reduces energy dependency
    if (playerCountry.infra.renewables_percentage > 0.4 || playerCountry.infra.oil_gas_reserves > 500000.0)
        playerCountry.economy.energy_import_dependency -= 0.003;
    if (playerCountry.economy.energy_import_dependency < 0.0) playerCountry.economy.energy_import_dependency = 0.0;
    if (playerCountry.economy.energy_import_dependency > 1.0) playerCountry.economy.energy_import_dependency = 1.0;

    // R&D and innovation reduce medicine dependency over time
    if (playerCountry.infra.innovation_index > 0.5)
        playerCountry.economy.medicine_import_dependency -= 0.002;
    if (playerCountry.economy.medicine_import_dependency < 0.0) playerCountry.economy.medicine_import_dependency = 0.0;
    if (playerCountry.economy.medicine_import_dependency > 1.0) playerCountry.economy.medicine_import_dependency = 1.0;

    // Supply chain vulnerability: weighted average of strategic dependencies
    playerCountry.economy.supply_chain_vulnerability = playerCountry.economy.food_import_dependency * 0.35
                                                     + playerCountry.economy.energy_import_dependency * 0.35
                                                     + playerCountry.economy.medicine_import_dependency * 0.30;
    // High vulnerability amplifies external shock damage
    if (playerCountry.economy.supply_chain_vulnerability > 0.6 && playerCountry.economy.commodity_prices < 0.8) {
        playerCountry.economy.inflation += 0.003;
        std::cout << "[!] SUPPLY CHAIN STRESS: High strategic import dependency during price shock." << std::endl;
    }

    // --- SANCTIONS REGIME DYNAMICS ---
    // High probability triggers activation; tier escalates with persistence
    if (!playerCountry.economy.sanctions_active && playerCountry.economy.international_sanctions_prob > 0.60) {
        playerCountry.economy.sanctions_active = true;
        playerCountry.economy.sanctions_tier = 1; // Start at targeted
        std::cout << "[!!] SANCTIONS IMPOSED: International community enacts targeted sanctions (individuals/assets)." << std::endl;
    }
    if (playerCountry.economy.sanctions_active) {
        // Escalation: sustained bad behavior raises tier
        if (playerCountry.economy.international_sanctions_prob > 0.75 && playerCountry.economy.sanctions_tier < 2) {
            playerCountry.economy.sanctions_tier = 2; // Sectoral
            std::cout << "[!!] SANCTIONS ESCALATION: Sectoral sanctions imposed (finance, energy, trade)." << std::endl;
        }
        if (playerCountry.economy.international_sanctions_prob > 0.90 && playerCountry.economy.sanctions_tier < 3) {
            playerCountry.economy.sanctions_tier = 3; // Comprehensive
            std::cout << "[!!!] TOTAL SANCTIONS: Comprehensive economic embargo in effect." << std::endl;
        }
        // GDP impact by tier: 1=0.5%, 2=2%, 3=6%
        double tier_impacts[] = {0.0, 0.005, 0.02, 0.06};
        playerCountry.economy.sanctions_gdp_impact = tier_impacts[playerCountry.economy.sanctions_tier];
        playerCountry.economy.gdp *= (1.0 - playerCountry.economy.sanctions_gdp_impact);
        playerCountry.economy.growth_rate -= playerCountry.economy.sanctions_gdp_impact * 0.3;
        // Sanctions relief: probability drops below threshold → de-escalate
        if (playerCountry.economy.international_sanctions_prob < 0.30) {
            playerCountry.economy.sanctions_active = false;
            playerCountry.economy.sanctions_tier = 0;
            playerCountry.economy.sanctions_gdp_impact = 0.0;
            std::cout << "[+] SANCTIONS LIFTED: International community removes restrictions." << std::endl;
        }
    }

    // --- CROSS-SYSTEM FEEDBACK: SECURITY → ECONOMIC ---
    {
        // War/conflict drains economy
        if (playerCountry.security.war_active) {
            playerCountry.economy.gdp *= (1.0 - 0.03 * (1.0 + playerCountry.security.war_duration * 0.1));
            playerCountry.infra.fdi_inflow_gdp *= 0.5; // FDI flees war zones
            playerCountry.economy.annual_visitors = (int)(playerCountry.economy.annual_visitors * 0.3);
            playerCountry.economy.debt_to_gdp_ratio += 0.02;
        }

        // High crime deters investment and tourism
        if (playerCountry.security.homicide_rate > 20.0) {
            double crime_factor = (playerCountry.security.homicide_rate - 20.0) / 100.0;
            if (crime_factor > 0.3) crime_factor = 0.3;
            playerCountry.economy.tourist_safety -= crime_factor * 0.1;
            if (playerCountry.economy.tourist_safety < 0.1) playerCountry.economy.tourist_safety = 0.1;
            playerCountry.infra.fdi_inflow_gdp -= crime_factor * 0.01;
            if (playerCountry.infra.fdi_inflow_gdp < 0.0) playerCountry.infra.fdi_inflow_gdp = 0.0;
            playerCountry.welfare.brain_drain += crime_factor * 0.01;
        }

        // Sanctions cascade into inflation and reserves
        if (playerCountry.economy.sanctions_active) {
            playerCountry.economy.inflation += playerCountry.economy.sanctions_tier * 0.01;
            playerCountry.economy.international_reserves -= playerCountry.economy.international_reserves * (playerCountry.economy.sanctions_tier * 0.01);
            playerCountry.economy.trade_balance -= playerCountry.economy.gdp * playerCountry.economy.sanctions_tier * 0.005;
        }

        // Terrorism/conflict destabilizes markets
        if (playerCountry.security.conflict_with_groups > 0.5) {
            playerCountry.infra.investment_climate_index -= 0.02;
            if (playerCountry.infra.investment_climate_index < 0.1) playerCountry.infra.investment_climate_index = 0.1;
        }
    }

    // --- CALCULATE DYNAMIC EXCHANGE RATE STABILITY ---
    // Stability depends on: Institutions (Autonomy) + War Chest (Reserves) + Fundamentals (Trade)
    double base_stability = 0.4 + (playerCountry.economy.central_bank_autonomy * 0.4);
    
    // Reserves Bonus: If reserves cover > 3 turns of imports
    if (playerCountry.economy.international_reserves > (imports * 3.0)) {
        base_stability += 0.1;
    }
    
    // Trade Bonus: Surplus boosts confidence
    if (playerCountry.economy.trade_balance > 0) {
        base_stability += 0.1;
    }
    
    // Shock Penalty: Crisis destroys all confidence
    if (playerCountry.economy.international_reserves <= 0) {
        base_stability = 0.1;
    }
    
    if (base_stability > 0.95) base_stability = 0.95;
    playerCountry.economy.exchange_rate_stability = base_stability;
    
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
    
    // Sovereign Default FDI Penalty:
    if (playerCountry.economy.debt_to_gdp_ratio > 1.2 && playerCountry.economy.international_reserves <= 0) {
        capital_flow_base = -1000000000.0; // Massive divestment
    }
    
    
    // FDI (Long term)
    // Autonomy is a Multiplier for TRUST.
    double fdi = (playerCountry.welfare.un_score * 0.02) * playerCountry.economy.gdp * (0.5 + playerCountry.economy.central_bank_autonomy); 
    
    // 3. Debt Service
    // We pay interest on external debt.
    double external_debt_interest = playerCountry.economy.gdp * playerCountry.economy.debt_to_gdp_ratio * 0.05; // 5% avg interest
    
    // 3. Sovereign Default (The Ultimate Economic Failure)
    // Triggered by high debt and zero liquidity.
    if (playerCountry.economy.debt_to_gdp_ratio > 1.2 && playerCountry.economy.international_reserves < 0) {
        std::cout << "[!!!] SOVEREIGN DEFAULT: The country has suspended debt payments!" << std::endl;
        
        // GDP Crash (-10%)
        playerCountry.economy.gdp *= 0.90;
        
        // Reputation destruction
        playerCountry.politics.popularity -= 0.30;
        playerCountry.politics.financial_power -= 0.5;
        playerCountry.welfare.un_score -= 0.4;
        
        // Social Panic
        playerCountry.politics.polarization_index += 0.20;
        
        // The "Default Trap" resets reserves to 0 but cuts you off from markets (FDI hit handled below).
        playerCountry.economy.international_reserves = 0;
        
        std::cout << "      Market chaos. Banking system frozen. FDI fleeing." << std::endl;
    }
    
    // Net Change in Reserves
    double reserves_change = playerCountry.economy.current_account_balance + fdi + capital_flow_base - external_debt_interest;
    
    playerCountry.economy.international_reserves += reserves_change;
    
    // Display BoP breakdown
    std::cout << "[TRADE] Exports: $" << (int)(total_exports / 1000000.0) << "M"
              << " (Goods $" << (int)(exports_goods    / 1000000.0) << "M"
              << " | Tourism $" << (int)(tourism_exports / 1000000.0) << "M"
              << " | Agri $"    << (int)(agri_exports    / 1000000.0) << "M"
              << " | Mining $"  << (int)(mining_exports  / 1000000.0) << "M)" << std::endl;
    {
        double t = playerCountry.economy.average_tariffs;
        double evasion_display = 0.15 + (t > 0.20 ? (t - 0.20) * 1.2 : 0.0);
        if (evasion_display > 0.80) evasion_display = 0.80;
        double customs_display = imports * t * (1.0 - evasion_display);
        std::string tariff_regime = t < 0.05  ? "Free Trade"
                                  : t < 0.15  ? "Moderate"
                                  : t < 0.25  ? "Protectionist"
                                  : t < 0.35  ? "Heavy"
                                  :             "Near-Autarky";
        std::cout << "[TRADE] Imports: $" << (int)(imports / 1000000.0) << "M"
                  << " | Goods Balance: " << (playerCountry.economy.trade_balance >= 0 ? "+" : "")
                  << (int)(playerCountry.economy.trade_balance / 1000000.0) << "M"
                  << " | Services: " << (playerCountry.economy.services_balance >= 0 ? "+" : "")
                  << (int)(playerCountry.economy.services_balance / 1000000.0) << "M"
                  << " | C/A: " << (playerCountry.economy.current_account_balance >= 0 ? "+" : "")
                  << (int)(playerCountry.economy.current_account_balance / 1000000.0) << "M" << std::endl;
        std::cout << "        Tariffs: " << (int)(t * 100) << "% [" << tariff_regime << "]"
                  << " (Mfg " << (int)(playerCountry.economy.tariff_manufactured * 100)
                  << "% | Agri " << (int)(playerCountry.economy.tariff_agricultural * 100)
                  << "% | Strat " << (int)(playerCountry.economy.tariff_strategic * 100) << "%)"
                  << " | NTB: " << (int)(playerCountry.economy.non_tariff_barriers * 100) << "%" << std::endl;
        std::cout << "        Customs: $" << (int)(customs_display / 1000000.0) << "M"
                  << " | FTAs: " << playerCountry.economy.free_trade_agreements
                  << " | Openness: " << (int)(playerCountry.economy.trade_openness * 100) << "%";
        if (playerCountry.economy.antidumping_cases > 0)
            std::cout << " | Antidumping: " << playerCountry.economy.antidumping_cases << " cases";
        std::cout << std::endl;
    }
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
    
    // --- PANDEMIC EVENT ---
    if (playerCountry.welfare.pandemic_active) {
        // Active pandemic: accumulate damage each turn
        playerCountry.welfare.pandemic_duration--;
        double sev = playerCountry.welfare.pandemic_severity;

        // Deaths: worse with low health coverage, high density
        double mortality_factor = sev * 0.003 * (1.0 + (1.0 - playerCountry.welfare.health_coverage));
        if (playerCountry.welfare.population_density > 300.0) mortality_factor *= 1.5;
        playerCountry.welfare.death_rate += mortality_factor;
        playerCountry.welfare.pandemic_death_toll += mortality_factor * 1000000.0; // Rough absolute

        // Economic damage: lockdowns, supply chain, labor loss
        double gdp_hit = playerCountry.economy.gdp * sev * 0.04;
        playerCountry.economy.gdp -= gdp_hit;
        playerCountry.welfare.pandemic_economic_cost += gdp_hit;
        playerCountry.welfare.unemployment_rate += sev * 0.02;
        if (playerCountry.welfare.unemployment_rate > 0.40)
            playerCountry.welfare.unemployment_rate = 0.40;

        // Health system strain
        playerCountry.welfare.health_coverage -= sev * 0.03;
        if (playerCountry.welfare.health_coverage < 0.05)
            playerCountry.welfare.health_coverage = 0.05;
        playerCountry.welfare.mental_health_index -= sev * 0.02;
        if (playerCountry.welfare.mental_health_index < 0.1)
            playerCountry.welfare.mental_health_index = 0.1;

        // Tourism collapses
        playerCountry.economy.annual_visitors *= (1.0 - sev * 0.4);

        // Popularity hit
        playerCountry.politics.popularity -= 0.02 * sev;

        // Hospital overload: if hospitals per capita too low, severity worsens
        double hospitals_per_million = (double)playerCountry.welfare.hospitals / (playerCountry.welfare.population / 1000000.0);
        if (hospitals_per_million < 20.0) {
            playerCountry.welfare.pandemic_severity += 0.02; // Overwhelmed system
            if (playerCountry.welfare.pandemic_severity > 1.0)
                playerCountry.welfare.pandemic_severity = 1.0;
        }

        std::cout << "[!!!] PANDEMIC (Turn " << (playerCountry.welfare.pandemic_duration + 1)
                  << " remaining): Severity " << (int)(sev * 100) << "%. Deaths: "
                  << (int)playerCountry.welfare.pandemic_death_toll
                  << ". GDP loss: $" << (long long)playerCountry.welfare.pandemic_economic_cost << std::endl;

        // End condition
        if (playerCountry.welfare.pandemic_duration <= 0) {
            playerCountry.welfare.pandemic_active = false;
            playerCountry.welfare.pandemic_severity = 0.0;
            std::cout << "[INFO] PANDEMIC OVER: Recovery begins. Total death toll: "
                      << (int)playerCountry.welfare.pandemic_death_toll
                      << ". Economic cost: $" << (long long)playerCountry.welfare.pandemic_economic_cost << std::endl;
            // Slow recovery: epidemic_prob elevated for a while
            playerCountry.welfare.epidemic_prob += 0.05;
        }
    } else {
        // Stochastic pandemic trigger
        if (dist(rng) < playerCountry.welfare.pandemic_prob) {
            playerCountry.welfare.pandemic_active = true;
            std::uniform_int_distribution<int> dur_dist(3, 8);
            playerCountry.welfare.pandemic_duration = dur_dist(rng);
            std::uniform_real_distribution<double> sev_dist(0.3, 1.0);
            playerCountry.welfare.pandemic_severity = sev_dist(rng);
            playerCountry.welfare.pandemic_death_toll = 0.0;
            playerCountry.welfare.pandemic_economic_cost = 0.0;
            std::cout << "[!!!!] PANDEMIC OUTBREAK: A novel pathogen has emerged! Severity: "
                      << (int)(playerCountry.welfare.pandemic_severity * 100)
                      << "%. Expected duration: " << playerCountry.welfare.pandemic_duration
                      << " turns." << std::endl;
        }
    }

    // --- EPIDEMIC EVENT ---
    if (playerCountry.welfare.epidemic_active) {
        playerCountry.welfare.epidemic_duration--;
        double sev = playerCountry.welfare.epidemic_severity;

        // Moderate health impact
        playerCountry.welfare.death_rate += sev * 0.001;
        playerCountry.welfare.health_coverage -= sev * 0.015;
        if (playerCountry.welfare.health_coverage < 0.05)
            playerCountry.welfare.health_coverage = 0.05;

        // Smaller economic hit than pandemic
        double gdp_hit = playerCountry.economy.gdp * sev * 0.015;
        playerCountry.economy.gdp -= gdp_hit;
        playerCountry.welfare.unemployment_rate += sev * 0.005;

        // Productivity loss
        playerCountry.politics.popularity -= 0.01 * sev;

        std::cout << "[!!] EPIDEMIC (Turn " << (playerCountry.welfare.epidemic_duration + 1)
                  << " remaining): Severity " << (int)(sev * 100) << "%" << std::endl;

        // Can escalate to pandemic if health system is overwhelmed
        if (playerCountry.welfare.health_coverage < 0.4 && !playerCountry.welfare.pandemic_active) {
            if (dist(rng) < 0.15) { // 15% chance per turn of escalation
                playerCountry.welfare.pandemic_active = true;
                playerCountry.welfare.pandemic_duration = 4;
                playerCountry.welfare.pandemic_severity = sev * 2.0;
                if (playerCountry.welfare.pandemic_severity > 1.0)
                    playerCountry.welfare.pandemic_severity = 1.0;
                playerCountry.welfare.pandemic_death_toll = 0.0;
                playerCountry.welfare.pandemic_economic_cost = 0.0;
                playerCountry.welfare.epidemic_active = false;
                std::cout << "[!!!!] EPIDEMIC ESCALATION: Overwhelmed health system — epidemic has become a PANDEMIC!" << std::endl;
            }
        }

        if (playerCountry.welfare.epidemic_active && playerCountry.welfare.epidemic_duration <= 0) {
            playerCountry.welfare.epidemic_active = false;
            playerCountry.welfare.epidemic_severity = 0.0;
            std::cout << "[INFO] EPIDEMIC CONTAINED: Public health response has controlled the outbreak." << std::endl;
        }
    } else if (!playerCountry.welfare.pandemic_active) {
        // Stochastic epidemic trigger (only if no pandemic active)
        if (dist(rng) < playerCountry.welfare.epidemic_prob) {
            playerCountry.welfare.epidemic_active = true;
            std::uniform_int_distribution<int> dur_dist(1, 4);
            playerCountry.welfare.epidemic_duration = dur_dist(rng);
            std::uniform_real_distribution<double> sev_dist(0.1, 0.5);
            playerCountry.welfare.epidemic_severity = sev_dist(rng);
            std::cout << "[!!] EPIDEMIC OUTBREAK: Disease spreading. Severity: "
                      << (int)(playerCountry.welfare.epidemic_severity * 100)
                      << "%. Duration: " << playerCountry.welfare.epidemic_duration
                      << " turns." << std::endl;
        }
    }

    // --- FAMINE / FOOD CONTAMINATION EVENT ---
    if (playerCountry.welfare.famine_active) {
        playerCountry.welfare.famine_duration--;
        double sev = playerCountry.welfare.famine_severity;

        // Death rate spikes
        playerCountry.welfare.death_rate += sev * 0.005;

        // Health system overwhelmed
        playerCountry.welfare.health_coverage -= sev * 0.02;
        if (playerCountry.welfare.health_coverage < 0.05)
            playerCountry.welfare.health_coverage = 0.05;

        // Massive popularity hit
        playerCountry.politics.popularity -= 0.04 * sev;

        // Protest escalation
        playerCountry.politics.protest_intensity += 0.05 * sev;

        // Mass emigration
        playerCountry.security.emigration_push_index += 0.03 * sev;
        if (playerCountry.security.emigration_push_index > 1.0)
            playerCountry.security.emigration_push_index = 1.0;

        // International aid dependency (reduces reserves drain)
        if (playerCountry.security.diplomatic_prestige > 0.5)
            playerCountry.economy.international_reserves += 5000000.0; // Aid
        else
            playerCountry.economy.international_reserves -= 10000000.0; // Must buy food

        std::cout << "[!!!] FAMINE (Turn " << (playerCountry.welfare.famine_duration + 1)
                  << " remaining): Severity " << (int)(sev * 100) << "%. People are starving." << std::endl;

        if (playerCountry.welfare.famine_duration <= 0) {
            playerCountry.welfare.famine_active = false;
            playerCountry.welfare.famine_severity = 0.0;
            std::cout << "[INFO] FAMINE SUBSIDES: Food supply stabilizing." << std::endl;
        }
    } else {
        // Famine triggered by: drought crop loss > 60% OR food contamination event
        bool famine_trigger = false;
        if (playerCountry.infra.drought_active && playerCountry.infra.crop_loss_pct > 0.6) {
            famine_trigger = true;
        }
        if (dist(rng) < playerCountry.welfare.food_contamination_prob * 0.3) {
            famine_trigger = true;
            std::cout << "[!!] FOOD CONTAMINATION: Major contamination event in food supply chain!" << std::endl;
        }
        if (famine_trigger) {
            playerCountry.welfare.famine_active = true;
            std::uniform_int_distribution<int> dur_dist(2, 5);
            playerCountry.welfare.famine_duration = dur_dist(rng);
            std::uniform_real_distribution<double> sev_dist(0.3, 0.9);
            playerCountry.welfare.famine_severity = sev_dist(rng);
            // If drought-triggered, severity scales with crop loss
            if (playerCountry.infra.drought_active) {
                playerCountry.welfare.famine_severity = std::max(playerCountry.welfare.famine_severity,
                                                                  playerCountry.infra.crop_loss_pct);
            }
            std::cout << "[!!!] FAMINE BEGINS: Food crisis! Severity: "
                      << (int)(playerCountry.welfare.famine_severity * 100)
                      << "%. Duration: " << playerCountry.welfare.famine_duration << " turns." << std::endl;
        }
    }

    // --- GENERAL STRIKE EVENT ---
    if (playerCountry.welfare.general_strike_active) {
        playerCountry.welfare.strike_duration--;

        // Economy halts: GDP hit scales with union strength
        double gdp_loss = playerCountry.economy.gdp * 0.04 * playerCountry.welfare.union_strength;
        playerCountry.economy.gdp -= gdp_loss;
        playerCountry.welfare.strike_economic_cost += gdp_loss;

        // Ports and airports shut
        playerCountry.infra.port_capacity *= 0.7;
        playerCountry.economy.annual_visitors *= 0.6; // Tourism disrupted

        // Exports halted
        playerCountry.economy.gdp -= playerCountry.economy.gdp * 0.01; // Trade paralysis

        // Popularity: depends on whether player concedes or represses
        playerCountry.politics.popularity -= 0.02;
        playerCountry.politics.protest_intensity += 0.03;

        std::cout << "[!!] GENERAL STRIKE (Turn " << (playerCountry.welfare.strike_duration + 1)
                  << " remaining): Economy paralyzed. Cost: $"
                  << (long long)playerCountry.welfare.strike_economic_cost << std::endl;

        if (playerCountry.welfare.strike_duration <= 0) {
            playerCountry.welfare.general_strike_active = false;
            // Strike resolution: automatic concession (wage increase)
            playerCountry.welfare.minimum_wage *= 1.1; // 10% raise
            playerCountry.economy.gdp -= playerCountry.economy.gdp * 0.005; // Fiscal cost of concession
            std::cout << "[INFO] STRIKE RESOLVED: Workers return. Minimum wage increased 10%." << std::endl;
        }
    } else {
        if (dist(rng) < playerCountry.welfare.general_strike_prob) {
            playerCountry.welfare.general_strike_active = true;
            // Duration scales with union strength
            int base_dur = (int)(playerCountry.welfare.union_strength * 4.0) + 1;
            if (base_dur > 4) base_dur = 4;
            playerCountry.welfare.strike_duration = base_dur;
            playerCountry.welfare.strike_economic_cost = 0.0;
            std::cout << "[!!] GENERAL STRIKE: Workers across all sectors walk out! Union strength: "
                      << (int)(playerCountry.welfare.union_strength * 100) << "%. Duration: "
                      << playerCountry.welfare.strike_duration << " turns." << std::endl;
        }
    }

    // --- RELIGIOUS CRISIS ---
    if (playerCountry.welfare.religious_crisis_active) {
        playerCountry.welfare.religious_crisis_duration--;

        // Sectarian violence
        playerCountry.security.homicide_rate += 2.0; // +2 per 100k
        playerCountry.welfare.death_rate += 0.001;

        // Polarization and protests
        playerCountry.politics.polarization_index += 0.03;
        if (playerCountry.politics.polarization_index > 1.0)
            playerCountry.politics.polarization_index = 1.0;
        playerCountry.politics.protest_intensity += 0.04;

        // Tourism collapses (safety)
        playerCountry.economy.annual_visitors *= 0.75;
        playerCountry.economy.travel_warning_level = std::max(playerCountry.economy.travel_warning_level, 2);

        // Minority protection erodes
        playerCountry.welfare.minority_protection -= 0.03;
        if (playerCountry.welfare.minority_protection < 0.0)
            playerCountry.welfare.minority_protection = 0.0;

        // Terror attack risk if radicalism high
        if (playerCountry.welfare.radicalism_prob > 0.3 && dist(rng) < playerCountry.welfare.radicalism_prob * 0.5) {
            int terror_casualties = 10 + (int)(dist(rng) * 100);
            playerCountry.welfare.death_rate += (double)terror_casualties / (double)playerCountry.welfare.population;
            playerCountry.politics.popularity -= 0.05;
            std::cout << "[!!!!] TERRORIST ATTACK: Radical group strikes! " << terror_casualties
                      << " casualties." << std::endl;
        }

        playerCountry.politics.popularity -= 0.02;

        std::cout << "[!!] RELIGIOUS CRISIS (Turn " << (playerCountry.welfare.religious_crisis_duration + 1)
                  << " remaining): Sectarian violence ongoing. Tension: "
                  << (int)(playerCountry.welfare.interreligious_tension * 100) << "%" << std::endl;

        if (playerCountry.welfare.religious_crisis_duration <= 0) {
            playerCountry.welfare.religious_crisis_active = false;
            std::cout << "[INFO] RELIGIOUS CRISIS EASES: Interfaith dialogue calming tensions." << std::endl;
        }
    } else {
        if (playerCountry.welfare.interreligious_tension > 0.7
            && dist(rng) < playerCountry.welfare.radicalism_prob) {
            playerCountry.welfare.religious_crisis_active = true;
            std::uniform_int_distribution<int> dur_dist(2, 4);
            playerCountry.welfare.religious_crisis_duration = dur_dist(rng);
            std::cout << "[!!] RELIGIOUS CRISIS: Sectarian tensions erupt into violence! "
                      << "Interreligious tension: " << (int)(playerCountry.welfare.interreligious_tension * 100)
                      << "%" << std::endl;
        }
    }

    // --- HUMAN RIGHTS CRISIS ---
    if (playerCountry.welfare.human_rights_crisis) {
        playerCountry.welfare.hr_crisis_duration--;

        // UN investigation: score plummets
        playerCountry.welfare.un_score -= 0.05;
        if (playerCountry.welfare.un_score < 0.0) playerCountry.welfare.un_score = 0.0;

        // Sanctions risk
        playerCountry.economy.international_sanctions_prob += 0.03;

        // Tourism warning
        playerCountry.economy.travel_warning_level = std::max(playerCountry.economy.travel_warning_level, 2);
        playerCountry.economy.annual_visitors *= 0.85;

        // FDI withdrawal
        playerCountry.economy.international_reserves -= 3000000.0;

        // Diplomatic prestige tanks
        playerCountry.security.diplomatic_prestige -= 0.04;
        if (playerCountry.security.diplomatic_prestige < 0.0)
            playerCountry.security.diplomatic_prestige = 0.0;

        // Domestic protest
        playerCountry.politics.protest_intensity += 0.03;
        playerCountry.politics.popularity -= 0.02;

        // Press freedom under pressure
        playerCountry.welfare.freedom_of_expression -= 0.02;
        if (playerCountry.welfare.freedom_of_expression < 0.05)
            playerCountry.welfare.freedom_of_expression = 0.05;

        std::cout << "[!!!] HUMAN RIGHTS CRISIS (Turn " << (playerCountry.welfare.hr_crisis_duration + 1)
                  << " remaining): UN investigation ongoing. Sanctions risk: "
                  << (int)(playerCountry.economy.international_sanctions_prob * 100) << "%" << std::endl;

        if (playerCountry.welfare.hr_crisis_duration <= 0) {
            playerCountry.welfare.human_rights_crisis = false;
            std::cout << "[INFO] HUMAN RIGHTS CRISIS EASES: International scrutiny subsiding." << std::endl;
        }
    } else {
        bool hr_trigger = (playerCountry.welfare.torture_index > 0.5
                          || playerCountry.welfare.forced_disappearances > 100);
        if (hr_trigger && dist(rng) < 0.15) {
            playerCountry.welfare.human_rights_crisis = true;
            std::uniform_int_distribution<int> dur_dist(2, 4);
            playerCountry.welfare.hr_crisis_duration = dur_dist(rng);
            std::cout << "[!!!] HUMAN RIGHTS CRISIS: International community condemns systematic abuses! "
                      << "UN Special Rapporteur dispatched." << std::endl;
        }
    }

    // --- MASS CASUALTY EVENT (Industrial/Transport Disaster) ---
    // Single-turn shock: reset flag each turn
    playerCountry.welfare.mass_casualty_event = false;
    if (dist(rng) < playerCountry.welfare.mass_casualty_prob) {
        playerCountry.welfare.mass_casualty_event = true;

        // Casualties inversely proportional to maintenance level
        double vulnerability = 1.0 - playerCountry.infra.maintenance_level;
        std::uniform_real_distribution<double> sev_dist(0.2, 1.0);
        double severity = sev_dist(rng);
        playerCountry.welfare.casualty_count = severity * vulnerability * 300.0; // Up to 300 deaths
        playerCountry.welfare.death_rate += playerCountry.welfare.casualty_count / (double)playerCountry.welfare.population;

        // Response quality
        playerCountry.welfare.disaster_response_score = playerCountry.welfare.health_coverage * 0.5
                                                       + playerCountry.infra.maintenance_level * 0.3
                                                       + (playerCountry.welfare.hospitals > 80 ? 0.2 : 0.1);
        if (playerCountry.welfare.disaster_response_score > 1.0)
            playerCountry.welfare.disaster_response_score = 1.0;

        // Emergency cost
        playerCountry.economy.international_reserves -= 5000000.0 * severity;

        // Popularity impact depends on response quality
        if (playerCountry.welfare.disaster_response_score > 0.7) {
            playerCountry.politics.popularity += 0.005; // Competent response
            std::cout << "[!!] MASS CASUALTY EVENT: " << (int)playerCountry.welfare.casualty_count
                      << " dead. Emergency response: EFFECTIVE." << std::endl;
        } else {
            playerCountry.politics.popularity -= 0.03 * severity;
            std::cout << "[!!] MASS CASUALTY EVENT: " << (int)playerCountry.welfare.casualty_count
                      << " dead. Emergency response: POOR. Public outrage." << std::endl;
        }
    }

    // --- COUP RISK DYNAMICS ---
    // Coup probability driven by: low civilian control, low popularity, military insubordination, history
    // IDEOLOGY: Military prefers right-authoritarian. Strong left shift increases coup risk.
    double ideology_mil_tension = 0.0;
    if (playerCountry.politics.economic_ideology < 0.3) {
        ideology_mil_tension += (0.3 - playerCountry.politics.economic_ideology) * 0.05;
    }
    if (playerCountry.politics.auth_dem_axis < 0.3 && playerCountry.security.politicized_officer_corps > 0.3) {
        ideology_mil_tension += (0.3 - playerCountry.politics.auth_dem_axis) * 0.03;
    }
    playerCountry.politics.coup_d_etat_prob = (1.0 - playerCountry.politics.civilian_military_control) * 0.03
                                            + playerCountry.security.military_insubordination_prob * 0.15
                                            + playerCountry.politics.coup_attempts_history * 0.005
                                            + (playerCountry.politics.popularity < 0.25 ? 0.02 : 0.0)
                                            + ideology_mil_tension;
    if (playerCountry.politics.coup_d_etat_prob > 0.5) playerCountry.politics.coup_d_etat_prob = 0.5;
    // Coup success probability: higher with weak institutions
    playerCountry.politics.coup_success_prob = (1.0 - playerCountry.politics.judicial_independence) * 0.4
                                             + (1.0 - playerCountry.politics.civilian_military_control) * 0.3;
    if (playerCountry.politics.coup_success_prob > 0.9) playerCountry.politics.coup_success_prob = 0.9;
    // Coup attempt event (stochastic)
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        if (dist(rng) < playerCountry.politics.coup_d_etat_prob) {
            playerCountry.politics.coup_attempts_history++;
            if (dist(rng) < playerCountry.politics.coup_success_prob) {
                std::cout << "[!!!] COUP D'ETAT: Military seizes power. Democratic order overthrown." << std::endl;
                std::cout << "GAME OVER." << std::endl;
                isRunning = false;
            } else {
                std::cout << "[!!] FAILED COUP ATTEMPT: Military faction attempted seizure — loyalists held." << std::endl;
                playerCountry.politics.popularity += 0.05; // Rally-around-flag
                playerCountry.politics.civilian_military_control += 0.05; // Purge strengthens control
                playerCountry.security.military_insubordination_prob -= 0.03;
                playerCountry.economy.international_reserves -= 20000000.0; // Crisis cost
                playerCountry.economy.exchange_rate_stability -= 0.08;
            }
        }
    }

    // --- AI & AUTOMATION DYNAMICS ---
    // AI development level: driven by talent, compute, data governance, and strategy
    playerCountry.infra.state_ai_development = playerCountry.infra.ai_talent_pool * 0.3
                                             + playerCountry.infra.ai_compute_capacity * 0.25
                                             + playerCountry.infra.ai_data_governance * 0.15
                                             + (playerCountry.infra.ai_national_strategy ? 0.15 : 0.0)
                                             + playerCountry.infra.innovation_index * 0.15;
    if (playerCountry.infra.state_ai_development > 1.0) playerCountry.infra.state_ai_development = 1.0;
    // AI talent pool grows with education and R&D
    playerCountry.infra.ai_talent_pool += playerCountry.infra.university_research_quality * 0.003
                                        + playerCountry.infra.total_rd_gdp * 0.5 * 0.002;
    // Brain drain reduces talent pool
    playerCountry.infra.ai_talent_pool -= playerCountry.welfare.brain_drain * 0.005;
    if (playerCountry.infra.ai_talent_pool < 0.05) playerCountry.infra.ai_talent_pool = 0.05;
    if (playerCountry.infra.ai_talent_pool > 1.0) playerCountry.infra.ai_talent_pool = 1.0;
    // Employment automation risk: scales with AI development, mitigated by retraining
    playerCountry.infra.employment_automation = playerCountry.infra.state_ai_development * 0.3
                                              + playerCountry.infra.manufacturing_automation_risk * 0.3
                                              + playerCountry.infra.service_automation_risk * 0.3
                                              - playerCountry.infra.automation_retraining_investment * 50.0;
    if (playerCountry.infra.employment_automation < 0.0) playerCountry.infra.employment_automation = 0.0;
    if (playerCountry.infra.employment_automation > 0.5) playerCountry.infra.employment_automation = 0.5;
    // High automation with low retraining → unemployment spike
    if (playerCountry.infra.employment_automation > 0.3 && playerCountry.infra.automation_retraining_investment < 0.005) {
        playerCountry.welfare.unemployment_rate += 0.005;
        playerCountry.politics.popularity -= 0.005;
    }
    // Cyber threat landscape
    playerCountry.infra.ai_cyberattack_prob = playerCountry.infra.state_sponsored_cyber_threat * 0.3
                                            + playerCountry.infra.critical_infrastructure_cyber_risk * 0.2
                                            + (1.0 - playerCountry.infra.cyber_defense_maturity) * 0.2;
    if (playerCountry.infra.ai_cyberattack_prob > 0.5) playerCountry.infra.ai_cyberattack_prob = 0.5;
    // Cyber defense maturity grows with intelligence capability + investment
    playerCountry.infra.cyber_defense_maturity += playerCountry.security.sigint_capability * 0.003
                                                + playerCountry.infra.total_rd_gdp * 0.01;
    if (playerCountry.infra.cyber_defense_maturity > 1.0) playerCountry.infra.cyber_defense_maturity = 1.0;
    // Algorithmic ethics: composite governance score
    playerCountry.infra.algorithmic_ethics = (playerCountry.infra.ai_ethics_law ? 0.3 : 0.0)
                                           + (1.0 - playerCountry.infra.algorithmic_bias_index) * 0.3
                                           + playerCountry.infra.ai_accountability_framework * 0.2
                                           + playerCountry.infra.autonomous_weapons_restraint * 0.2;
    if (playerCountry.infra.algorithmic_ethics > 1.0) playerCountry.infra.algorithmic_ethics = 1.0;

    // --- TECHNOLOGY / CYBER CRISIS ---
    if (playerCountry.infra.tech_crisis_active) {
        playerCountry.infra.tech_crisis_duration--;

        // Financial system disruption: GDP hit
        playerCountry.economy.gdp -= playerCountry.economy.gdp * 0.025;

        // Banking/payment systems down
        playerCountry.economy.international_reserves -= 5000000.0;

        // Intelligence compromised
        playerCountry.security.document_leak_prob += 0.1;
        if (playerCountry.security.document_leak_prob > 0.8)
            playerCountry.security.document_leak_prob = 0.8;
        playerCountry.security.informant_network -= 0.05;
        if (playerCountry.security.informant_network < 0.0)
            playerCountry.security.informant_network = 0.0;

        // Automation shock: unemployment spike
        playerCountry.welfare.unemployment_rate += 0.008;

        // Grid vulnerability exposed
        playerCountry.infra.cyber_grid_vulnerability += 0.05;
        if (playerCountry.infra.cyber_grid_vulnerability > 0.8)
            playerCountry.infra.cyber_grid_vulnerability = 0.8;

        playerCountry.politics.popularity -= 0.02;

        std::cout << "[!!!] TECH CRISIS (Turn " << (playerCountry.infra.tech_crisis_duration + 1)
                  << " remaining): Critical systems compromised. Financial disruption ongoing." << std::endl;

        if (playerCountry.infra.tech_crisis_duration <= 0) {
            playerCountry.infra.tech_crisis_active = false;
            playerCountry.infra.cyber_defense_maturity += 0.05; // Lessons learned
            std::cout << "[INFO] TECH CRISIS RESOLVED: Systems restored. Cyber defense strengthened." << std::endl;
        }
    } else {
        if (dist(rng) < playerCountry.infra.ai_cyberattack_prob) {
            playerCountry.infra.tech_crisis_active = true;
            std::uniform_int_distribution<int> dur_dist(1, 3);
            playerCountry.infra.tech_crisis_duration = dur_dist(rng);
            // State-sponsored attribution
            if (playerCountry.infra.state_sponsored_cyber_threat > 0.3) {
                std::cout << "[!!!!] STATE-SPONSORED CYBERATTACK: Foreign power has breached critical infrastructure! "
                          << "Duration: " << playerCountry.infra.tech_crisis_duration << " turns." << std::endl;
                playerCountry.security.diplomatic_prestige -= 0.05;
            } else {
                std::cout << "[!!!] MAJOR CYBERATTACK: Banking, defense, and communications systems compromised! "
                          << "Duration: " << playerCountry.infra.tech_crisis_duration << " turns." << std::endl;
            }
        }
    }

    // --- SPACE PROGRAM DYNAMICS ---
    // Total satellites
    playerCountry.infra.satellite_capacity = playerCountry.infra.satellites_civil + playerCountry.infra.satellites_military;
    // Space budget as GDP share
    if (playerCountry.economy.gdp > 0)
        playerCountry.infra.space_budget_gdp = playerCountry.infra.space_budget / playerCountry.economy.gdp;
    // Launch maturity: more successful launches reduce failure probability
    if (playerCountry.infra.successful_launches > 10)
        playerCountry.infra.launch_failure_prob = 0.05;
    else if (playerCountry.infra.successful_launches > 3)
        playerCountry.infra.launch_failure_prob = 0.08;
    // Space prestige
    playerCountry.infra.space_prestige = playerCountry.infra.satellites_civil * 0.02
                                       + playerCountry.infra.satellites_military * 0.03
                                       + (playerCountry.infra.own_launch_capability ? 0.2 : 0.0)
                                       + (playerCountry.infra.human_spaceflight_capable ? 0.3 : 0.0)
                                       + playerCountry.infra.international_space_partnerships * 0.02;
    if (playerCountry.infra.space_prestige > 1.0) playerCountry.infra.space_prestige = 1.0;
    // Tech export share: innovation + patents + space
    playerCountry.infra.tech_export_share = playerCountry.infra.innovation_index * 0.1
                                          + playerCountry.infra.patents_granted_international * 0.001;
    if (playerCountry.infra.tech_export_share > 0.5) playerCountry.infra.tech_export_share = 0.5;
    // Technological prestige: composite
    playerCountry.infra.technological_prestige = playerCountry.infra.innovation_index * 0.4
                                               + playerCountry.infra.space_prestige * 0.3
                                               + playerCountry.infra.tech_export_share * 0.3;
    if (playerCountry.infra.technological_prestige > 1.0) playerCountry.infra.technological_prestige = 1.0;

    // --- SPACE RACE CRISIS (Launch Disaster) ---
    // Check each turn if a launch attempt happens AND fails
    if (playerCountry.infra.own_launch_capability && dist(rng) < 0.3) { // 30% chance of launch attempt per turn
        if (dist(rng) < playerCountry.infra.launch_failure_prob) {
            playerCountry.infra.space_disaster = true;

            // Cost of lost payload
            playerCountry.infra.space_disaster_cost = playerCountry.infra.space_budget * 0.5;
            playerCountry.economy.international_reserves -= playerCountry.infra.space_disaster_cost;

            // Prestige collapses
            playerCountry.infra.space_prestige *= 0.5;
            playerCountry.infra.technological_prestige -= 0.1;
            if (playerCountry.infra.technological_prestige < 0.0)
                playerCountry.infra.technological_prestige = 0.0;

            // Crewed mission: casualties
            if (playerCountry.infra.human_spaceflight_capable && dist(rng) < 0.3) {
                int crew_lost = 3 + (int)(dist(rng) * 4); // 3-7 astronauts
                playerCountry.welfare.death_rate += (double)crew_lost / (double)playerCountry.welfare.population;
                playerCountry.politics.popularity -= 0.04;
                std::cout << "[!!!!] SPACE DISASTER: Crewed launch failure! " << crew_lost
                          << " astronauts killed. National mourning." << std::endl;
            } else {
                std::cout << "[!!!] SPACE DISASTER: Launch failure! Payload lost. Cost: $"
                          << (long long)playerCountry.infra.space_disaster_cost << std::endl;
            }

            // Public questions spending during hardship
            if (playerCountry.economy.in_recession) {
                playerCountry.politics.popularity -= 0.03;
                std::cout << "       Public outrage: Why spend on space during a recession?" << std::endl;
            } else {
                playerCountry.politics.popularity -= 0.01;
            }
        } else {
            // Successful launch
            playerCountry.infra.successful_launches++;
            playerCountry.infra.space_prestige += 0.02;
            playerCountry.infra.space_disaster = false;
        }
    } else {
        playerCountry.infra.space_disaster = false;
    }

    // --- ENERGY DYNAMICS ---
    // Renewables percentage: sum of solar + wind vs total capacity
    double total_re_gw = playerCountry.infra.solar_capacity_gw + playerCountry.infra.wind_capacity_gw;
    double implied_total_gw = total_re_gw / (playerCountry.infra.renewables_percentage + 0.01);
    if (implied_total_gw > 0)
        playerCountry.infra.renewables_percentage = total_re_gw / implied_total_gw;
    playerCountry.infra.fossil_fuel_dependency = 1.0 - playerCountry.infra.renewables_percentage;
    // Energy transition speed: net-zero commitment accelerates transition
    if (playerCountry.infra.net_zero_commitment)
        playerCountry.infra.energy_transition_speed = 0.01;
    playerCountry.infra.fossil_fuel_dependency -= playerCountry.infra.energy_transition_speed;
    if (playerCountry.infra.fossil_fuel_dependency < 0.05) playerCountry.infra.fossil_fuel_dependency = 0.05;
    // Stranded asset risk: high fossil dependency + global transition pressure
    playerCountry.infra.stranded_asset_risk = playerCountry.infra.fossil_fuel_dependency * 0.3;
    // Oil/gas reserves deplete over time
    playerCountry.infra.oil_gas_reserves *= (1.0 - playerCountry.infra.reserve_depletion_rate);
    // Grid resilience: storage + maintenance + low transmission losses
    playerCountry.infra.grid_resilience = 0.3 + playerCountry.infra.energy_storage_hours * 0.1
                                        + playerCountry.infra.maintenance_level * 0.2
                                        + (1.0 - playerCountry.infra.transmission_loss_rate) * 0.2;
    if (playerCountry.infra.grid_resilience > 1.0) playerCountry.infra.grid_resilience = 1.0;
    // Blackout probability: inversely related to resilience + capacity margin + cyber risk
    playerCountry.infra.blackout_prob = (1.0 - playerCountry.infra.grid_resilience) * 0.1
                                      + (playerCountry.infra.generation_capacity < 1.0 ? 0.1 : 0.0)
                                      + playerCountry.infra.cyber_grid_vulnerability * 0.05;
    if (playerCountry.infra.blackout_prob < 0.005) playerCountry.infra.blackout_prob = 0.005;
    // SAIDI: interruption hours driven by blackout prob and grid resilience
    playerCountry.infra.saidi_hours = playerCountry.infra.blackout_prob * 200.0
                                    + (1.0 - playerCountry.infra.grid_resilience) * 10.0;
    // Energy affordability
    playerCountry.infra.energy_affordability = 1.0 - playerCountry.infra.kwh_price * 2.0
                                             + playerCountry.infra.energy_subsidy_gdp * 20.0;
    if (playerCountry.infra.energy_affordability < 0.1) playerCountry.infra.energy_affordability = 0.1;
    if (playerCountry.infra.energy_affordability > 1.0) playerCountry.infra.energy_affordability = 1.0;
    // Industrial price: lower than residential, driven by subsidies
    playerCountry.infra.industrial_kwh_price = playerCountry.infra.kwh_price * 0.7;
    // Curtailment: renewables wasted when storage is insufficient
    if (playerCountry.infra.renewables_percentage > 0.4 && playerCountry.infra.energy_storage_hours < 2.0)
        playerCountry.infra.grid_curtailment_rate = (playerCountry.infra.renewables_percentage - 0.4) * 0.3;
    else
        playerCountry.infra.grid_curtailment_rate = 0.01;

    // --- MASS BLACKOUT EVENT ---
    if (playerCountry.infra.blackout_active) {
        playerCountry.infra.blackout_duration--;

        // GDP hit: industry halts
        double gdp_loss = playerCountry.economy.gdp * 0.03 * playerCountry.infra.fossil_fuel_dependency;
        playerCountry.economy.gdp -= gdp_loss;
        playerCountry.infra.blackout_economic_loss += gdp_loss;

        // Unemployment spike from industrial shutdown
        playerCountry.welfare.unemployment_rate += 0.01;
        if (playerCountry.welfare.unemployment_rate > 0.35)
            playerCountry.welfare.unemployment_rate = 0.35;

        // Hospital deaths if backup is poor
        if (playerCountry.infra.critical_infrastructure_backup < 0.5) {
            double hospital_deaths = (1.0 - playerCountry.infra.critical_infrastructure_backup) * 50.0;
            playerCountry.welfare.death_rate += hospital_deaths / (double)playerCountry.welfare.population;
            std::cout << "[!!!] BLACKOUT: Hospitals losing power. Estimated deaths: "
                      << (int)hospital_deaths << std::endl;
        }

        // Protests and popularity
        playerCountry.politics.protest_intensity += 0.04;
        playerCountry.politics.popularity -= 0.03;

        std::cout << "[!!!] MASS BLACKOUT (Turn " << (playerCountry.infra.blackout_duration + 1)
                  << " remaining): Economic loss: $" << (long long)playerCountry.infra.blackout_economic_loss
                  << std::endl;

        if (playerCountry.infra.blackout_duration <= 0) {
            playerCountry.infra.blackout_active = false;
            std::cout << "[INFO] POWER RESTORED: Grid back online. Total loss: $"
                      << (long long)playerCountry.infra.blackout_economic_loss << std::endl;
        }
    } else {
        if (dist(rng) < playerCountry.infra.blackout_prob) {
            playerCountry.infra.blackout_active = true;
            std::uniform_int_distribution<int> dur_dist(1, 3);
            playerCountry.infra.blackout_duration = dur_dist(rng);
            playerCountry.infra.blackout_economic_loss = 0.0;

            // Check if cyberattack caused it
            if (dist(rng) < playerCountry.infra.cyber_grid_vulnerability) {
                std::cout << "[!!!!] CYBERATTACK BLACKOUT: Grid compromised by hostile cyber operation! Duration: "
                          << playerCountry.infra.blackout_duration << " turns." << std::endl;
                playerCountry.security.diplomatic_prestige -= 0.05; // Humiliation
            } else {
                std::cout << "[!!!] MASS BLACKOUT: Grid failure! Duration: "
                          << playerCountry.infra.blackout_duration << " turns." << std::endl;
            }
        }
    }

    // --- INFRASTRUCTURE DYNAMICS ---
    // Logistics performance: composite of road, rail, port, and air
    playerCountry.infra.logistics_performance = playerCountry.infra.road_connectivity * 0.3
                                              + playerCountry.infra.rail_connectivity * 0.2
                                              + playerCountry.infra.port_efficiency * 0.25
                                              + (double)playerCountry.infra.international_airports / 5.0 * 0.25;
    if (playerCountry.infra.logistics_performance > 1.0) playerCountry.infra.logistics_performance = 1.0;
    // Road quality degrades without maintenance
    playerCountry.infra.road_quality_index += (playerCountry.infra.maintenance_level - 0.5) * 0.02;
    if (playerCountry.infra.road_quality_index < 0.1) playerCountry.infra.road_quality_index = 0.1;
    if (playerCountry.infra.road_quality_index > 1.0) playerCountry.infra.road_quality_index = 1.0;
    // Port efficiency from capacity utilization and investment
    playerCountry.infra.port_efficiency = playerCountry.infra.port_capacity * 0.6
                                        + playerCountry.infra.maintenance_level * 0.4;
    if (playerCountry.infra.port_efficiency > 1.0) playerCountry.infra.port_efficiency = 1.0;
    // Digital divide narrows with internet investment
    playerCountry.infra.digital_divide_rural = (1.0 - playerCountry.infra.internet_coverage) * 0.6
                                             + (1.0 - playerCountry.infra.broadband_penetration) * 0.4;
    if (playerCountry.infra.digital_divide_rural > 1.0) playerCountry.infra.digital_divide_rural = 1.0;
    // Water stress: population density + climate vulnerability
    playerCountry.infra.water_stress_index = playerCountry.welfare.population_density * 0.002
                                           + playerCountry.infra.climate_vulnerability_index * 0.3
                                           - playerCountry.infra.potable_water_access * 0.1;
    if (playerCountry.infra.water_stress_index < 0.0) playerCountry.infra.water_stress_index = 0.0;
    if (playerCountry.infra.water_stress_index > 1.0) playerCountry.infra.water_stress_index = 1.0;
    // Maintenance level: driven by public works spending
    playerCountry.infra.maintenance_level = playerCountry.infra.public_works_spending_gdp * 12.0;
    if (playerCountry.infra.maintenance_level > 1.0) playerCountry.infra.maintenance_level = 1.0;
    // Deferred maintenance backlog accumulates when spending is inadequate
    double annual_maintenance_need = playerCountry.economy.gdp * playerCountry.infra.infrastructure_depreciation_rate;
    double actual_maintenance = playerCountry.economy.gdp * playerCountry.infra.public_works_spending_gdp;
    playerCountry.infra.deferred_maintenance_backlog += (annual_maintenance_need - actual_maintenance);
    if (playerCountry.infra.deferred_maintenance_backlog < 0.0) playerCountry.infra.deferred_maintenance_backlog = 0.0;

    // --- CROSS-SYSTEM FEEDBACK: INFRASTRUCTURE → ECONOMIC ---
    {
        // Blackouts devastate economy
        if (playerCountry.infra.blackout_active) {
            playerCountry.economy.gdp *= (1.0 - 0.01 * playerCountry.infra.blackout_duration);
            playerCountry.welfare.unemployment_rate += 0.005;
        }

        // Poor logistics raises trade costs → worsens trade balance
        if (playerCountry.infra.logistics_performance < 0.4) {
            double logistics_penalty = (0.4 - playerCountry.infra.logistics_performance) * 0.02;
            playerCountry.economy.trade_balance -= playerCountry.economy.gdp * logistics_penalty;
            playerCountry.economy.non_tariff_barriers += 0.01; // De facto barrier from poor infra
            if (playerCountry.economy.non_tariff_barriers > 0.8) playerCountry.economy.non_tariff_barriers = 0.8;
        }

        // High renewables reduce fossil import dependency → energy independence
        if (playerCountry.infra.renewables_percentage > 0.5) {
            double green_bonus = (playerCountry.infra.renewables_percentage - 0.5) * 0.1;
            playerCountry.economy.energy_import_dependency -= green_bonus;
            if (playerCountry.economy.energy_import_dependency < 0.05) playerCountry.economy.energy_import_dependency = 0.05;
            // Lower energy costs boost competitiveness
            playerCountry.economy.growth_rate += green_bonus * 0.05;
        }

        // Deferred maintenance → cascading infrastructure failures
        double backlog_ratio = playerCountry.infra.deferred_maintenance_backlog / (playerCountry.economy.gdp * 0.1 + 1.0);
        if (backlog_ratio > 0.3) {
            // Infrastructure deterioration accelerates
            playerCountry.infra.road_connectivity -= 0.01;
            if (playerCountry.infra.road_connectivity < 0.2) playerCountry.infra.road_connectivity = 0.2;
            playerCountry.infra.port_capacity -= 0.005;
            if (playerCountry.infra.port_capacity < 0.2) playerCountry.infra.port_capacity = 0.2;
            // Increases blackout probability
            playerCountry.infra.blackout_prob += 0.01;
            if (playerCountry.infra.blackout_prob > 0.3) playerCountry.infra.blackout_prob = 0.3;
            // GDP drag from crumbling infrastructure
            playerCountry.economy.growth_rate -= backlog_ratio * 0.01;
        }

        // Internet coverage boosts innovation and service economy
        if (playerCountry.infra.internet_coverage > 0.9 && playerCountry.infra.broadband_penetration > 0.7) {
            playerCountry.infra.innovation_index += 0.005;
            if (playerCountry.infra.innovation_index > 1.0) playerCountry.infra.innovation_index = 1.0;
            playerCountry.economy.growth_rate += 0.002;
        }
    }

    // --- SCIENCE, TECHNOLOGY & INNOVATION DYNAMICS ---
    // Total R&D intensity
    playerCountry.infra.total_rd_gdp = playerCountry.infra.st_investment_gdp + playerCountry.infra.private_rd_gdp;
    // Researcher density grows with R&D spending and education
    playerCountry.infra.researcher_density += (playerCountry.infra.total_rd_gdp * 100.0 - 2.0) * 5.0;
    if (playerCountry.infra.researcher_density < 50.0) playerCountry.infra.researcher_density = 50.0;
    // Investment climate driven by rule of law, corruption, and stability
    playerCountry.infra.investment_climate_index = playerCountry.politics.judicial_independence * 0.3
                                                 + (1.0 - playerCountry.politics.administrative_corruption) * 0.3
                                                 + playerCountry.economy.exchange_rate_stability * 0.2
                                                 + playerCountry.security.press_freedom * 0.1;
    if (playerCountry.infra.investment_climate_index > 1.0) playerCountry.infra.investment_climate_index = 1.0;
    // Capital flight risk: sanctions, instability, high corruption
    playerCountry.infra.capital_flight_risk = playerCountry.economy.international_sanctions_prob * 0.3
                                            + playerCountry.politics.polarization_index * 0.2
                                            + playerCountry.politics.administrative_corruption * 0.2;
    if (playerCountry.infra.capital_flight_risk > 0.8) playerCountry.infra.capital_flight_risk = 0.8;
    // Patent commercialization driven by industry-academia linkage and startup ecosystem
    playerCountry.infra.patent_commercialization = playerCountry.infra.industry_academia_linkage * 0.5
                                                 + playerCountry.infra.startup_ecosystem_strength * 0.3;
    if (playerCountry.infra.patent_commercialization > 0.8) playerCountry.infra.patent_commercialization = 0.8;
    // Technology transfer: FDI + trade openness enable it
    playerCountry.infra.technology_transfer_index = playerCountry.infra.fdi_inflow_gdp * 2.0
                                                  + playerCountry.economy.trade_openness * 0.2;
    if (playerCountry.infra.technology_transfer_index > 1.0) playerCountry.infra.technology_transfer_index = 1.0;
    // Innovation index: GII-style composite
    playerCountry.infra.innovation_index = playerCountry.infra.total_rd_gdp * 5.0 * 0.25
                                         + playerCountry.infra.patent_commercialization * 0.25
                                         + playerCountry.infra.startup_ecosystem_strength * 0.25
                                         + playerCountry.infra.university_research_quality * 0.25;
    if (playerCountry.infra.innovation_index > 1.0) playerCountry.infra.innovation_index = 1.0;
    // University research quality grows slowly with spending
    playerCountry.infra.university_research_quality += (playerCountry.welfare.research_spending_gdp - 0.005) * 0.5;
    if (playerCountry.infra.university_research_quality < 0.1) playerCountry.infra.university_research_quality = 0.1;
    if (playerCountry.infra.university_research_quality > 1.0) playerCountry.infra.university_research_quality = 1.0;

    // --- ENVIRONMENT & CLIMATE DYNAMICS ---
    // Air quality: driven by CO2, industry, and pollution events
    playerCountry.infra.air_quality_index = 1.0 - playerCountry.infra.pollution_prob * 0.3
                                          - playerCountry.infra.co2_emissions / 5000000.0;
    if (playerCountry.infra.air_quality_index < 0.1) playerCountry.infra.air_quality_index = 0.1;
    // Water quality linked to mining legacy and sanitation
    playerCountry.infra.water_quality_index = 1.0 - playerCountry.economy.mining_legacy_damage * 0.3
                                            - playerCountry.infra.soil_contamination_index * 0.2;
    if (playerCountry.infra.water_quality_index < 0.1) playerCountry.infra.water_quality_index = 0.1;
    // Biodiversity eroded by deforestation and pollution
    playerCountry.infra.biodiversity_index -= playerCountry.infra.deforestation_rate * 0.05
                                           + playerCountry.infra.illegal_wildlife_trade * 0.01;
    playerCountry.infra.biodiversity_index += playerCountry.infra.protected_area_coverage * 0.005;
    if (playerCountry.infra.biodiversity_index < 0.05) playerCountry.infra.biodiversity_index = 0.05;
    if (playerCountry.infra.biodiversity_index > 1.0) playerCountry.infra.biodiversity_index = 1.0;
    // Climate vulnerability composite
    playerCountry.infra.climate_vulnerability_index = playerCountry.infra.drought_prob * 0.25
                                                    + playerCountry.infra.storm_prob * 0.20
                                                    + playerCountry.infra.flood_prob * 0.25
                                                    + (1.0 - playerCountry.infra.climate_adaptation_investment * 50.0) * 0.30;
    if (playerCountry.infra.climate_vulnerability_index < 0.0) playerCountry.infra.climate_vulnerability_index = 0.0;
    if (playerCountry.infra.climate_vulnerability_index > 1.0) playerCountry.infra.climate_vulnerability_index = 1.0;
    // CO2 per capita
    if (playerCountry.welfare.population > 0)
        playerCountry.infra.co2_per_capita = playerCountry.infra.co2_emissions / (double)playerCountry.welfare.population;
    // Emissions trajectory
    playerCountry.infra.co2_emissions *= (1.0 + playerCountry.infra.emissions_trajectory);
    if (playerCountry.infra.co2_emissions < 0.0) playerCountry.infra.co2_emissions = 0.0;
    // Carbon tax revenue
    if (playerCountry.infra.carbon_tax_active && playerCountry.infra.carbon_tax_rate > 0.0) {
        double carbon_revenue = playerCountry.infra.co2_emissions * playerCountry.infra.carbon_tax_rate;
        playerCountry.economy.tax_collection += carbon_revenue;
        // Carbon tax incentivizes emissions reduction
        playerCountry.infra.emissions_trajectory -= 0.002;
        playerCountry.infra.fossil_fuel_dependency -= 0.005;
        if (playerCountry.infra.fossil_fuel_dependency < 0.0) playerCountry.infra.fossil_fuel_dependency = 0.0;
    }

    // --- STORM / HURRICANE EVENT ---
    if (playerCountry.infra.storm_active) {
        // Aftermath: lingering effects during recovery
        playerCountry.infra.storm_aftermath_turns--;

        // Reconstruction costs drain reserves
        double rebuild_cost = playerCountry.infra.storm_damage * 0.3; // 30% of damage per turn
        playerCountry.economy.international_reserves -= rebuild_cost;
        if (playerCountry.economy.international_reserves < 0)
            playerCountry.economy.debt_to_gdp_ratio += 0.02; // Must borrow

        // Blackout probability elevated during recovery
        playerCountry.infra.blackout_prob += 0.05;

        // Tourism suppressed
        playerCountry.economy.annual_visitors *= 0.85;

        std::cout << "[!!] STORM AFTERMATH (" << (playerCountry.infra.storm_aftermath_turns + 1)
                  << " turns left): Rebuilding. Cost so far: $"
                  << (long long)(playerCountry.infra.storm_damage) << std::endl;

        if (playerCountry.infra.storm_aftermath_turns <= 0) {
            playerCountry.infra.storm_active = false;
            std::cout << "[INFO] STORM RECOVERY COMPLETE: Infrastructure restored." << std::endl;
        }
    } else {
        // Stochastic storm trigger
        if (dist(rng) < playerCountry.infra.storm_prob) {
            playerCountry.infra.storm_active = true;
            std::uniform_int_distribution<int> after_dist(2, 3);
            playerCountry.infra.storm_aftermath_turns = after_dist(rng);

            // Severity determines damage
            std::uniform_real_distribution<double> sev_dist(0.3, 1.0);
            double severity = sev_dist(rng);

            // Infrastructure damage
            playerCountry.infra.road_connectivity -= severity * 0.1;
            if (playerCountry.infra.road_connectivity < 0.1) playerCountry.infra.road_connectivity = 0.1;
            playerCountry.infra.port_capacity -= severity * 0.08;
            if (playerCountry.infra.port_capacity < 0.05) playerCountry.infra.port_capacity = 0.05;
            playerCountry.infra.grid_resilience -= severity * 0.1;
            if (playerCountry.infra.grid_resilience < 0.1) playerCountry.infra.grid_resilience = 0.1;

            // Casualties: worse with poor infrastructure
            double vulnerability = 1.0 - playerCountry.infra.maintenance_level;
            playerCountry.infra.storm_casualties = severity * vulnerability * 500.0; // Up to hundreds
            playerCountry.welfare.death_rate += playerCountry.infra.storm_casualties / (double)playerCountry.welfare.population;

            // Damage cost
            playerCountry.infra.storm_damage = playerCountry.economy.gdp * severity * 0.03;

            // Immediate GDP hit
            playerCountry.economy.gdp -= playerCountry.infra.storm_damage * 0.5;

            // Popularity: good response helps (high health_coverage)
            if (playerCountry.welfare.health_coverage > 0.7) {
                playerCountry.politics.popularity += 0.01; // Rally effect
            } else {
                playerCountry.politics.popularity -= 0.03 * severity;
            }

            std::cout << "[!!!] HURRICANE/STORM: Category " << (int)(severity * 5)
                      << " storm hits! Casualties: " << (int)playerCountry.infra.storm_casualties
                      << ". Damage: $" << (long long)playerCountry.infra.storm_damage
                      << ". Recovery: " << playerCountry.infra.storm_aftermath_turns
                      << " turns." << std::endl;
        }
    }

    // --- FLOOD EVENT ---
    if (playerCountry.infra.flood_active) {
        playerCountry.infra.flood_duration--;
        double flood_dmg_rate = 0.02 * (1.0 - playerCountry.infra.maintenance_level);

        // Infrastructure heavily damaged
        playerCountry.infra.road_connectivity -= flood_dmg_rate;
        if (playerCountry.infra.road_connectivity < 0.1) playerCountry.infra.road_connectivity = 0.1;
        playerCountry.infra.port_capacity -= flood_dmg_rate * 0.5;
        if (playerCountry.infra.port_capacity < 0.05) playerCountry.infra.port_capacity = 0.05;

        // Disease outbreak risk
        playerCountry.welfare.epidemic_prob += 0.05;
        if (playerCountry.welfare.epidemic_prob > 0.5) playerCountry.welfare.epidemic_prob = 0.5;

        // Agricultural loss
        playerCountry.infra.crop_loss_pct += 0.1;
        if (playerCountry.infra.crop_loss_pct > 0.8) playerCountry.infra.crop_loss_pct = 0.8;

        // Displacement and GDP hit
        playerCountry.economy.gdp -= playerCountry.economy.gdp * 0.015;
        playerCountry.welfare.urban_population_ratio += 0.003; // Displacement
        playerCountry.politics.popularity -= 0.015;

        // Accumulate damage cost
        double turn_cost = playerCountry.economy.gdp * 0.01;
        playerCountry.infra.flood_damage += turn_cost;
        playerCountry.economy.international_reserves -= turn_cost;

        std::cout << "[!!] FLOOD (Turn " << (playerCountry.infra.flood_duration + 1)
                  << " remaining): Infrastructure damage ongoing. Cost: $"
                  << (long long)playerCountry.infra.flood_damage << std::endl;

        if (playerCountry.infra.flood_duration <= 0) {
            playerCountry.infra.flood_active = false;
            std::cout << "[INFO] FLOODWATERS RECEDE: Cleanup begins. Disease risk elevated." << std::endl;
        }
    } else {
        if (dist(rng) < playerCountry.infra.flood_prob) {
            playerCountry.infra.flood_active = true;
            std::uniform_int_distribution<int> dur_dist(1, 3);
            playerCountry.infra.flood_duration = dur_dist(rng);
            playerCountry.infra.flood_damage = 0.0;
            std::cout << "[!!] MAJOR FLOOD: Rivers overflowing. Duration: "
                      << playerCountry.infra.flood_duration << " turns." << std::endl;
        }
    }

    // --- TORNADO EVENT ---
    // Rare, localized, single-turn event
    if (dist(rng) < playerCountry.infra.tornado_prob) {
        std::uniform_real_distribution<double> sev_dist(0.2, 0.8);
        double severity = sev_dist(rng);

        // Localized infrastructure damage
        playerCountry.infra.road_connectivity -= severity * 0.03;
        if (playerCountry.infra.road_connectivity < 0.1) playerCountry.infra.road_connectivity = 0.1;

        // Concentrated casualties
        double vulnerability = 1.0 - playerCountry.infra.maintenance_level;
        playerCountry.infra.tornado_casualties = severity * vulnerability * 200.0;
        playerCountry.welfare.death_rate += playerCountry.infra.tornado_casualties / (double)playerCountry.welfare.population;

        // Damage cost (smaller than hurricane)
        playerCountry.infra.tornado_damage = playerCountry.economy.gdp * severity * 0.005;
        playerCountry.economy.international_reserves -= playerCountry.infra.tornado_damage;

        // Emergency response quality determines popularity impact
        if (playerCountry.welfare.health_coverage > 0.7 && playerCountry.infra.maintenance_level > 0.6) {
            playerCountry.politics.popularity += 0.005; // Good response
            std::cout << "[!] TORNADO: EF" << (int)(severity * 5) << " tornado strikes! Casualties: "
                      << (int)playerCountry.infra.tornado_casualties
                      << ". Emergency response effective." << std::endl;
        } else {
            playerCountry.politics.popularity -= 0.02 * severity;
            std::cout << "[!] TORNADO: EF" << (int)(severity * 5) << " tornado strikes! Casualties: "
                      << (int)playerCountry.infra.tornado_casualties
                      << ". Damage: $" << (long long)playerCountry.infra.tornado_damage
                      << ". Poor emergency response." << std::endl;
        }
    }

    // --- DROUGHT EVENT ---
    if (playerCountry.infra.drought_active) {
        playerCountry.infra.drought_duration--;
        double sev = playerCountry.infra.drought_severity;

        // Crop loss accumulates
        playerCountry.infra.crop_loss_pct += sev * 0.15;
        if (playerCountry.infra.crop_loss_pct > 0.9) playerCountry.infra.crop_loss_pct = 0.9;

        // Inflation: food prices spike
        playerCountry.economy.inflation += sev * 0.02;

        // Agricultural GDP hit (agri sector ~ agricultural_power proxy)
        playerCountry.economy.gdp -= playerCountry.economy.gdp * sev * 0.01;

        // Water scarcity: health impact
        playerCountry.welfare.health_coverage -= sev * 0.01;
        if (playerCountry.welfare.health_coverage < 0.05)
            playerCountry.welfare.health_coverage = 0.05;

        // Rural-to-urban migration
        playerCountry.welfare.urban_population_ratio += sev * 0.005;
        if (playerCountry.welfare.urban_population_ratio > 0.98)
            playerCountry.welfare.urban_population_ratio = 0.98;

        // Popularity hit: people blame government
        playerCountry.politics.popularity -= 0.015 * sev;

        // Prolonged drought → social unrest
        if (playerCountry.infra.crop_loss_pct > 0.6) {
            playerCountry.politics.protest_intensity += 0.03;
            std::cout << "[!!!] DROUGHT FAMINE RISK: Crop losses exceed 60%. Social unrest rising." << std::endl;
        }

        std::cout << "[!!] DROUGHT (Turn " << (playerCountry.infra.drought_duration + 1)
                  << " remaining): Crop loss " << (int)(playerCountry.infra.crop_loss_pct * 100)
                  << "%. Severity " << (int)(sev * 100) << "%." << std::endl;

        if (playerCountry.infra.drought_duration <= 0) {
            playerCountry.infra.drought_active = false;
            std::cout << "[INFO] DROUGHT ENDED: Rains return. Crop recovery will take time." << std::endl;
            // Slow crop recovery over subsequent turns
            playerCountry.infra.crop_loss_pct *= 0.5; // Halve immediately, rest recovers naturally
        }
    } else {
        // Natural crop recovery
        if (playerCountry.infra.crop_loss_pct > 0.0) {
            playerCountry.infra.crop_loss_pct -= 0.05;
            if (playerCountry.infra.crop_loss_pct < 0.0) playerCountry.infra.crop_loss_pct = 0.0;
        }
        // Stochastic drought trigger
        if (dist(rng) < playerCountry.infra.drought_prob) {
            playerCountry.infra.drought_active = true;
            std::uniform_int_distribution<int> dur_dist(2, 6);
            playerCountry.infra.drought_duration = dur_dist(rng);
            std::uniform_real_distribution<double> sev_dist(0.2, 1.0);
            playerCountry.infra.drought_severity = sev_dist(rng);
            playerCountry.infra.crop_loss_pct = 0.0;
            std::cout << "[!!] DROUGHT BEGINS: Rainfall has plummeted. Severity: "
                      << (int)(playerCountry.infra.drought_severity * 100)
                      << "%. Expected duration: " << playerCountry.infra.drought_duration
                      << " turns." << std::endl;
        }
    }

    // --- MEDIA & PROPAGANDA DYNAMICS ---
    // Media pluralism: inverse of control + ownership concentration
    playerCountry.security.media_pluralism = 1.0 - playerCountry.security.media_control * 0.5
                                           - playerCountry.politics.media_ownership_concentration * 0.3;
    if (playerCountry.security.media_pluralism < 0.05) playerCountry.security.media_pluralism = 0.05;
    // Narrative reach: composite of channels
    playerCountry.security.narrative_reach = playerCountry.security.social_media_reach * 0.5
                                           + playerCountry.security.traditional_media_reach * 0.35
                                           + playerCountry.security.diaspora_narrative_reach * 0.15;
    if (playerCountry.security.narrative_reach > 1.0) playerCountry.security.narrative_reach = 1.0;
    // Social media reach scales with internet coverage
    playerCountry.security.social_media_reach = playerCountry.infra.internet_coverage * 0.8;
    // Fake news success: inversely related to media literacy and fact-checking
    playerCountry.security.fake_news_success_prob = 0.1 + playerCountry.security.foreign_disinfo_operations * 0.3
                                                  + (1.0 - playerCountry.security.media_literacy) * 0.25
                                                  - playerCountry.security.fact_checking_ecosystem * 0.15;
    if (playerCountry.security.fake_news_success_prob < 0.02) playerCountry.security.fake_news_success_prob = 0.02;
    if (playerCountry.security.fake_news_success_prob > 0.8) playerCountry.security.fake_news_success_prob = 0.8;
    // Media literacy improves with education
    playerCountry.security.media_literacy = playerCountry.welfare.educational_quality * 0.5
                                          + playerCountry.welfare.secondary_enrollment * 0.3;
    if (playerCountry.security.media_literacy > 1.0) playerCountry.security.media_literacy = 1.0;
    // Press freedom: composite index
    playerCountry.security.press_freedom = playerCountry.security.journalist_safety * 0.3
                                         + playerCountry.security.media_pluralism * 0.3
                                         + (1.0 - playerCountry.security.legal_harassment_index) * 0.2
                                         + (1.0 - playerCountry.security.self_censorship_rate) * 0.2;
    if (playerCountry.security.press_freedom > 1.0) playerCountry.security.press_freedom = 1.0;
    // Jailing journalists worsens self-censorship and safety
    if (playerCountry.security.journalists_jailed > 0) {
        playerCountry.security.self_censorship_rate += playerCountry.security.journalists_jailed * 0.02;
        if (playerCountry.security.self_censorship_rate > 0.9) playerCountry.security.self_censorship_rate = 0.9;
        playerCountry.security.journalist_safety -= playerCountry.security.journalists_jailed * 0.03;
        if (playerCountry.security.journalist_safety < 0.1) playerCountry.security.journalist_safety = 0.1;
    }

    // --- MEDIA / PROPAGANDA CRISIS ---
    if (playerCountry.security.media_crisis_active) {
        playerCountry.security.media_crisis_duration--;

        // Disinformation spiral
        playerCountry.politics.polarization_index += 0.04;
        if (playerCountry.politics.polarization_index > 1.0)
            playerCountry.politics.polarization_index = 1.0;
        playerCountry.politics.affective_polarization += 0.03;
        playerCountry.politics.media_echo_chamber += 0.03;

        // International press condemnation
        playerCountry.security.press_freedom -= 0.03;
        if (playerCountry.security.press_freedom < 0.05)
            playerCountry.security.press_freedom = 0.05;
        playerCountry.security.diplomatic_prestige -= 0.02;

        // Public trust collapse
        playerCountry.politics.protest_intensity += 0.02;
        playerCountry.welfare.un_score -= 0.02;

        // Media literacy degrades
        playerCountry.security.media_literacy -= 0.02;
        if (playerCountry.security.media_literacy < 0.1)
            playerCountry.security.media_literacy = 0.1;

        playerCountry.politics.popularity -= 0.01;

        std::cout << "[!!] MEDIA CRISIS (Turn " << (playerCountry.security.media_crisis_duration + 1)
                  << " remaining): Disinformation spiral. Polarization: "
                  << (int)(playerCountry.politics.polarization_index * 100) << "%" << std::endl;

        if (playerCountry.security.media_crisis_duration <= 0) {
            playerCountry.security.media_crisis_active = false;
            std::cout << "[INFO] MEDIA CRISIS SUBSIDES: Fact-checkers regaining ground." << std::endl;
        }
    } else {
        if (playerCountry.security.press_freedom < 0.3
            && dist(rng) < playerCountry.security.fake_news_success_prob) {
            playerCountry.security.media_crisis_active = true;
            std::uniform_int_distribution<int> dur_dist(2, 3);
            playerCountry.security.media_crisis_duration = dur_dist(rng);

            // Foreign disinfo involvement
            if (playerCountry.security.foreign_disinfo_operations > 0.3) {
                std::cout << "[!!!] MEDIA CRISIS: Foreign-backed disinformation campaign destabilizing public discourse!"
                          << std::endl;
            } else {
                std::cout << "[!!] MEDIA CRISIS: Domestic propaganda spiral — public trust in media collapsing!"
                          << std::endl;
            }
        }
    }

    // --- DEFENSE & MILITARY DYNAMICS ---
    // Troop morale driven by veteran benefits, combat stress, and pay (personnel budget)
    playerCountry.security.troop_morale = 0.5 + playerCountry.security.veteran_benefit_adequacy * 0.2
                                        + playerCountry.security.personnel_vs_equipment_ratio * 0.15
                                        - playerCountry.security.combat_stress_index * 0.2;
    if (playerCountry.security.troop_morale < 0.1) playerCountry.security.troop_morale = 0.1;
    if (playerCountry.security.troop_morale > 1.0) playerCountry.security.troop_morale = 1.0;
    // Desertion rises with low morale and active conflict
    playerCountry.security.desertion_rate = (1.0 - playerCountry.security.troop_morale) * 0.02
                                          + playerCountry.security.conflict_with_groups * 0.01;
    // Combat stress from active conflict
    playerCountry.security.combat_stress_index = playerCountry.security.conflict_with_groups * 0.3
                                               + playerCountry.security.conflict_casualties_annual * 0.0002;
    if (playerCountry.security.combat_stress_index > 1.0) playerCountry.security.combat_stress_index = 1.0;
    // Equipment readiness: spending, corruption, and domestic production
    double effective_defense_spend = playerCountry.security.military_spending_gdp * playerCountry.economy.gdp
                                   * (1.0 - playerCountry.security.corruption_in_procurement);
    playerCountry.security.equipment_readiness = playerCountry.security.equipment_modernization * 0.6
                                               + playerCountry.security.defense_industrial_base * 0.2
                                               + (effective_defense_spend > 0 ? 0.2 : 0.0);
    if (playerCountry.security.equipment_readiness > 1.0) playerCountry.security.equipment_readiness = 1.0;
    // Naval: blue water capability requires submarines + modernization
    playerCountry.security.sea_lane_control = playerCountry.security.naval_force_strength * 0.6
                                            + playerCountry.security.submarines * 0.02;
    if (playerCountry.security.sea_lane_control > 1.0) playerCountry.security.sea_lane_control = 1.0;
    playerCountry.security.blue_water_capability = (playerCountry.security.naval_force_strength > 0.7
                                                   && playerCountry.security.submarines > 3);
    // Politicized military erodes civilian control → feeds coup risk
    playerCountry.politics.civilian_military_control -= playerCountry.security.politicized_officer_corps * 0.005
                                                     + playerCountry.security.military_business_interests * 0.003;
    if (playerCountry.politics.civilian_military_control < 0.1) playerCountry.politics.civilian_military_control = 0.1;
    if (playerCountry.politics.civilian_military_control > 1.0) playerCountry.politics.civilian_military_control = 1.0;

    // --- MILITARY MUTINY CRISIS ---
    if (playerCountry.security.military_mutiny_active) {
        playerCountry.security.mutiny_duration--;

        // Coup probability spikes during mutiny
        playerCountry.politics.coup_d_etat_prob += 0.05;
        if (playerCountry.politics.coup_d_etat_prob > 0.5)
            playerCountry.politics.coup_d_etat_prob = 0.5;

        // Defense capability compromised
        playerCountry.security.equipment_modernization -= 0.02;
        if (playerCountry.security.equipment_modernization < 0.1)
            playerCountry.security.equipment_modernization = 0.1;

        // Civilian control erodes
        playerCountry.politics.civilian_military_control -= 0.05;
        if (playerCountry.politics.civilian_military_control < 0.1)
            playerCountry.politics.civilian_military_control = 0.1;

        // International weakness exposed
        playerCountry.security.alliance_protection -= 0.02;
        if (playerCountry.security.alliance_protection < 0.0)
            playerCountry.security.alliance_protection = 0.0;

        playerCountry.politics.popularity -= 0.02;

        std::cout << "[!!!] MILITARY MUTINY (Turn " << (playerCountry.security.mutiny_duration + 1)
                  << " remaining): Barracks in revolt. Coup risk elevated." << std::endl;

        if (playerCountry.security.mutiny_duration <= 0) {
            // Suppression check
            if (playerCountry.politics.civilian_military_control > 0.4) {
                playerCountry.security.military_mutiny_active = false;
                playerCountry.security.troop_morale += 0.05; // Purge restores some order
                std::cout << "[INFO] MUTINY SUPPRESSED: Disloyal officers purged. Order restored." << std::endl;
            } else {
                // Escalates to coup attempt
                playerCountry.security.military_mutiny_active = false;
                if (dist(rng) < playerCountry.politics.coup_d_etat_prob) {
                    if (dist(rng) < playerCountry.politics.coup_success_prob) {
                        std::cout << "[!!!!!] MUTINY BECOMES COUP: Military seizes power!" << std::endl;
                        std::cout << "GAME OVER." << std::endl;
                        exit(0);
                    } else {
                        playerCountry.politics.coup_attempts_history++;
                        std::cout << "[!!!!] FAILED COUP FROM MUTINY: Mutineers attempted to seize power but failed." << std::endl;
                    }
                } else {
                    std::cout << "[INFO] MUTINY DISSIPATES: Officers return to barracks grudgingly." << std::endl;
                }
            }
        }
    } else {
        if (playerCountry.security.troop_morale < 0.4
            && dist(rng) < playerCountry.security.military_insubordination_prob) {
            playerCountry.security.military_mutiny_active = true;
            std::uniform_int_distribution<int> dur_dist(1, 3);
            playerCountry.security.mutiny_duration = dur_dist(rng);
            std::cout << "[!!!] MILITARY MUTINY: Troops refuse orders! Barracks revolt. Morale: "
                      << (int)(playerCountry.security.troop_morale * 100) << "%" << std::endl;
        }
    }

    // --- INTELLIGENCE DYNAMICS ---
    // HUMINT scales with espionage budget and informant network
    playerCountry.security.humint_capability = playerCountry.security.informant_network * 0.5
                                             + (playerCountry.security.espionage_budget / 2000000.0) * 0.3;
    if (playerCountry.security.humint_capability > 1.0) playerCountry.security.humint_capability = 1.0;
    // SIGINT scales with tech + cyber capability
    playerCountry.security.sigint_capability = playerCountry.security.cyber_surveillance * 0.5
                                             + playerCountry.infra.innovation_index * 0.3;
    if (playerCountry.security.sigint_capability > 1.0) playerCountry.security.sigint_capability = 1.0;
    // Informant penetration of NSGs: scales with network * budget
    playerCountry.security.informant_penetration_nsg = playerCountry.security.informant_network * 0.4
                                                     + playerCountry.security.humint_capability * 0.3;
    if (playerCountry.security.informant_penetration_nsg > 0.9) playerCountry.security.informant_penetration_nsg = 0.9;
    // Attack detection: composite of all intelligence capabilities
    playerCountry.security.attack_detection_prob = playerCountry.security.humint_capability * 0.3
                                                 + playerCountry.security.sigint_capability * 0.3
                                                 + playerCountry.security.foreign_intelligence_sharing * 0.2
                                                 + playerCountry.security.informant_reliability * 0.2;
    if (playerCountry.security.attack_detection_prob > 0.95) playerCountry.security.attack_detection_prob = 0.95;
    // Terrorism detection
    playerCountry.security.terrorism_detection_prob = playerCountry.security.attack_detection_prob * 0.9
                                                   + playerCountry.security.informant_penetration_nsg * 0.1;
    // Cyber attack detection
    playerCountry.security.cyberattack_detection_prob = playerCountry.security.sigint_capability * 0.5
                                                      + playerCountry.security.cyber_surveillance * 0.3;
    if (playerCountry.security.cyberattack_detection_prob > 0.95) playerCountry.security.cyberattack_detection_prob = 0.95;
    // Early warning time scales inversely with detection quality
    playerCountry.security.early_warning_time_hours = 12.0 + playerCountry.security.attack_detection_prob * 60.0;
    // Mass surveillance: active if cyber_surveillance high and no press freedom check
    if (playerCountry.security.cyber_surveillance > 0.6 && playerCountry.security.press_freedom < 0.5)
        playerCountry.security.mass_surveillance_active = true;
    else
        playerCountry.security.mass_surveillance_active = false;

    // --- INTELLIGENCE CRISIS (Espionage Scandal) ---
    if (playerCountry.security.intel_crisis_active) {
        playerCountry.security.intel_crisis_duration--;

        // Spy network compromised
        playerCountry.security.informant_network -= 0.05;
        if (playerCountry.security.informant_network < 0.0)
            playerCountry.security.informant_network = 0.0;
        playerCountry.security.informant_reliability -= 0.03;
        if (playerCountry.security.informant_reliability < 0.1)
            playerCountry.security.informant_reliability = 0.1;

        // Foreign relations strained
        playerCountry.security.diplomatic_prestige -= 0.03;
        if (playerCountry.security.diplomatic_prestige < 0.0)
            playerCountry.security.diplomatic_prestige = 0.0;
        playerCountry.security.foreign_intelligence_sharing -= 0.05;
        if (playerCountry.security.foreign_intelligence_sharing < 0.0)
            playerCountry.security.foreign_intelligence_sharing = 0.0;

        // Domestic scandal
        playerCountry.politics.popularity -= 0.02;
        playerCountry.politics.trust_in_justice -= 0.02;

        // Sanctions risk
        playerCountry.economy.international_sanctions_prob += 0.02;

        std::cout << "[!!!] INTELLIGENCE CRISIS (Turn " << (playerCountry.security.intel_crisis_duration + 1)
                  << " remaining): Spy networks exposed. Intelligence sharing frozen." << std::endl;

        if (playerCountry.security.intel_crisis_duration <= 0) {
            playerCountry.security.intel_crisis_active = false;
            std::cout << "[INFO] INTELLIGENCE CRISIS RESOLVED: Agencies restructuring." << std::endl;
        }
    } else {
        if (dist(rng) < playerCountry.security.document_leak_prob) {
            playerCountry.security.intel_crisis_active = true;
            std::uniform_int_distribution<int> dur_dist(2, 3);
            playerCountry.security.intel_crisis_duration = dur_dist(rng);
            std::cout << "[!!!] ESPIONAGE SCANDAL: Classified documents leaked! Intelligence operations compromised."
                      << std::endl;
        }
    }

    // --- SECURITY & VIOLENCE DYNAMICS ---
    // Non-state groups composition: total = guerrillas + criminal orgs + militia
    playerCountry.security.non_state_groups = playerCountry.security.guerrilla_groups
                                            + playerCountry.security.criminal_organizations
                                            + playerCountry.security.militia_groups;
    // Territorial control scales with criminal org strength and impunity
    playerCountry.security.nsg_territorial_control = playerCountry.security.criminal_organizations * 0.02
                                                   + playerCountry.politics.impunity * 0.05;
    if (playerCountry.security.nsg_territorial_control > 0.5) playerCountry.security.nsg_territorial_control = 0.5;
    // Conflict casualties from active confrontation
    playerCountry.security.conflict_casualties_annual = (int)(playerCountry.security.conflict_with_groups
                                                            * playerCountry.security.non_state_groups * 80);
    // Ceasefire reduces casualties
    if (playerCountry.security.ceasefire_active)
        playerCountry.security.conflict_casualties_annual = (int)(playerCountry.security.conflict_casualties_annual * 0.2);
    // Homicide decomposition: organized crime + domestic + state
    // Organized crime share grows with impunity and NSG strength
    playerCountry.security.organized_crime_homicide_share = 0.2 + playerCountry.politics.impunity_organized_crime * 0.3
                                                          + playerCountry.security.nsg_territorial_control * 0.3;
    if (playerCountry.security.organized_crime_homicide_share > 0.8) playerCountry.security.organized_crime_homicide_share = 0.8;
    // Weapons drive overall homicide rate
    playerCountry.security.homicide_rate = 5.0 + playerCountry.security.weapons_in_population * 15.0
                                         + playerCountry.security.illegal_weapons_share * 10.0
                                         + playerCountry.security.nsg_territorial_control * 20.0
                                         - playerCountry.politics.trust_in_justice * 5.0;
    if (playerCountry.security.homicide_rate < 1.0) playerCountry.security.homicide_rate = 1.0;
    // Femicide correlates with domestic violence and impunity
    playerCountry.security.femicide_rate = 1.0 + (1.0 - playerCountry.welfare.minority_protection) * 4.0
                                         + playerCountry.politics.impunity * 3.0;
    // Mass shooting driven by weapons access + mental health
    playerCountry.security.mass_shooting_prob = playerCountry.security.weapons_in_population * 0.01
                                              + playerCountry.security.military_grade_weapons_civilian * 0.05
                                              + (1.0 - playerCountry.welfare.mental_health_index) * 0.005;

    // --- NEIGHBOR COUNTRIES DYNAMICS ---
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        for (auto& neighbor : playerCountry.neighbors) {
            // Relations drift based on ideology alignment
            double ideology_gap = std::abs(playerCountry.politics.economic_ideology - neighbor.ideology);
            if (ideology_gap > 0.5) {
                neighbor.diplomatic_relations -= 0.01;
            } else if (ideology_gap < 0.2) {
                neighbor.diplomatic_relations += 0.005;
            }
            // Clamp
            if (neighbor.diplomatic_relations > 1.0) neighbor.diplomatic_relations = 1.0;
            if (neighbor.diplomatic_relations < -1.0) neighbor.diplomatic_relations = -1.0;

            // Trade volume proportional to relations and openness
            double trade_potential = neighbor.gdp * 0.05 * playerCountry.economy.trade_openness;
            if (neighbor.diplomatic_relations > 0.3) {
                neighbor.trade_volume += (trade_potential - neighbor.trade_volume) * 0.1;
            } else if (neighbor.diplomatic_relations < -0.3) {
                neighbor.trade_volume *= 0.95; // Trade declines with hostility
            }
            if (neighbor.sanctions_against_us) neighbor.trade_volume *= 0.5;

            // Hostile neighbor effects
            if (neighbor.diplomatic_relations < -0.5) {
                playerCountry.security.invasion_prob += 0.005;
                // Espionage against us
                if (dist(rng) < 0.1) {
                    playerCountry.security.document_leak_prob += 0.01;
                }
                // Border incidents
                if (dist(rng) < 0.05) {
                    neighbor.diplomatic_relations -= 0.05;
                    playerCountry.security.conflict_casualties_annual += 5;
                    std::cout << "[NEIGHBOR] Border skirmish with " << neighbor.name
                              << "! Relations deteriorating." << std::endl;
                }
            }

            // Allied neighbor benefits
            if (neighbor.diplomatic_relations > 0.5) {
                playerCountry.security.foreign_intelligence_sharing += 0.002;
                if (playerCountry.security.foreign_intelligence_sharing > 1.0)
                    playerCountry.security.foreign_intelligence_sharing = 1.0;
            }

            // Neighbor economy affects our trade
            neighbor.gdp *= (1.0 + neighbor.economic_growth);
            if (neighbor.in_crisis) {
                neighbor.economic_growth = -0.02;
                neighbor.refugee_generation += 0.01;
                // Refugee pressure
                if (neighbor.refugee_generation > 0.05) {
                    playerCountry.security.mass_migration_prob += 0.02;
                }
            } else {
                // Random neighbor crisis
                if (dist(rng) < 0.03) {
                    neighbor.in_crisis = true;
                    std::cout << "[NEIGHBOR] " << neighbor.name << " enters economic crisis! "
                              << "Refugee flows and trade disruption expected." << std::endl;
                }
            }
            // Neighbor crisis recovery
            if (neighbor.in_crisis && dist(rng) < 0.15) {
                neighbor.in_crisis = false;
                neighbor.economic_growth = 0.01;
                neighbor.refugee_generation = 0.0;
                std::cout << "[NEIGHBOR] " << neighbor.name << " stabilizes. Trade normalizing." << std::endl;
            }

            // Territorial claim → invasion risk
            if (neighbor.has_territorial_claim && neighbor.diplomatic_relations < -0.7
                && neighbor.military_strength > 0.5 && dist(rng) < 0.03) {
                std::cout << "[!!!] " << neighbor.name << " masses troops on border! Invasion threat!" << std::endl;
                playerCountry.security.invasion_prob += 0.05;
                playerCountry.security.territorial_dispute_intensity += 0.1;
            }

            // --- NEIGHBOR WAR SYSTEM ---
            // Hostile neighbor with territorial claim may declare war
            if (!neighbor.at_war && neighbor.has_territorial_claim
                && neighbor.diplomatic_relations < -0.8
                && neighbor.military_strength > playerCountry.security.military_spending_gdp * 20.0
                && dist(rng) < 0.02) {
                neighbor.at_war = true;
                neighbor.diplomatic_relations = -1.0;
                neighbor.trade_volume *= 0.1;
                neighbor.sanctions_against_us = true;
                playerCountry.security.war_active = true;
                playerCountry.security.war_duration = 0;
                playerCountry.security.equipment_readiness -= 0.1;
                std::cout << "[WAR] " << neighbor.name << " has DECLARED WAR! Territorial invasion underway!" << std::endl;
                std::cout << "[WAR] Trade collapses. Military mobilization required." << std::endl;
            }

            // Active war with neighbor
            if (neighbor.at_war) {
                playerCountry.security.war_duration++;
                double war_expense = playerCountry.economy.gdp * 0.02;
                playerCountry.economy.gdp -= war_expense;
                playerCountry.security.war_cost += war_expense;
                playerCountry.security.combat_stress_index += 0.03;
                if (playerCountry.security.combat_stress_index > 1.0) playerCountry.security.combat_stress_index = 1.0;
                playerCountry.politics.popularity -= 0.02 * playerCountry.security.combat_stress_index;

                // Military outcome based on strength comparison
                double our_power = playerCountry.security.military_spending_gdp * 30.0 * playerCountry.security.equipment_readiness;
                double their_power = neighbor.military_strength * 0.8;
                double battle_result = dist(rng);

                if (battle_result < our_power / (our_power + their_power + 0.01)) {
                    neighbor.military_strength -= 0.05;
                    if (neighbor.military_strength < 0.1) neighbor.military_strength = 0.1;
                    playerCountry.security.war_casualties += 50.0;
                    std::cout << "[WAR] Our forces repel " << neighbor.name << " advance." << std::endl;

                    // Peace treaty if they're weakened enough
                    if (neighbor.military_strength < 0.2 || (playerCountry.security.combat_stress_index > 0.6 && dist(rng) < 0.3)) {
                        neighbor.at_war = false;
                        neighbor.diplomatic_relations = -0.3;
                        neighbor.has_territorial_claim = false;
                        neighbor.sanctions_against_us = false;
                        neighbor.trade_volume = neighbor.gdp * 0.05;
                        playerCountry.security.war_active = false;
                        playerCountry.security.diplomatic_prestige += 0.1;
                        std::cout << "[PEACE] " << neighbor.name << " accepts peace treaty. Territorial claim renounced." << std::endl;
                    }
                } else {
                    playerCountry.security.equipment_readiness -= 0.03;
                    playerCountry.security.territorial_dispute_intensity += 0.05;
                    playerCountry.security.war_casualties += 150.0;
                    if (playerCountry.security.equipment_readiness < 0.2) playerCountry.security.equipment_readiness = 0.2;
                    std::cout << "[WAR] " << neighbor.name << " forces advance. Casualties mounting." << std::endl;

                    // Forced peace if we're too weak
                    if (playerCountry.security.equipment_readiness < 0.25 && playerCountry.security.combat_stress_index > 0.8) {
                        neighbor.at_war = false;
                        neighbor.diplomatic_relations = -0.5;
                        neighbor.trade_volume = neighbor.gdp * 0.02;
                        playerCountry.security.war_active = false;
                        playerCountry.security.diplomatic_prestige -= 0.15;
                        playerCountry.politics.popularity -= 0.1;
                        std::cout << "[DEFEAT] Forced to accept unfavorable peace with " << neighbor.name << "." << std::endl;
                    }
                }

                // Proxy conflicts: neighbor at war supports our non-state groups
                if (dist(rng) < 0.15) {
                    playerCountry.security.nsg_territorial_control += 0.02;
                    std::cout << "[PROXY] " << neighbor.name << " funneling arms to rebel groups." << std::endl;
                }
            }
        }

        // --- BILATERAL TRADE EFFECTS ---
        double total_neighbor_trade = 0.0;
        double total_refugee_pressure = 0.0;
        for (auto& neighbor : playerCountry.neighbors) {
            // Sanctions from neighbor reduce trade volume sharply
            if (neighbor.sanctions_against_us) {
                neighbor.trade_volume *= 0.92; // -8% per turn under sanctions
                if (neighbor.trade_volume < 1000000.0) neighbor.trade_volume = 1000000.0;
            }

            // FTA with allied neighbor boosts bilateral trade
            if (neighbor.diplomatic_relations > 0.6 && playerCountry.economy.trade_openness > 0.5) {
                double fta_boost = neighbor.trade_volume * 0.02 * neighbor.diplomatic_relations;
                neighbor.trade_volume += fta_boost;
            }

            // Neighbor in recession → our exports to them drop
            if (neighbor.in_crisis || neighbor.economic_growth < -0.01) {
                double export_loss = neighbor.trade_volume * 0.05;
                neighbor.trade_volume -= export_loss;
                playerCountry.economy.gdp -= export_loss * 0.3; // Spillover to our GDP
                if (neighbor.trade_volume < 5000000.0) neighbor.trade_volume = 5000000.0;
            }

            // Neighbor refugee crisis → migration pressure on us
            if (neighbor.in_crisis && neighbor.refugee_generation > 0) {
                double refugees = neighbor.refugee_generation * 0.1; // 10% flow to us
                total_refugee_pressure += refugees;
                playerCountry.welfare.population += (int)(refugees);
                playerCountry.welfare.unemployment_rate += refugees / playerCountry.welfare.population * 2.0;
                playerCountry.politics.popularity -= 0.002 * (refugees / 10000.0);
            }

            total_neighbor_trade += neighbor.trade_volume;
        }

        // Net contribution from neighbor trade to trade balance
        playerCountry.economy.trade_balance += total_neighbor_trade * 0.01;

        // Refugee pressure narrative
        if (total_refugee_pressure > 5000) {
            std::cout << "[MIGRATION] Refugee influx from neighbors: " << (int)total_refugee_pressure
                      << " people. Social tensions rising." << std::endl;
            playerCountry.politics.popularity -= 0.01;
        }
    }

    // --- GEOPOLITICS & INTERNATIONAL RELATIONS ---
    // IDEOLOGY → INTERNATIONAL ALIGNMENT
    // Left-leaning → China alignment drift, Right-leaning → US alignment drift
    if (playerCountry.politics.economic_ideology < 0.35) {
        playerCountry.security.china_alignment += 0.005;
        if (playerCountry.security.china_alignment > 1.0) playerCountry.security.china_alignment = 1.0;
        playerCountry.security.us_alignment -= 0.003;
        if (playerCountry.security.us_alignment < 0.0) playerCountry.security.us_alignment = 0.0;
    } else if (playerCountry.politics.economic_ideology > 0.65) {
        playerCountry.security.us_alignment += 0.005;
        if (playerCountry.security.us_alignment > 1.0) playerCountry.security.us_alignment = 1.0;
        playerCountry.security.china_alignment -= 0.003;
        if (playerCountry.security.china_alignment < 0.0) playerCountry.security.china_alignment = 0.0;
    }
    // Authoritarian shift → diplomatic prestige erodes, sanctions risk increases
    if (playerCountry.politics.auth_dem_axis > 0.7) {
        playerCountry.security.diplomatic_prestige -= 0.005;
        if (playerCountry.security.diplomatic_prestige < 0.05) playerCountry.security.diplomatic_prestige = 0.05;
        playerCountry.economy.international_sanctions_prob += 0.005;
    }
    // Strong left ideology → capital flight risk and FDI caution
    if (playerCountry.politics.economic_ideology < 0.25) {
        playerCountry.infra.capital_flight_risk += 0.005;
        playerCountry.infra.fdi_inflow_gdp -= 0.001;
        if (playerCountry.infra.fdi_inflow_gdp < 0.0) playerCountry.infra.fdi_inflow_gdp = 0.0;
    }
    // Non-alignment: inverse of how much you lean towards either bloc
    playerCountry.security.non_aligned_index = 1.0 - (playerCountry.security.us_alignment + playerCountry.security.china_alignment) / 2.0;
    if (playerCountry.security.non_aligned_index < 0.0) playerCountry.security.non_aligned_index = 0.0;
    // Geopolitical rent: great powers pay for alignment
    playerCountry.security.geopolitical_rent = playerCountry.security.us_alignment * playerCountry.economy.gdp * 0.005
                                             + playerCountry.security.china_alignment * playerCountry.economy.gdp * 0.004;
    playerCountry.economy.international_reserves += playerCountry.security.geopolitical_rent;
    // Emigration push: poverty + conflict + low freedom
    playerCountry.security.emigration_push_index = playerCountry.welfare.poverty_rate * 0.4
                                                 + playerCountry.security.conflict_with_groups * 0.3
                                                 + (1.0 - playerCountry.welfare.freedom_of_expression) * 0.3;
    if (playerCountry.security.emigration_push_index > 1.0) playerCountry.security.emigration_push_index = 1.0;
    playerCountry.security.mass_migration_prob = playerCountry.security.emigration_push_index * 0.15;
    // Nuclear deterrence
    if (playerCountry.security.nuclear_umbrella || playerCountry.security.own_nuclear_capability)
        playerCountry.security.nuclear_attack_prob *= 0.1;
    // Invasion probability: territorial disputes + weak alliances
    playerCountry.security.invasion_prob = playerCountry.security.hostile_neighbors * 0.005
                                         + playerCountry.security.territorial_dispute_intensity * 0.05
                                         - playerCountry.security.alliance_protection * 0.03;
    if (playerCountry.security.invasion_prob < 0.001) playerCountry.security.invasion_prob = 0.001;

    // --- WAR / INVASION EVENT ---
    if (playerCountry.security.war_active) {
        playerCountry.security.war_duration++;

        // Military strength: morale * equipment
        double defense_strength = playerCountry.security.troop_morale * 0.5
                                + playerCountry.security.equipment_modernization * 0.3
                                + playerCountry.security.alliance_protection * 0.2;

        // Casualties each turn of war
        double turn_casualties = (1.0 - defense_strength) * 2000.0;
        playerCountry.security.war_casualties += turn_casualties;
        playerCountry.welfare.death_rate += turn_casualties / (double)playerCountry.welfare.population;

        // Massive economic drain
        double war_gdp_cost = playerCountry.economy.gdp * 0.06;
        playerCountry.economy.gdp -= war_gdp_cost;
        playerCountry.security.war_cost += war_gdp_cost;
        playerCountry.economy.international_reserves -= war_gdp_cost * 0.5;
        playerCountry.economy.debt_to_gdp_ratio += 0.03;

        // Conscription: unemployment drops but productivity crashes
        playerCountry.welfare.unemployment_rate -= 0.02;
        if (playerCountry.welfare.unemployment_rate < 0.01)
            playerCountry.welfare.unemployment_rate = 0.01;

        // Rally-around-the-flag (initial turns) vs war fatigue (later)
        if (playerCountry.security.war_duration <= 2) {
            playerCountry.politics.popularity += 0.03; // Patriotic surge
        } else {
            playerCountry.politics.popularity -= 0.04; // War fatigue
        }

        // Refugee crisis from war
        playerCountry.security.mass_migration_prob += 0.05;

        // Can we repel? Check each turn
        if (defense_strength > 0.6 && playerCountry.security.war_duration >= 2) {
            if (dist(rng) < defense_strength * 0.3) {
                playerCountry.security.war_active = false;
                playerCountry.security.invasion_repelled = true;
                playerCountry.security.diplomatic_prestige += 0.15;
                playerCountry.politics.popularity += 0.10; // Victory boost
                std::cout << "[!!!] INVASION REPELLED! Victory after " << playerCountry.security.war_duration
                          << " turns. Casualties: " << (int)playerCountry.security.war_casualties
                          << ". Cost: $" << (long long)playerCountry.security.war_cost << std::endl;
            }
        }

        // Prolonged war → GAME OVER risk
        if (playerCountry.security.war_active && playerCountry.security.war_duration > 8) {
            if (dist(rng) < 0.3) {
                std::cout << "[!!!!!] TOTAL DEFEAT: The country has been overrun after "
                          << playerCountry.security.war_duration << " turns of war." << std::endl;
                std::cout << "GAME OVER." << std::endl;
                exit(0);
            }
        }

        if (playerCountry.security.war_active) {
            std::cout << "[!!!!] WAR (Turn " << playerCountry.security.war_duration
                      << "): Casualties: " << (int)playerCountry.security.war_casualties
                      << ". Cost: $" << (long long)playerCountry.security.war_cost
                      << ". Defense strength: " << (int)(defense_strength * 100) << "%" << std::endl;
        }
    } else {
        // Stochastic invasion trigger
        if (dist(rng) < playerCountry.security.invasion_prob) {
            playerCountry.security.war_active = true;
            playerCountry.security.war_duration = 0;
            playerCountry.security.war_casualties = 0.0;
            playerCountry.security.war_cost = 0.0;
            playerCountry.security.invasion_repelled = false;
            std::cout << "[!!!!] INVASION: A hostile neighbor has launched a military attack!" << std::endl;
            std::cout << "       Mobilizing defense forces. Alliance protection: "
                      << (int)(playerCountry.security.alliance_protection * 100) << "%" << std::endl;
        }
    }

    // --- NUCLEAR STRIKE EVENT ---
    if (playerCountry.security.nuclear_strike) {
        // Long-term contamination effects (persists after strike)
        playerCountry.security.nuclear_contamination -= 0.01; // Very slow decay
        if (playerCountry.security.nuclear_contamination > 0.0) {
            playerCountry.welfare.food_radiation_prob = playerCountry.security.nuclear_contamination;
            playerCountry.welfare.death_rate += playerCountry.security.nuclear_contamination * 0.005;
            playerCountry.welfare.health_coverage -= 0.02;
            if (playerCountry.welfare.health_coverage < 0.05)
                playerCountry.welfare.health_coverage = 0.05;
            std::cout << "[!!!!] NUCLEAR FALLOUT: Contamination level "
                      << (int)(playerCountry.security.nuclear_contamination * 100)
                      << "%. Ongoing health crisis." << std::endl;
        } else {
            playerCountry.security.nuclear_strike = false;
            playerCountry.security.nuclear_contamination = 0.0;
            std::cout << "[INFO] NUCLEAR CONTAMINATION CLEARED: Long recovery ahead." << std::endl;
        }
    } else if (dist(rng) < playerCountry.security.nuclear_attack_prob) {
        playerCountry.security.nuclear_strike = true;

        // Catastrophic immediate damage
        playerCountry.security.nuclear_casualties = playerCountry.welfare.population * 0.05; // 5% immediate dead
        playerCountry.welfare.population -= (int)playerCountry.security.nuclear_casualties;
        playerCountry.welfare.death_rate += 0.05;

        // Infrastructure devastated
        playerCountry.infra.road_connectivity *= 0.3;
        playerCountry.infra.port_capacity *= 0.3;
        playerCountry.infra.grid_resilience *= 0.2;
        playerCountry.welfare.hospitals = (int)(playerCountry.welfare.hospitals * 0.3);

        // Long-term contamination
        playerCountry.security.nuclear_contamination = 0.8;
        playerCountry.welfare.food_radiation_prob = 0.9;

        // Economy destroyed
        playerCountry.economy.gdp *= 0.4; // 60% GDP wiped
        playerCountry.economy.international_reserves *= 0.2;

        // International response
        playerCountry.security.diplomatic_prestige += 0.2; // Sympathy
        playerCountry.economy.international_sanctions_prob -= 0.3; // Sanctions lifted out of solidarity

        // Near game over
        playerCountry.politics.popularity = 0.5; // Rally effect, but everything is destroyed

        std::cout << "[!!!!!] NUCLEAR STRIKE: A nuclear weapon has detonated on national territory!" << std::endl;
        std::cout << "        Immediate casualties: " << (int)playerCountry.security.nuclear_casualties
                  << ". Infrastructure devastated. GDP collapsed to 40%." << std::endl;
        std::cout << "        Long-term contamination: radiation will persist for many turns." << std::endl;
    }

    // --- MASS MIGRATION CRISIS ---
    if (playerCountry.security.migration_crisis_active) {
        playerCountry.security.migration_wave_duration--;

        // Refugee inflow each turn
        double inflow = playerCountry.security.refugee_inflow * (1.0 - playerCountry.security.migration_policy_restrictiveness);
        playerCountry.security.refugee_population += (int)inflow;
        playerCountry.welfare.population += (int)inflow;

        // Welfare strain: hospitals, housing, services
        playerCountry.welfare.health_coverage -= 0.01;
        if (playerCountry.welfare.health_coverage < 0.1) playerCountry.welfare.health_coverage = 0.1;
        playerCountry.welfare.unemployment_rate += 0.005;

        // Social tension
        playerCountry.welfare.interreligious_tension += 0.02;
        if (playerCountry.welfare.interreligious_tension > 1.0)
            playerCountry.welfare.interreligious_tension = 1.0;
        playerCountry.politics.polarization_index += 0.02;

        // But also economic potential if economy is strong
        if (playerCountry.welfare.unemployment_rate < 0.08) {
            playerCountry.economy.gdp += inflow * 5000.0; // Each refugee contributes some GDP
        }

        // Popularity depends on public opinion
        if (playerCountry.politics.polarization_index > 0.6) {
            playerCountry.politics.popularity -= 0.02; // Xenophobic backlash
        } else {
            playerCountry.politics.popularity -= 0.005; // Mild concern
        }

        std::cout << "[!!] MIGRATION CRISIS (Turn " << (playerCountry.security.migration_wave_duration + 1)
                  << " remaining): Refugees hosted: " << playerCountry.security.refugee_population
                  << ". Social tension rising." << std::endl;

        if (playerCountry.security.migration_wave_duration <= 0) {
            playerCountry.security.migration_crisis_active = false;
            std::cout << "[INFO] MIGRATION WAVE SUBSIDES: " << playerCountry.security.refugee_population
                      << " refugees now integrated into society." << std::endl;
        }
    } else {
        if (dist(rng) < playerCountry.security.mass_migration_prob) {
            playerCountry.security.migration_crisis_active = true;
            std::uniform_int_distribution<int> dur_dist(2, 5);
            playerCountry.security.migration_wave_duration = dur_dist(rng);
            std::uniform_real_distribution<double> flow_dist(10000.0, 200000.0);
            playerCountry.security.refugee_inflow = flow_dist(rng);
            std::cout << "[!!] MASS MIGRATION CRISIS: Refugee wave incoming! ~"
                      << (int)playerCountry.security.refugee_inflow << " per turn for "
                      << playerCountry.security.migration_wave_duration << " turns." << std::endl;
        }
    }

    // --- DIPLOMATIC CRISIS ---
    if (playerCountry.security.diplomatic_crisis_active) {
        playerCountry.security.diplomatic_crisis_duration--;
        playerCountry.security.diplomatic_fallout += 0.05;
        if (playerCountry.security.diplomatic_fallout > 1.0)
            playerCountry.security.diplomatic_fallout = 1.0;

        // Trade partners reduce cooperation
        playerCountry.economy.gdp -= playerCountry.economy.gdp * 0.005 * playerCountry.security.diplomatic_fallout;

        // Tariff retaliation
        playerCountry.economy.average_tariffs += 0.01;

        // Tourism warning
        playerCountry.economy.travel_warning_level = std::max(playerCountry.economy.travel_warning_level, 2);

        // FDI drops
        playerCountry.economy.international_reserves -= 2000000.0;

        // Diplomatic prestige erodes
        playerCountry.security.diplomatic_prestige -= 0.03;
        if (playerCountry.security.diplomatic_prestige < 0.0)
            playerCountry.security.diplomatic_prestige = 0.0;

        std::cout << "[!!] DIPLOMATIC CRISIS (Turn " << (playerCountry.security.diplomatic_crisis_duration + 1)
                  << " remaining): International fallout " << (int)(playerCountry.security.diplomatic_fallout * 100)
                  << "%." << std::endl;

        if (playerCountry.security.diplomatic_crisis_duration <= 0) {
            playerCountry.security.diplomatic_crisis_active = false;
            playerCountry.security.diplomatic_fallout *= 0.5; // Partial recovery
            std::cout << "[INFO] DIPLOMATIC CRISIS RESOLVED: Relations normalizing." << std::endl;
        }
    } else {
        // Trigger: low prestige, active sanctions, or spy leak
        bool diplo_trigger = false;
        if (playerCountry.security.diplomatic_prestige < 0.2) diplo_trigger = true;
        if (playerCountry.economy.international_sanctions_prob > 0.5) diplo_trigger = true;
        if (playerCountry.security.document_leak_prob > 0.3 && dist(rng) < playerCountry.security.document_leak_prob * 0.2) {
            diplo_trigger = true;
            std::cout << "[!!] SPY LEAK: Classified documents exposed — triggering diplomatic fallout!" << std::endl;
        }
        if (diplo_trigger && dist(rng) < 0.1) { // 10% chance when conditions met
            playerCountry.security.diplomatic_crisis_active = true;
            std::uniform_int_distribution<int> dur_dist(2, 4);
            playerCountry.security.diplomatic_crisis_duration = dur_dist(rng);
            playerCountry.security.diplomatic_fallout = 0.0;
            std::cout << "[!!] DIPLOMATIC CRISIS: International incident! Duration: "
                      << playerCountry.security.diplomatic_crisis_duration << " turns." << std::endl;
        }
    }

    // Soft power: education + heritage + diplomacy
    playerCountry.security.soft_power_index = playerCountry.welfare.educational_quality * 0.3
                                            + playerCountry.economy.heritage_preservation * 0.2
                                            + playerCountry.security.diplomatic_prestige * 0.3
                                            + playerCountry.security.multilateral_leadership * 0.2;
    if (playerCountry.security.soft_power_index > 1.0) playerCountry.security.soft_power_index = 1.0;

    // --- LOBBY POWER DYNAMICS ---
    // Extractive sector power correlates with mining concessions and commodity boom
    playerCountry.politics.extractive_sector_power = 0.3 + playerCountry.economy.mining_concessions * 0.03
                                                   + (playerCountry.economy.commodity_prices > 1.2 ? 0.1 : 0.0);
    if (playerCountry.politics.extractive_sector_power > 1.0) playerCountry.politics.extractive_sector_power = 1.0;
    // Media ownership concentration: high financial power + low press freedom
    playerCountry.politics.media_ownership_concentration = playerCountry.politics.financial_power * 0.4
                                                        + (1.0 - playerCountry.security.press_freedom) * 0.3;
    if (playerCountry.politics.media_ownership_concentration > 1.0) playerCountry.politics.media_ownership_concentration = 1.0;
    // Total rent-seeking cost: all lobbies extract rents proportional to their power
    playerCountry.politics.total_lobby_rent_gdp = (playerCountry.politics.industrial_power
                                                 + playerCountry.politics.agricultural_power
                                                 + playerCountry.politics.financial_power
                                                 + playerCountry.politics.tech_power
                                                 + playerCountry.politics.extractive_sector_power) * 0.006;
    // Rent-seeking drags GDP growth
    playerCountry.economy.growth_rate -= playerCountry.politics.total_lobby_rent_gdp * 0.1;

    // --- PROSECUTION & IMPUNITY DYNAMICS ---
    // Impunity derives from prosecutor capacity, specialized units, and witness protection
    double prosecutor_capacity = (double)playerCountry.politics.prosecutors / (double)(playerCountry.politics.case_files + 1);
    playerCountry.politics.impunity = 1.0 - (prosecutor_capacity * 0.4
                                           + playerCountry.politics.specialized_units * 0.03
                                           + playerCountry.politics.witness_protection_capacity * 0.15
                                           + playerCountry.politics.anticorruption_enforcement * 0.1);
    if (playerCountry.politics.impunity < 0.05) playerCountry.politics.impunity = 0.05;
    if (playerCountry.politics.impunity > 0.98) playerCountry.politics.impunity = 0.98;
    // Sector-specific impunity
    playerCountry.politics.impunity_homicide = playerCountry.politics.impunity * 1.2; // Homicide harder to prosecute
    if (playerCountry.politics.impunity_homicide > 0.98) playerCountry.politics.impunity_homicide = 0.98;
    playerCountry.politics.impunity_corruption = playerCountry.politics.impunity * 1.5
                                               - playerCountry.politics.anticorruption_enforcement * 0.3;
    if (playerCountry.politics.impunity_corruption > 0.98) playerCountry.politics.impunity_corruption = 0.98;
    if (playerCountry.politics.impunity_corruption < 0.1) playerCountry.politics.impunity_corruption = 0.1;
    playerCountry.politics.impunity_organized_crime = playerCountry.politics.impunity * 1.3
                                                    - playerCountry.politics.witness_protection_capacity * 0.2;
    if (playerCountry.politics.impunity_organized_crime > 0.98) playerCountry.politics.impunity_organized_crime = 0.98;
    if (playerCountry.politics.impunity_organized_crime < 0.1) playerCountry.politics.impunity_organized_crime = 0.1;
    // High impunity erodes trust
    playerCountry.politics.trust_in_justice -= (playerCountry.politics.impunity - 0.5) * 0.005;
    if (playerCountry.politics.trust_in_justice < 0.1) playerCountry.politics.trust_in_justice = 0.1;
    if (playerCountry.politics.trust_in_justice > 1.0) playerCountry.politics.trust_in_justice = 1.0;

    // --- IMPUNITY CRISIS (High-profile case collapse) ---
    if (playerCountry.politics.impunity_crisis_active) {
        playerCountry.politics.impunity_crisis_duration--;

        // Public outrage: protests
        playerCountry.politics.protest_intensity += 0.04;
        playerCountry.politics.popularity -= 0.03;

        // Trust in justice collapses
        playerCountry.politics.trust_in_justice -= 0.03;
        if (playerCountry.politics.trust_in_justice < 0.05)
            playerCountry.politics.trust_in_justice = 0.05;

        // Vigilante justice risk
        playerCountry.security.homicide_rate += 0.5; // People taking law into own hands

        // International shame
        playerCountry.welfare.un_score -= 0.02;

        std::cout << "[!!] IMPUNITY CRISIS (Turn " << (playerCountry.politics.impunity_crisis_duration + 1)
                  << " remaining): High-profile case collapsed. Public fury." << std::endl;

        if (playerCountry.politics.impunity_crisis_duration <= 0) {
            playerCountry.politics.impunity_crisis_active = false;
            std::cout << "[INFO] IMPUNITY CRISIS SUBSIDES: Prosecution reforms promised." << std::endl;
        }
    } else {
        if (playerCountry.politics.impunity > 0.8 && dist(rng) < 0.1) {
            playerCountry.politics.impunity_crisis_active = true;
            std::uniform_int_distribution<int> dur_dist(2, 3);
            playerCountry.politics.impunity_crisis_duration = dur_dist(rng);
            std::cout << "[!!] IMPUNITY CRISIS: Major criminal case dismissed! Impunity rate: "
                      << (int)(playerCountry.politics.impunity * 100) << "%. Public outrage erupts." << std::endl;
        }
    }

    // --- JUDICIAL SYSTEM DYNAMICS ---
    // Court packing risk rises with democratic backsliding and low independence
    playerCountry.politics.court_packing_risk = playerCountry.politics.democratic_backsliding_index * 0.3
                                              + playerCountry.politics.electoral_manipulation_capacity * 0.2;
    if (playerCountry.politics.court_packing_risk > 0.8) playerCountry.politics.court_packing_risk = 0.8;
    // Budget adequacy drives sentencing speed
    playerCountry.politics.sentencing_speed = playerCountry.politics.judicial_budget_adequacy * 0.6
                                            + (1.0 - (double)playerCountry.politics.case_backlog / 200000.0) * 0.4;
    if (playerCountry.politics.sentencing_speed < 0.1) playerCountry.politics.sentencing_speed = 0.1;
    if (playerCountry.politics.sentencing_speed > 1.0) playerCountry.politics.sentencing_speed = 1.0;
    // Case backlog grows with low speed, shrinks with prosecutors and budget
    playerCountry.politics.case_backlog += (int)(playerCountry.politics.case_files * 0.3);
    playerCountry.politics.case_backlog -= (int)(playerCountry.politics.prosecutors * playerCountry.politics.sentencing_speed * 2.0);
    if (playerCountry.politics.case_backlog < 0) playerCountry.politics.case_backlog = 0;
    // Pretrial detention worsens with backlog
    playerCountry.politics.pretrial_detention_rate = 0.15 + (double)playerCountry.politics.case_backlog / 300000.0;
    if (playerCountry.politics.pretrial_detention_rate > 0.8) playerCountry.politics.pretrial_detention_rate = 0.8;
    // Average case duration in years
    playerCountry.politics.average_case_duration_years = 1.0 + (1.0 - playerCountry.politics.sentencing_speed) * 6.0;
    // State compliance: independent courts that rule against state need compliance
    playerCountry.politics.state_compliance_rate = playerCountry.politics.judicial_independence * 0.7
                                                 + playerCountry.security.press_freedom * 0.2;
    if (playerCountry.politics.state_compliance_rate > 1.0) playerCountry.politics.state_compliance_rate = 1.0;
    // Trust decomposition: poor trust shaped by pretrial detention and impunity
    playerCountry.politics.poor_trust_in_justice = playerCountry.politics.trust_in_justice * 0.6
                                                 - playerCountry.politics.pretrial_detention_rate * 0.3
                                                 - playerCountry.politics.impunity * 0.2;
    if (playerCountry.politics.poor_trust_in_justice < 0.0) playerCountry.politics.poor_trust_in_justice = 0.0;
    playerCountry.politics.elite_trust_in_justice = playerCountry.politics.trust_in_justice * 0.8
                                                  + playerCountry.politics.state_compliance_rate * 0.2;
    if (playerCountry.politics.elite_trust_in_justice > 1.0) playerCountry.politics.elite_trust_in_justice = 1.0;

    // --- JUDICIARY CRISIS (Judiciary vs Executive) ---
    if (playerCountry.politics.judiciary_crisis_active) {
        playerCountry.politics.judiciary_crisis_duration--;

        // Constitutional crisis: courts block executive
        playerCountry.politics.pending_bills += 2; // Reform bills blocked
        playerCountry.politics.state_compliance_rate -= 0.05;
        if (playerCountry.politics.state_compliance_rate < 0.1)
            playerCountry.politics.state_compliance_rate = 0.1;

        // Democratic backsliding pressure (temptation to pack courts)
        playerCountry.politics.court_packing_risk += 0.03;
        if (playerCountry.politics.court_packing_risk > 0.8)
            playerCountry.politics.court_packing_risk = 0.8;
        playerCountry.politics.democratic_backsliding_index += 0.01;

        // International scrutiny
        playerCountry.economy.international_sanctions_prob += 0.01;
        playerCountry.welfare.un_score -= 0.01;

        // Trust in justice shifts
        playerCountry.politics.trust_in_justice -= 0.02;
        if (playerCountry.politics.trust_in_justice < 0.1)
            playerCountry.politics.trust_in_justice = 0.1;

        playerCountry.politics.popularity -= 0.015;

        std::cout << "[!!] JUDICIARY CRISIS (Turn " << (playerCountry.politics.judiciary_crisis_duration + 1)
                  << " remaining): Courts blocking executive agenda. Constitutional showdown." << std::endl;

        if (playerCountry.politics.judiciary_crisis_duration <= 0) {
            playerCountry.politics.judiciary_crisis_active = false;
            // Court wins: judicial independence asserted
            playerCountry.politics.judicial_independence += 0.05;
            if (playerCountry.politics.judicial_independence > 1.0)
                playerCountry.politics.judicial_independence = 1.0;
            std::cout << "[INFO] JUDICIARY CRISIS RESOLVED: Courts assert independence. Executive backs down."
                      << std::endl;
        }
    } else {
        if (playerCountry.politics.ruling_against_state_prob > 0.6
            && playerCountry.politics.judicial_independence > 0.7
            && playerCountry.politics.popularity < 0.4
            && dist(rng) < 0.12) {
            playerCountry.politics.judiciary_crisis_active = true;
            std::uniform_int_distribution<int> dur_dist(2, 3);
            playerCountry.politics.judiciary_crisis_duration = dur_dist(rng);
            std::cout << "[!!] JUDICIARY CRISIS: Supreme Court strikes down key executive policy! "
                      << "Constitutional confrontation." << std::endl;
        }
    }

    // --- REGIONAL DYNAMICS ---
    // Fiscal transfer inadequacy + budget disparity erode regional loyalty
    playerCountry.politics.fiscal_transfer_adequacy = 0.5 + (1.0 - playerCountry.politics.federal_budget_disparity) * 0.3
                                                    + playerCountry.politics.fiscal_autonomy * 0.2;
    if (playerCountry.politics.fiscal_transfer_adequacy > 1.0) playerCountry.politics.fiscal_transfer_adequacy = 1.0;
    playerCountry.politics.regional_loyalty += (playerCountry.politics.fiscal_transfer_adequacy - 0.5) * 0.01;
    if (playerCountry.politics.regional_loyalty < 0.0) playerCountry.politics.regional_loyalty = 0.0;
    if (playerCountry.politics.regional_loyalty > 1.0) playerCountry.politics.regional_loyalty = 1.0;
    // Lagging regions breed separatism
    playerCountry.politics.lagging_region_share = playerCountry.politics.regional_gdp_gini * 0.7;
    if (playerCountry.politics.lagging_region_share > 1.0) playerCountry.politics.lagging_region_share = 1.0;
    // Separatism dynamics: grows with low loyalty, high disparity, high autonomy demand
    playerCountry.politics.separatist_support_pop += (1.0 - playerCountry.politics.regional_loyalty) * 0.003
                                                   + playerCountry.politics.lagging_region_share * 0.002;
    playerCountry.politics.separatist_support_pop -= playerCountry.politics.fiscal_transfer_adequacy * 0.002;
    if (playerCountry.politics.separatist_support_pop < 0.0) playerCountry.politics.separatist_support_pop = 0.0;
    if (playerCountry.politics.separatist_support_pop > 0.5) playerCountry.politics.separatist_support_pop = 0.5;
    // Active movements emerge above 15% support
    playerCountry.politics.active_separatist_movements = (int)(playerCountry.politics.separatist_support_pop / 0.15);
    // Separatism probability derives from structural factors
    playerCountry.politics.separatism_prob = playerCountry.politics.separatist_support_pop * 0.1
                                           + playerCountry.politics.active_separatist_movements * 0.02;
    // Regions in conflict: loyalty below 0.4 = confrontation
    playerCountry.politics.regions_in_conflict = (playerCountry.politics.regional_loyalty < 0.4)
                                               ? playerCountry.politics.active_separatist_movements + 1 : 0;

    // --- SEPARATIST / GOVERNORS CRISIS ---
    if (playerCountry.politics.separatist_crisis_active) {
        playerCountry.politics.separatist_crisis_duration--;

        // Region withholds taxes
        playerCountry.economy.tax_collection -= playerCountry.economy.tax_collection * 0.05;

        // Federal budget dispute
        playerCountry.politics.federal_budget_disparity += 0.03;
        if (playerCountry.politics.federal_budget_disparity > 0.8)
            playerCountry.politics.federal_budget_disparity = 0.8;

        // Separatist support grows
        playerCountry.politics.separatist_support_pop += 0.02;
        if (playerCountry.politics.separatist_support_pop > 0.5)
            playerCountry.politics.separatist_support_pop = 0.5;

        // Polarization and diplomatic embarrassment
        playerCountry.politics.polarization_index += 0.02;
        playerCountry.security.diplomatic_prestige -= 0.02;

        // Independence referendum pressure
        if (playerCountry.politics.separatist_support_pop > 0.3)
            playerCountry.politics.independence_referendum_pending = true;

        playerCountry.politics.popularity -= 0.02;

        std::cout << "[!!!] SEPARATIST CRISIS (Turn " << (playerCountry.politics.separatist_crisis_duration + 1)
                  << " remaining): Region demands autonomy. Separatist support: "
                  << (int)(playerCountry.politics.separatist_support_pop * 100) << "%" << std::endl;

        if (playerCountry.politics.separatist_crisis_duration <= 0) {
            playerCountry.politics.separatist_crisis_active = false;
            // Concession: increase autonomy
            playerCountry.politics.provincial_autonomy += 0.1;
            if (playerCountry.politics.provincial_autonomy > 1.0)
                playerCountry.politics.provincial_autonomy = 1.0;
            std::cout << "[INFO] SEPARATIST CRISIS RESOLVED: Autonomy deal reached. Provincial autonomy increased."
                      << std::endl;
        }
    } else {
        if (playerCountry.politics.regional_loyalty < 0.3
            && dist(rng) < playerCountry.politics.separatism_prob) {
            playerCountry.politics.separatist_crisis_active = true;
            std::uniform_int_distribution<int> dur_dist(3, 5);
            playerCountry.politics.separatist_crisis_duration = dur_dist(rng);
            std::cout << "[!!!] SEPARATIST CRISIS: Region declares autonomy push! Loyalty: "
                      << (int)(playerCountry.politics.regional_loyalty * 100) << "%. Duration: "
                      << playerCountry.politics.separatist_crisis_duration << " turns." << std::endl;
        }
    }

    // --- LEGISLATIVE PIPELINE ---
    // Bills passed depends on support, law blockade, and pending queue
    double pass_rate = playerCountry.politics.congressional_support * (1.0 - playerCountry.politics.law_blockade_prob);
    playerCountry.politics.bills_passed_this_turn = (int)(playerCountry.politics.pending_bills * pass_rate);
    if (playerCountry.politics.bills_passed_this_turn < 0) playerCountry.politics.bills_passed_this_turn = 0;
    playerCountry.politics.pending_bills -= playerCountry.politics.bills_passed_this_turn;
    playerCountry.politics.pending_bills += 2; // New bills introduced each turn
    if (playerCountry.politics.pending_bills < 0) playerCountry.politics.pending_bills = 0;
    // Legislative efficiency: smoothed ratio
    if (playerCountry.politics.pending_bills + playerCountry.politics.bills_passed_this_turn > 0)
        playerCountry.politics.legislative_efficiency = (double)playerCountry.politics.bills_passed_this_turn
            / (playerCountry.politics.pending_bills + playerCountry.politics.bills_passed_this_turn);
    // Constitutional reforms: need supermajority (>0.66 support) and high judicial independence
    if (playerCountry.politics.constitutional_reforms_pending > 0
        && playerCountry.politics.congressional_support > 0.66
        && playerCountry.politics.judicial_independence > 0.6) {
        playerCountry.politics.constitutional_reforms_pending--;
        std::cout << "[+] CONSTITUTIONAL REFORM: Reform passed with supermajority." << std::endl;
    }

    // --- LOBBYING & REGULATORY CAPTURE ---
    // Regulatory capture grows with revolving door and high lobby power; shrinks with anticorruption
    double total_lobby_power = (playerCountry.politics.industrial_power + playerCountry.politics.financial_power
                              + playerCountry.politics.agricultural_power + playerCountry.politics.tech_power) / 4.0;
    playerCountry.politics.regulatory_capture_index += playerCountry.politics.revolving_door_intensity * 0.005
                                                    + total_lobby_power * 0.003;
    playerCountry.politics.regulatory_capture_index -= playerCountry.politics.anticorruption_enforcement * 0.004;
    if (playerCountry.politics.regulatory_capture_index < 0.0) playerCountry.politics.regulatory_capture_index = 0.0;
    if (playerCountry.politics.regulatory_capture_index > 1.0) playerCountry.politics.regulatory_capture_index = 1.0;
    // High capture reduces effective tax collection (regulatory loopholes)
    if (playerCountry.politics.regulatory_capture_index > 0.5)
        playerCountry.economy.tax_collection *= (1.0 - (playerCountry.politics.regulatory_capture_index - 0.5) * 0.06);

    // --- REGULATORY CAPTURE / LOBBIES CRISIS ---
    if (playerCountry.politics.regulatory_capture_crisis) {
        playerCountry.politics.regulatory_capture_crisis_duration--;

        // Lobbies block critical reform (health, environment, labor)
        playerCountry.politics.pending_bills += 2;
        playerCountry.economy.gdp -= playerCountry.economy.gdp * 0.003; // Rent-seeking drag

        // Public outrage at plutocracy
        playerCountry.politics.protest_intensity += 0.02;
        playerCountry.politics.popularity -= 0.02;
        playerCountry.politics.polarization_index += 0.02;

        // Tax loopholes widen
        playerCountry.economy.tax_collection *= 0.97;

        std::cout << "[!!] LOBBIES CRISIS (Turn " << (playerCountry.politics.regulatory_capture_crisis_duration + 1)
                  << " remaining): Industry lobbies blocking reform. Rent-seeking: "
                  << (int)(playerCountry.politics.total_lobby_rent_gdp * 100) << "% GDP." << std::endl;

        if (playerCountry.politics.regulatory_capture_crisis_duration <= 0) {
            playerCountry.politics.regulatory_capture_crisis = false;
            std::cout << "[INFO] LOBBIES CRISIS RESOLVED: Anti-corruption push breaks deadlock." << std::endl;
        }
    } else {
        if (total_lobby_power > 0.7
            && playerCountry.politics.regulatory_capture_index > 0.6
            && dist(rng) < 0.08) {
            playerCountry.politics.regulatory_capture_crisis = true;
            std::uniform_int_distribution<int> dur_dist(2, 3);
            playerCountry.politics.regulatory_capture_crisis_duration = dur_dist(rng);
            std::cout << "[!!] LOBBIES CRISIS: Powerful lobbies capture regulatory process! "
                      << "Critical reforms blocked." << std::endl;
        }
    }

    // --- PARTY SYSTEM DYNAMICS ---
    // Effective parties from fragmentation index: frag 0.2=~2 parties, 0.8=~6+
    playerCountry.politics.effective_parties = 2 + (int)(playerCountry.politics.party_fragmentation * 5.0);
    playerCountry.politics.two_party_dominance = (playerCountry.politics.effective_parties <= 2);
    // Coalition formation cost: more parties = harder to build majority
    playerCountry.politics.coalition_formation_cost = playerCountry.politics.party_fragmentation * 0.5
                                                   + (1.0 - playerCountry.politics.coalition_cohesion) * 0.3;
    if (playerCountry.politics.coalition_formation_cost > 1.0) playerCountry.politics.coalition_formation_cost = 1.0;
    // High coalition cost drains political capital → popularity penalty
    if (playerCountry.politics.coalition_formation_cost > 0.6)
        playerCountry.politics.popularity -= 0.005;

    // --- LEGISLATIVE FRICTION ---
    // Law blockade probability derives from veto players, filibuster, and low support
    playerCountry.politics.filibuster_usage = playerCountry.politics.veto_player_strength * 0.3
                                            + playerCountry.politics.party_fragmentation * 0.2;
    if (playerCountry.politics.filibuster_usage > 0.8) playerCountry.politics.filibuster_usage = 0.8;
    playerCountry.politics.law_blockade_prob = (1.0 - playerCountry.politics.congressional_support) * 0.3
                                            + playerCountry.politics.filibuster_usage * 0.3
                                            + playerCountry.politics.presidential_veto_prob;
    if (playerCountry.politics.law_blockade_prob > 0.9) playerCountry.politics.law_blockade_prob = 0.9;

    // --- LEGISLATIVE DYNAMICS ---
    // Coalition cohesion erodes with polarization and low popularity
    playerCountry.politics.coalition_cohesion -= playerCountry.politics.polarization_index * 0.005;
    if (playerCountry.politics.popularity < 0.35)
        playerCountry.politics.coalition_cohesion -= 0.01; // Rats leaving sinking ship
    if (playerCountry.politics.coalition_cohesion < 0.2) playerCountry.politics.coalition_cohesion = 0.2;
    if (playerCountry.politics.coalition_cohesion > 1.0) playerCountry.politics.coalition_cohesion = 1.0;
    // Opposition seats derived from support level
    playerCountry.politics.opposition_seats = (int)((1.0 - playerCountry.politics.congressional_support) * 100);
    // Veto player strength: fragmented opposition with strong institutions can block
    playerCountry.politics.veto_player_strength = (1.0 - playerCountry.politics.congressional_support) * 0.5
                                                + playerCountry.politics.judicial_independence * 0.3;
    if (playerCountry.politics.veto_player_strength > 1.0) playerCountry.politics.veto_player_strength = 1.0;

    // --- LEGISLATIVE CRISIS (Congressional Blockade / Government Shutdown) ---
    if (playerCountry.politics.legislative_crisis_active) {
        playerCountry.politics.legislative_crisis_duration--;

        // Government shutdown: public services degrade
        playerCountry.welfare.health_coverage -= 0.01;
        if (playerCountry.welfare.health_coverage < 0.1)
            playerCountry.welfare.health_coverage = 0.1;

        // Budget freeze: infrastructure maintenance drops
        playerCountry.infra.maintenance_level -= 0.02;
        if (playerCountry.infra.maintenance_level < 0.2)
            playerCountry.infra.maintenance_level = 0.2;

        // No new legislation possible
        playerCountry.politics.pending_bills += 3; // Bills pile up

        // GDP slowdown from governance paralysis
        playerCountry.economy.gdp -= playerCountry.economy.gdp * 0.005;

        // Popularity hit: government looks dysfunctional
        playerCountry.politics.popularity -= 0.025;

        // Credit rating agencies worried
        playerCountry.economy.debt_to_gdp_ratio += 0.003;

        std::cout << "[!!] LEGISLATIVE CRISIS (Turn " << (playerCountry.politics.legislative_crisis_duration + 1)
                  << " remaining): Government shutdown. " << playerCountry.politics.pending_bills
                  << " bills stalled." << std::endl;

        if (playerCountry.politics.legislative_crisis_duration <= 0) {
            playerCountry.politics.legislative_crisis_active = false;
            std::cout << "[INFO] LEGISLATIVE CRISIS RESOLVED: Compromise reached. Government reopens." << std::endl;
        }
    } else {
        if (playerCountry.politics.congressional_support < 0.3
            && dist(rng) < playerCountry.politics.law_blockade_prob) {
            playerCountry.politics.legislative_crisis_active = true;
            std::uniform_int_distribution<int> dur_dist(2, 4);
            playerCountry.politics.legislative_crisis_duration = dur_dist(rng);
            std::cout << "[!!] LEGISLATIVE CRISIS: Congress blocks all executive agenda! "
                      << "Government shutdown begins. Support: "
                      << (int)(playerCountry.politics.congressional_support * 100) << "%" << std::endl;
        }
    }

    // --- POLARIZATION DYNAMICS ---
    // Derive composite polarization from sub-dimensions
    // Poverty and inequality drive economic polarization
    playerCountry.politics.economic_polarization = playerCountry.welfare.poverty_rate * 0.8
                                                 + (1.0 - playerCountry.welfare.minority_protection) * 0.2;
    if (playerCountry.politics.economic_polarization > 1.0) playerCountry.politics.economic_polarization = 1.0;
    // Low media pluralism + high media control → echo chambers
    playerCountry.politics.media_echo_chamber = playerCountry.security.media_control * 0.4
                                              + (1.0 - playerCountry.security.press_freedom) * 0.3
                                              + playerCountry.security.fake_news_success_prob * 0.3;
    if (playerCountry.politics.media_echo_chamber > 1.0) playerCountry.politics.media_echo_chamber = 1.0;
    // Affective polarization grows with protests and echo chambers, shrinks with education
    playerCountry.politics.affective_polarization += playerCountry.politics.media_echo_chamber * 0.003
                                                   + playerCountry.politics.protest_intensity * 0.005;
    playerCountry.politics.affective_polarization -= playerCountry.welfare.educational_quality * 0.003;
    if (playerCountry.politics.affective_polarization < 0.0) playerCountry.politics.affective_polarization = 0.0;
    if (playerCountry.politics.affective_polarization > 1.0) playerCountry.politics.affective_polarization = 1.0;
    // Composite index
    playerCountry.politics.polarization_index = playerCountry.politics.affective_polarization * 0.40
                                              + playerCountry.politics.economic_polarization * 0.35
                                              + playerCountry.politics.media_echo_chamber * 0.25;

    // --- PROTEST DYNAMICS ---
    {
        int total_protests = playerCountry.politics.marches + playerCountry.politics.blockades
                           + playerCountry.politics.mobilizations;
        if (total_protests > 0) {
            // Intensity scales with blockades (disruptive) and sustained duration
            playerCountry.politics.protest_intensity = (playerCountry.politics.blockades * 0.15
                                                      + playerCountry.politics.marches * 0.05
                                                      + playerCountry.politics.mobilizations * 0.08);
            if (playerCountry.politics.protest_intensity > 1.0) playerCountry.politics.protest_intensity = 1.0;
            playerCountry.politics.protest_duration_turns++;
            // Protests are pro-govt if popularity > 0.6 and polarization high
            playerCountry.politics.protest_pro_government = (playerCountry.politics.popularity > 0.6
                                                           && playerCountry.politics.polarization_index > 0.5);
            // Sustained high-intensity protests destabilize
            if (playerCountry.politics.protest_duration_turns > 3 && playerCountry.politics.protest_intensity > 0.5) {
                playerCountry.economy.growth_rate -= 0.003;
                playerCountry.economy.exchange_rate_stability -= 0.01;
                if (!playerCountry.politics.protest_pro_government)
                    playerCountry.politics.popularity -= 0.02;
            }
            // Natural decay of protests
            playerCountry.politics.marches = (int)(playerCountry.politics.marches * 0.7);
            playerCountry.politics.blockades = (int)(playerCountry.politics.blockades * 0.6);
            playerCountry.politics.mobilizations = (int)(playerCountry.politics.mobilizations * 0.7);
        } else {
            playerCountry.politics.protest_intensity = 0.0;
            playerCountry.politics.protest_duration_turns = 0;
        }
    }

    // --- CORRUPTION DYNAMICS ---
    // Derive administrative_corruption from petty + grand components
    playerCountry.politics.administrative_corruption = playerCountry.politics.petty_corruption * 0.6
                                                     + playerCountry.politics.grand_corruption * 0.4;
    // Anti-corruption enforcement reduces both over time
    if (playerCountry.politics.anticorruption_enforcement > 0.5) {
        playerCountry.politics.petty_corruption -= 0.003;
        playerCountry.politics.grand_corruption -= 0.002;
    }
    // Weak judiciary enables corruption growth
    if (playerCountry.politics.judicial_independence < 0.4) {
        playerCountry.politics.grand_corruption += 0.003;
        playerCountry.politics.petty_corruption += 0.002;
    }
    if (playerCountry.politics.petty_corruption < 0.0) playerCountry.politics.petty_corruption = 0.0;
    if (playerCountry.politics.petty_corruption > 1.0) playerCountry.politics.petty_corruption = 1.0;
    if (playerCountry.politics.grand_corruption < 0.0) playerCountry.politics.grand_corruption = 0.0;
    if (playerCountry.politics.grand_corruption > 1.0) playerCountry.politics.grand_corruption = 1.0;
    // CPI: inverse of corruption, scaled to TI 0–100
    playerCountry.politics.corruption_perception_index = (1.0 - playerCountry.politics.administrative_corruption) * 100.0;
    if (playerCountry.politics.corruption_perception_index < 0.0) playerCountry.politics.corruption_perception_index = 0.0;
    if (playerCountry.politics.corruption_perception_index > 100.0) playerCountry.politics.corruption_perception_index = 100.0;

    // --- DEMOCRATIC BACKSLIDING & EMERGENCY POWERS ---
    // Electoral manipulation and media control erode democracy over time
    playerCountry.politics.democratic_backsliding_index += playerCountry.politics.electoral_manipulation_capacity * 0.005
                                                        + playerCountry.security.media_control * 0.003;
    // Judicial independence and press freedom resist backsliding
    playerCountry.politics.democratic_backsliding_index -= playerCountry.politics.judicial_independence * 0.004
                                                        + playerCountry.security.press_freedom * 0.002;
    if (playerCountry.politics.democratic_backsliding_index < 0.0) playerCountry.politics.democratic_backsliding_index = 0.0;
    if (playerCountry.politics.democratic_backsliding_index > 1.0) playerCountry.politics.democratic_backsliding_index = 1.0;
    // High backsliding raises authoritarianism probability
    playerCountry.politics.authoritarianism_prob = playerCountry.politics.democratic_backsliding_index * 0.5;
    // Emergency powers: escalate backsliding faster and reduce judicial independence
    if (playerCountry.politics.state_of_emergency_active) {
        playerCountry.politics.emergency_turns_elapsed++;
        playerCountry.politics.democratic_backsliding_index += 0.015;
        playerCountry.politics.judicial_independence -= 0.01;
        if (playerCountry.politics.judicial_independence < 0.1) playerCountry.politics.judicial_independence = 0.1;
        // Prolonged emergency draws international condemnation
        if (playerCountry.politics.emergency_turns_elapsed > 3) {
            playerCountry.welfare.un_score -= 0.03;
            playerCountry.economy.international_sanctions_prob += 0.01;
            std::cout << "[!] PROLONGED EMERGENCY: International concern over indefinite emergency decree." << std::endl;
        }
    }

    // --- REVOLUTION / CIVIL WAR ---
    if (playerCountry.politics.civil_war_active) {
        playerCountry.politics.civil_war_duration++;

        // Massive GDP collapse
        playerCountry.economy.gdp -= playerCountry.economy.gdp * 0.08;

        // Casualties
        playerCountry.welfare.death_rate += 0.01;

        // Infrastructure destroyed
        playerCountry.infra.road_connectivity -= 0.03;
        if (playerCountry.infra.road_connectivity < 0.05) playerCountry.infra.road_connectivity = 0.05;

        // Refugees flee
        playerCountry.security.emigration_push_index = 0.95;
        playerCountry.welfare.population -= (int)(playerCountry.welfare.population * 0.005); // Brain drain

        // International intervention check
        if (playerCountry.politics.civil_war_duration > 4 && playerCountry.security.alliance_protection > 0.5) {
            if (dist(rng) < 0.25) {
                playerCountry.politics.civil_war_active = false;
                std::cout << "[!!!] INTERNATIONAL INTERVENTION: Allied forces restore order after "
                          << playerCountry.politics.civil_war_duration << " turns of civil war." << std::endl;
                playerCountry.security.diplomatic_prestige -= 0.3; // Humiliation of intervention
                playerCountry.politics.popularity = 0.3; // Reset
            }
        }

        // Prolonged civil war → GAME OVER
        if (playerCountry.politics.civil_war_active && playerCountry.politics.civil_war_duration > 10) {
            std::cout << "[!!!!!] STATE COLLAPSE: After " << playerCountry.politics.civil_war_duration
                      << " turns of civil war, the state has disintegrated." << std::endl;
            std::cout << "GAME OVER." << std::endl;
            exit(0);
        }

        if (playerCountry.politics.civil_war_active) {
            std::cout << "[!!!!] CIVIL WAR (Turn " << playerCountry.politics.civil_war_duration
                      << "): Nation tearing itself apart. GDP: $" << (long long)playerCountry.economy.gdp << std::endl;
        }
    // --- CROSS-SYSTEM FEEDBACK: POLITICAL → SECURITY ---
    {
        // Low popularity + high polarization → protest escalation and coup risk
        if (playerCountry.politics.popularity < 0.3 && playerCountry.politics.polarization_index > 0.6) {
            playerCountry.politics.protest_intensity += 0.02;
            if (playerCountry.politics.protest_intensity > 1.0) playerCountry.politics.protest_intensity = 1.0;
            playerCountry.politics.coup_d_etat_prob += 0.005;
        }

        // Democratic backsliding erodes institutions
        if (playerCountry.politics.democratic_backsliding_index > 0.3) {
            double backslide = playerCountry.politics.democratic_backsliding_index;
            playerCountry.security.press_freedom -= backslide * 0.02;
            if (playerCountry.security.press_freedom < 0.05) playerCountry.security.press_freedom = 0.05;
            playerCountry.politics.judicial_independence -= backslide * 0.01;
            if (playerCountry.politics.judicial_independence < 0.1) playerCountry.politics.judicial_independence = 0.1;
            // International community notices
            playerCountry.economy.international_sanctions_prob += backslide * 0.01;
        }

        // State of emergency: short-term stability but long-term erosion
        if (playerCountry.politics.state_of_emergency_active) {
            playerCountry.politics.emergency_turns_elapsed++;
            // Suppresses protests temporarily
            playerCountry.politics.protest_intensity *= 0.9;
            // But erodes civil liberties
            playerCountry.welfare.freedom_of_expression -= 0.02;
            if (playerCountry.welfare.freedom_of_expression < 0.1) playerCountry.welfare.freedom_of_expression = 0.1;
            playerCountry.politics.democratic_backsliding_index += 0.01;
            // International pressure builds
            if (playerCountry.politics.emergency_turns_elapsed > 3) {
                playerCountry.economy.international_sanctions_prob += 0.03;
                playerCountry.security.diplomatic_prestige -= 0.02;
            }
        }

        // High corruption → institutional decay
        if (playerCountry.politics.administrative_corruption > 0.5) {
            playerCountry.politics.trust_in_justice -= 0.01;
            if (playerCountry.politics.trust_in_justice < 0.1) playerCountry.politics.trust_in_justice = 0.1;
            playerCountry.politics.impunity += 0.005;
            if (playerCountry.politics.impunity > 0.95) playerCountry.politics.impunity = 0.95;
        }
    }

    } else {
        // Revolution risk assessment
        playerCountry.politics.revolution_risk = (playerCountry.politics.protest_intensity > 0.8
                                                 && playerCountry.politics.popularity < 0.2
                                                 && playerCountry.politics.polarization_index > 0.8);

        if (playerCountry.politics.revolution_risk) {
            // Revolution probability compounds each turn conditions persist
            playerCountry.politics.revolution_prob += 0.05;
            if (playerCountry.politics.revolution_prob > 0.5)
                playerCountry.politics.revolution_prob = 0.5;

            std::cout << "[!!!] REVOLUTION RISK: Conditions critical. Revolution probability: "
                      << (int)(playerCountry.politics.revolution_prob * 100) << "%" << std::endl;

            if (dist(rng) < playerCountry.politics.revolution_prob) {
                // Revolution triggered — either regime change or civil war
                if (playerCountry.security.troop_morale > 0.6 && playerCountry.politics.civilian_military_control < 0.4) {
                    // Military sides with revolutionaries → regime change
                    std::cout << "[!!!!!] REVOLUTION: The military has sided with the people. Regime overthrown!" << std::endl;
                    std::cout << "GAME OVER." << std::endl;
                    exit(0);
                } else {
                    // Military loyal → civil war
                    playerCountry.politics.civil_war_active = true;
                    playerCountry.politics.civil_war_duration = 0;
                    std::cout << "[!!!!!] CIVIL WAR ERUPTS: Revolution met with military force. "
                              << "The country descends into armed conflict!" << std::endl;
                }
            }
        } else {
            // Decay revolution probability when conditions ease
            playerCountry.politics.revolution_prob -= 0.02;
            if (playerCountry.politics.revolution_prob < 0.0)
                playerCountry.politics.revolution_prob = 0.0;
        }
    }

    // --- AUTONOMOUS ACTORS: CONGRESS ---
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // Congress proposes hostile bills when support is low
        if (playerCountry.politics.congressional_support < 0.4 && dist(rng) < 0.2) {
            int bill_type = (int)(dist(rng) * 3);
            if (bill_type == 0) {
                // Budget amendment: congress redirects spending
                playerCountry.economy.tax_collection *= 0.98;
                std::cout << "[CONGRESS] Hostile budget amendment passed. Revenue redirected to opposition projects." << std::endl;
            } else if (bill_type == 1) {
                // Censure motion
                playerCountry.politics.popularity -= 0.02;
                std::cout << "[CONGRESS] Censure motion against a minister. Government credibility damaged." << std::endl;
            } else {
                // Regulatory obstruction
                playerCountry.politics.pending_bills += 3;
                playerCountry.politics.legislative_efficiency -= 0.05;
                if (playerCountry.politics.legislative_efficiency < 0.1) playerCountry.politics.legislative_efficiency = 0.1;
                std::cout << "[CONGRESS] Opposition floods agenda with counter-proposals. Legislative gridlock!" << std::endl;
            }
        }

        // Impeachment motion: very low popularity + scandals active
        if (playerCountry.politics.popularity < 0.25
            && playerCountry.politics.active_scandals > 0
            && playerCountry.politics.congressional_support < 0.35
            && dist(rng) < 0.1) {
            double impeach_votes = (1.0 - playerCountry.politics.congressional_support) * 0.8
                                 + playerCountry.politics.active_scandals * 0.1;
            if (impeach_votes > 0.67) {
                std::cout << "[!!!] IMPEACHMENT: Congress votes to remove the president! "
                          << (int)(impeach_votes * 100) << "% in favor." << std::endl;
                std::cout << "GAME OVER." << std::endl;
                exit(0);
            } else {
                std::cout << "[!!] IMPEACHMENT MOTION: Congress debates removal. "
                          << (int)(impeach_votes * 100) << "% in favor (67% needed)." << std::endl;
                playerCountry.politics.popularity -= 0.05;
                playerCountry.politics.polarization_index += 0.05;
            }
        }
    }

    // --- AUTONOMOUS ACTORS: JUDICIARY ---
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // Courts strike down unconstitutional laws
        if (playerCountry.politics.judicial_independence > 0.6
            && playerCountry.politics.democratic_backsliding_index > 0.3
            && dist(rng) < playerCountry.politics.ruling_against_state_prob) {
            std::cout << "[JUDICIARY] Supreme Court strikes down executive order as unconstitutional!" << std::endl;
            playerCountry.politics.democratic_backsliding_index -= 0.03;
            if (playerCountry.politics.democratic_backsliding_index < 0.0) playerCountry.politics.democratic_backsliding_index = 0.0;
            playerCountry.politics.popularity -= 0.02; // Government embarrassed
            playerCountry.politics.trust_in_justice += 0.02;
        }

        // Judicial investigation of leader when scandals are active
        if (playerCountry.politics.judicial_independence > 0.5
            && playerCountry.politics.active_scandals > 0
            && dist(rng) < playerCountry.politics.anticorruption_enforcement * 0.15) {
            std::cout << "[JUDICIARY] Court orders investigation into leader's financial dealings!" << std::endl;
            playerCountry.politics.popularity -= 0.03;
            playerCountry.politics.media_exposure_intensity += 0.1;
            if (playerCountry.politics.media_exposure_intensity > 1.0) playerCountry.politics.media_exposure_intensity = 1.0;
            // Asset freeze
            if (dist(rng) < 0.3) {
                std::cout << "   [!] Assets of officials frozen pending investigation." << std::endl;
            }
        }

        // Habeas corpus challenges to emergency measures
        if (playerCountry.politics.state_of_emergency_active
            && playerCountry.politics.judicial_independence > 0.7
            && dist(rng) < 0.2) {
            std::cout << "[JUDICIARY] Court challenges emergency measures: some restrictions lifted." << std::endl;
            playerCountry.politics.protest_intensity += 0.05; // Protests resume
            playerCountry.welfare.freedom_of_expression += 0.02;
        }
    }

    // --- AUTONOMOUS ACTORS: OPPOSITION PARTIES ---
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // Opposition exploits scandals
        if (playerCountry.politics.active_scandals > 0 && dist(rng) < 0.3) {
            playerCountry.politics.popularity -= 0.01 * playerCountry.politics.active_scandals;
            playerCountry.politics.protest_intensity += 0.02;
            std::cout << "[OPPOSITION] Parties amplify scandal coverage. Street protests organized." << std::endl;
        }

        // Opposition coalition building when combined opposition > 50%
        if (playerCountry.politics.opposition_seats > 50 && dist(rng) < 0.15) {
            playerCountry.politics.coalition_cohesion -= 0.03;
            if (playerCountry.politics.coalition_cohesion < 0.2) playerCountry.politics.coalition_cohesion = 0.2;
            playerCountry.politics.congressional_support -= 0.02;
            if (playerCountry.politics.congressional_support < 0.1) playerCountry.politics.congressional_support = 0.1;
            std::cout << "[OPPOSITION] Coalition bloc formed. Government coalition weakening." << std::endl;
        }

        // Alternative policy proposals during crisis
        if (playerCountry.economy.in_recession && dist(rng) < 0.2) {
            playerCountry.politics.popularity -= 0.01;
            std::cout << "[OPPOSITION] Rival party presents alternative economic plan. Public comparing." << std::endl;
        }
    }

    // --- AUTONOMOUS ACTORS: MILITARY PRESSURE ---
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // Budget demands
        if (playerCountry.security.military_spending_gdp < 0.015 && dist(rng) < 0.2) {
            playerCountry.security.military_insubordination_prob += 0.01;
            playerCountry.security.troop_morale -= 0.02;
            std::cout << "[MILITARY] Armed forces demand higher budget allocation. Tension rising." << std::endl;
        }

        // Policy influence from politicized officer corps
        if (playerCountry.security.politicized_officer_corps > 0.4 && dist(rng) < 0.15) {
            playerCountry.politics.auth_dem_axis += 0.01;
            if (playerCountry.politics.auth_dem_axis > 1.0) playerCountry.politics.auth_dem_axis = 1.0;
            std::cout << "[MILITARY] Generals privately pressure cabinet on security policy." << std::endl;
        }
    }

    // --- AUTONOMOUS ACTORS: LOBBIES ---
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // Industrial lobby blocks environmental regulation
        if (playerCountry.politics.industrial_power > 0.6 && playerCountry.infra.carbon_tax_active && dist(rng) < 0.15) {
            playerCountry.infra.carbon_tax_rate *= 0.9;
            playerCountry.politics.popularity += 0.01; // Industry workers grateful
            std::cout << "[LOBBY] Industrial lobby weakens carbon tax through legal challenges." << std::endl;
        }

        // Financial lobby resists regulation
        if (playerCountry.politics.financial_power > 0.7 && dist(rng) < 0.1) {
            playerCountry.politics.regulatory_capture_index += 0.02;
            if (playerCountry.politics.regulatory_capture_index > 0.8) playerCountry.politics.regulatory_capture_index = 0.8;
            std::cout << "[LOBBY] Financial sector successfully lobbies against new regulations." << std::endl;
        }

        // Agricultural lobby demands subsidies
        if (playerCountry.politics.agricultural_power > 0.5 && dist(rng) < 0.1) {
            playerCountry.economy.gdp -= 5000000.0;
            playerCountry.welfare.poverty_rate -= 0.005;
            std::cout << "[LOBBY] Agricultural lobby secures $5M in subsidies." << std::endl;
        }
    }

    // --- AUTONOMOUS ACTORS: INTERNATIONAL ---
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // UN sends observers when democracy erodes
        if (playerCountry.politics.democratic_backsliding_index > 0.5 && dist(rng) < 0.15) {
            playerCountry.security.diplomatic_prestige -= 0.02;
            std::cout << "[INTERNATIONAL] UN Human Rights Council deploys observers." << std::endl;
        }

        // IMF conditionality during debt crisis
        if (playerCountry.economy.debt_to_gdp_ratio > 0.8
            && playerCountry.economy.in_recession && dist(rng) < 0.2) {
            // IMF demands austerity
            playerCountry.economy.tax_collection *= 1.05; // Forced tax hike
            playerCountry.welfare.minimum_wage *= 0.95; // Wage cuts
            playerCountry.politics.popularity -= 0.03;
            std::cout << "[INTERNATIONAL] IMF imposes austerity conditions for bailout. Public outrage!" << std::endl;
        }

        // Foreign powers meddle in politics
        if (playerCountry.security.foreign_disinfo_operations > 0.3 && dist(rng) < 0.1) {
            playerCountry.politics.polarization_index += 0.03;
            playerCountry.security.fake_news_success_prob += 0.02;
            std::cout << "[INTERNATIONAL] Foreign disinformation campaign detected. Polarization rising." << std::endl;
        }

        // NGOs report on human rights
        if (playerCountry.welfare.torture_index > 0.3 && dist(rng) < 0.2) {
            playerCountry.welfare.un_score -= 0.02;
            playerCountry.security.diplomatic_prestige -= 0.01;
            std::cout << "[INTERNATIONAL] Human rights NGO publishes critical report." << std::endl;
        }
    }

    // --- AUTONOMOUS ACTORS: PEOPLE / CIVIL SOCIETY ---
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // Spontaneous protests during deteriorating conditions
        if (playerCountry.welfare.poverty_rate > 0.4 && playerCountry.welfare.unemployment_rate > 0.15
            && dist(rng) < 0.2) {
            playerCountry.politics.marches += 1;
            playerCountry.politics.protest_intensity += 0.05;
            if (playerCountry.politics.protest_intensity > 1.0) playerCountry.politics.protest_intensity = 1.0;
            std::cout << "[PEOPLE] Spontaneous protests erupt over economic hardship." << std::endl;
        }

        // Brain drain during crises
        if ((playerCountry.security.war_active || playerCountry.economy.in_recession
             || playerCountry.politics.democratic_backsliding_index > 0.6) && dist(rng) < 0.3) {
            playerCountry.welfare.brain_drain += 0.01;
            if (playerCountry.welfare.brain_drain > 0.3) playerCountry.welfare.brain_drain = 0.3;
            playerCountry.welfare.diaspora_population += (int)(playerCountry.welfare.population * 0.001);
            std::cout << "[PEOPLE] Educated professionals emigrating. Brain drain accelerating." << std::endl;
        }

        // Vigilante justice when impunity is extreme
        if (playerCountry.politics.impunity > 0.8 && dist(rng) < 0.1) {
            playerCountry.security.homicide_rate += 0.5;
            playerCountry.politics.trust_in_justice -= 0.03;
            if (playerCountry.politics.trust_in_justice < 0.05) playerCountry.politics.trust_in_justice = 0.05;
            std::cout << "[PEOPLE] Vigilante justice: mobs attack suspected criminals. Rule of law collapsing." << std::endl;
        }

        // Social media amplification
        if (playerCountry.security.social_media_reach > 0.7 && playerCountry.politics.active_scandals > 0
            && dist(rng) < 0.25) {
            playerCountry.politics.media_exposure_intensity += 0.05;
            playerCountry.politics.protest_intensity += 0.02;
            std::cout << "[PEOPLE] Scandal goes viral on social media. Hashtag campaigns trending." << std::endl;
        }
    }

    // --- AUTONOMOUS ACTORS: PROSECUTOR ---
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // Prosecutor opens investigations against corrupt officials
        if (playerCountry.politics.anticorruption_enforcement > 0.4
            && playerCountry.politics.administrative_corruption > 0.3
            && dist(rng) < playerCountry.politics.anticorruption_enforcement * 0.1) {
            // Plea bargain: exposes larger network
            if (dist(rng) < 0.4) {
                playerCountry.politics.administrative_corruption -= 0.02;
                playerCountry.politics.popularity += 0.01;
                std::cout << "[PROSECUTOR] Plea bargain reveals corruption network. More officials implicated." << std::endl;
            } else {
                playerCountry.politics.impunity += 0.01;
                std::cout << "[PROSECUTOR] Investigation stalls. Key witnesses refuse to cooperate." << std::endl;
            }
        }

        // International court cooperation
        if (playerCountry.welfare.torture_index > 0.3
            && playerCountry.welfare.un_score < 0.5
            && dist(rng) < 0.05) {
            std::cout << "[PROSECUTOR] International Criminal Court opens preliminary examination!" << std::endl;
            playerCountry.security.diplomatic_prestige -= 0.05;
            playerCountry.economy.international_sanctions_prob += 0.03;
        }
    }

    // --- SCANDAL SYSTEM ---
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // SCANDAL GENERATION: Each turn, roll for new scandals
        if (playerCountry.politics.scandal_corruption_severity == 0.0
            && dist(rng) < playerCountry.politics.administrative_corruption * 0.1) {
            std::uniform_real_distribution<double> sev(0.2, 0.8);
            playerCountry.politics.scandal_corruption_severity = sev(rng);
            playerCountry.politics.active_scandals++;
            std::cout << "[SCANDAL] CORRUPTION: Government officials caught in embezzlement scheme! Severity: "
                      << (int)(playerCountry.politics.scandal_corruption_severity * 100) << "%" << std::endl;
        }
        if (playerCountry.politics.scandal_sex_severity == 0.0 && dist(rng) < 0.02) {
            std::uniform_real_distribution<double> sev(0.1, 0.6);
            playerCountry.politics.scandal_sex_severity = sev(rng);
            playerCountry.politics.active_scandals++;
            std::cout << "[SCANDAL] SEX: Leader's personal scandal exposed by media!" << std::endl;
        }
        if (playerCountry.politics.scandal_financial_severity == 0.0
            && dist(rng) < playerCountry.politics.grand_corruption * 0.08) {
            std::uniform_real_distribution<double> sev(0.3, 0.9);
            playerCountry.politics.scandal_financial_severity = sev(rng);
            playerCountry.politics.active_scandals++;
            std::cout << "[SCANDAL] FINANCIAL: Insider trading and tax evasion uncovered! Severity: "
                      << (int)(playerCountry.politics.scandal_financial_severity * 100) << "%" << std::endl;
        }
        if (playerCountry.politics.scandal_violence_severity == 0.0 && dist(rng) < 0.01) {
            std::uniform_real_distribution<double> sev(0.3, 0.7);
            playerCountry.politics.scandal_violence_severity = sev(rng);
            playerCountry.politics.active_scandals++;
            std::cout << "[SCANDAL] VIOLENCE: Assault allegations against high-ranking official!" << std::endl;
        }
        if (playerCountry.politics.scandal_substance_severity == 0.0 && dist(rng) < 0.015) {
            std::uniform_real_distribution<double> sev(0.1, 0.5);
            playerCountry.politics.scandal_substance_severity = sev(rng);
            playerCountry.politics.active_scandals++;
            std::cout << "[SCANDAL] SUBSTANCE: Drug use in government circles exposed!" << std::endl;
        }
        if (playerCountry.politics.scandal_treason_severity == 0.0
            && dist(rng) < playerCountry.security.espionage_budget / 10000000.0 * 0.05) {
            std::uniform_real_distribution<double> sev(0.5, 1.0);
            playerCountry.politics.scandal_treason_severity = sev(rng);
            playerCountry.politics.active_scandals++;
            std::cout << "[SCANDAL] TREASON: Espionage links to foreign power discovered! Severity: "
                      << (int)(playerCountry.politics.scandal_treason_severity * 100) << "%" << std::endl;
        }

        // --- EXPANDED SCANDAL TYPES ---
        if (playerCountry.politics.scandal_organized_crime_severity == 0.0
            && dist(rng) < playerCountry.security.homicide_rate / 200.0 * 0.05) {
            std::uniform_real_distribution<double> sev(0.4, 0.9);
            playerCountry.politics.scandal_organized_crime_severity = sev(rng);
            playerCountry.politics.active_scandals++;
            std::cout << "[SCANDAL] ORGANIZED CRIME: Government officials linked to cartel operations!" << std::endl;
        }
        if (playerCountry.politics.scandal_trafficking_severity == 0.0
            && dist(rng) < playerCountry.security.nsg_territorial_control * 0.03) {
            std::uniform_real_distribution<double> sev(0.5, 1.0);
            playerCountry.politics.scandal_trafficking_severity = sev(rng);
            playerCountry.politics.active_scandals++;
            std::cout << "[SCANDAL] TRAFFICKING: Human/arms trafficking ring tied to state officials!" << std::endl;
        }
        if (playerCountry.politics.scandal_money_laundering_severity == 0.0
            && dist(rng) < playerCountry.politics.grand_corruption * 0.06) {
            std::uniform_real_distribution<double> sev(0.3, 0.8);
            playerCountry.politics.scandal_money_laundering_severity = sev(rng);
            playerCountry.politics.active_scandals++;
            std::cout << "[SCANDAL] MONEY LAUNDERING: Offshore accounts and shell companies exposed!" << std::endl;
        }
        if (playerCountry.politics.scandal_blackmail_severity == 0.0
            && dist(rng) < playerCountry.security.cyber_surveillance * 0.03) {
            std::uniform_real_distribution<double> sev(0.3, 0.7);
            playerCountry.politics.scandal_blackmail_severity = sev(rng);
            playerCountry.politics.active_scandals++;
            std::cout << "[SCANDAL] BLACKMAIL: State surveillance data used for political coercion!" << std::endl;
        }
        if (playerCountry.politics.scandal_extortion_severity == 0.0
            && dist(rng) < playerCountry.politics.impunity * 0.04) {
            std::uniform_real_distribution<double> sev(0.2, 0.6);
            playerCountry.politics.scandal_extortion_severity = sev(rng);
            playerCountry.politics.active_scandals++;
            std::cout << "[SCANDAL] EXTORTION: Officials running protection rackets exposed!" << std::endl;
        }
        if (playerCountry.politics.scandal_environmental_severity == 0.0
            && dist(rng) < playerCountry.economy.mining_legacy_damage * 0.05) {
            std::uniform_real_distribution<double> sev(0.3, 0.8);
            playerCountry.politics.scandal_environmental_severity = sev(rng);
            playerCountry.politics.active_scandals++;
            std::cout << "[SCANDAL] ENVIRONMENTAL: Illegal dumping and pollution cover-up revealed!" << std::endl;
        }

        // SCANDAL EXPOSURE: Cover-up vs press freedom
        double total_scandal_severity = playerCountry.politics.scandal_corruption_severity
                                      + playerCountry.politics.scandal_sex_severity
                                      + playerCountry.politics.scandal_financial_severity
                                      + playerCountry.politics.scandal_violence_severity
                                      + playerCountry.politics.scandal_substance_severity
                                      + playerCountry.politics.scandal_treason_severity
                                      + playerCountry.politics.scandal_organized_crime_severity
                                      + playerCountry.politics.scandal_trafficking_severity
                                      + playerCountry.politics.scandal_money_laundering_severity
                                      + playerCountry.politics.scandal_blackmail_severity
                                      + playerCountry.politics.scandal_extortion_severity
                                      + playerCountry.politics.scandal_environmental_severity;

        if (total_scandal_severity > 0.0) {
            // Cover-up attempt: success depends on media control vs press freedom
            double cover_success = playerCountry.politics.cover_up_probability
                                 * (1.0 - playerCountry.security.press_freedom);
            if (dist(rng) > cover_success) {
                // Cover-up fails: media exposure spikes
                playerCountry.politics.media_exposure_intensity = total_scandal_severity * playerCountry.security.press_freedom;
                if (playerCountry.politics.media_exposure_intensity > 1.0) playerCountry.politics.media_exposure_intensity = 1.0;
            } else {
                playerCountry.politics.media_exposure_intensity *= 0.7; // Suppressed, slowly fading
            }

            // INVESTIGATION: Prosecutor may investigate
            double investigation_prob = playerCountry.politics.anticorruption_enforcement
                                      * (1.0 - playerCountry.politics.impunity) * 0.2;
            if (dist(rng) < investigation_prob && total_scandal_severity > 0.3) {
                std::cout << "[INVESTIGATION] Prosecutor opens formal inquiry into government scandal." << std::endl;
                // Conviction chance
                double conviction_prob = investigation_prob * 0.5;
                if (dist(rng) < conviction_prob) {
                    std::cout << "[CONVICTION] Official found guilty! Scandal partially resolved." << std::endl;
                    // Reduce largest active scandal
                    if (playerCountry.politics.scandal_corruption_severity > 0.3)
                        playerCountry.politics.scandal_corruption_severity *= 0.3;
                    else if (playerCountry.politics.scandal_financial_severity > 0.3)
                        playerCountry.politics.scandal_financial_severity *= 0.3;
                    playerCountry.politics.popularity += 0.02; // Justice works
                    playerCountry.politics.trust_in_justice += 0.02;
                }
            }

            // POPULARITY IMPACT: Scandals erode support
            double scandal_impact = total_scandal_severity * playerCountry.politics.media_exposure_intensity * 0.05;
            // Scandal fatigue: diminishing returns
            if (playerCountry.politics.scandal_fatigue_turns > 3) {
                scandal_impact *= 0.5;
            }
            playerCountry.politics.popularity -= scandal_impact;
            playerCountry.politics.scandal_fatigue_turns++;

            // CASCADING EFFECTS by scandal type
            if (playerCountry.politics.scandal_corruption_severity > 0.3) {
                playerCountry.infra.fdi_inflow_gdp -= 0.002;
                if (playerCountry.infra.fdi_inflow_gdp < 0.0) playerCountry.infra.fdi_inflow_gdp = 0.0;
                playerCountry.politics.corruption_perception_index -= 1.0;
                if (playerCountry.politics.corruption_perception_index < 0.0) playerCountry.politics.corruption_perception_index = 0.0;
            }
            if (playerCountry.politics.scandal_financial_severity > 0.3) {
                playerCountry.infra.capital_flight_risk += 0.01;
                playerCountry.economy.exchange_rate_stability -= 0.01;
            }
            if (playerCountry.politics.scandal_violence_severity > 0.3) {
                playerCountry.welfare.un_score -= 0.02;
                playerCountry.economy.international_sanctions_prob += 0.01;
            }
            if (playerCountry.politics.scandal_treason_severity > 0.3) {
                playerCountry.security.diplomatic_prestige -= 0.03;
                playerCountry.security.diplomatic_crisis_active = true;
                playerCountry.security.diplomatic_crisis_duration = 3;
            }
            // Expanded scandal cascading effects
            if (playerCountry.politics.scandal_organized_crime_severity > 0.3) {
                playerCountry.economy.international_sanctions_prob += 0.02;
                playerCountry.security.homicide_rate += 0.5;
                playerCountry.politics.trust_in_justice -= 0.02;
                playerCountry.infra.fdi_inflow_gdp -= 0.003;
            }
            if (playerCountry.politics.scandal_trafficking_severity > 0.3) {
                playerCountry.economy.international_sanctions_prob += 0.03;
                playerCountry.welfare.un_score -= 0.03;
                playerCountry.security.diplomatic_prestige -= 0.03;
                playerCountry.welfare.torture_index += 0.02;
            }
            if (playerCountry.politics.scandal_money_laundering_severity > 0.3) {
                playerCountry.economy.exchange_rate_stability -= 0.02;
                playerCountry.infra.capital_flight_risk += 0.02;
                playerCountry.politics.corruption_perception_index -= 2.0;
            }
            if (playerCountry.politics.scandal_blackmail_severity > 0.3) {
                playerCountry.security.press_freedom -= 0.02;
                playerCountry.politics.trust_in_justice -= 0.02;
                playerCountry.politics.democratic_backsliding_index += 0.01;
            }
            if (playerCountry.politics.scandal_extortion_severity > 0.3) {
                playerCountry.politics.impunity += 0.01;
                playerCountry.infra.investment_climate_index -= 0.02;
            }
            if (playerCountry.politics.scandal_environmental_severity > 0.3) {
                playerCountry.infra.pollution_prob += 0.02;
                playerCountry.economy.community_conflicts += 0.03;
                playerCountry.security.diplomatic_prestige -= 0.01;
                playerCountry.infra.biodiversity_index -= 0.01;
            }
            // Multiple simultaneous scandals compound exponentially
            if (playerCountry.politics.active_scandals >= 3) {
                double compound_penalty = playerCountry.politics.active_scandals * 0.01;
                playerCountry.politics.popularity -= compound_penalty;
                playerCountry.politics.regime_legitimacy -= compound_penalty * 0.5;
                if (playerCountry.politics.regime_legitimacy < 0.1) playerCountry.politics.regime_legitimacy = 0.1;
            }

            // SCANDAL RESOLUTION: Scandals fade after 4-6 turns
            auto decay_scandal = [&](double& severity) {
                if (severity > 0.0) {
                    severity -= 0.15;
                    if (severity <= 0.05) {
                        severity = 0.0;
                        playerCountry.politics.active_scandals--;
                        if (playerCountry.politics.active_scandals < 0) playerCountry.politics.active_scandals = 0;
                    }
                }
            };
            decay_scandal(playerCountry.politics.scandal_corruption_severity);
            decay_scandal(playerCountry.politics.scandal_sex_severity);
            decay_scandal(playerCountry.politics.scandal_financial_severity);
            decay_scandal(playerCountry.politics.scandal_violence_severity);
            decay_scandal(playerCountry.politics.scandal_substance_severity);
            decay_scandal(playerCountry.politics.scandal_treason_severity);
            decay_scandal(playerCountry.politics.scandal_organized_crime_severity);
            decay_scandal(playerCountry.politics.scandal_trafficking_severity);
            decay_scandal(playerCountry.politics.scandal_money_laundering_severity);
            decay_scandal(playerCountry.politics.scandal_blackmail_severity);
            decay_scandal(playerCountry.politics.scandal_extortion_severity);
            decay_scandal(playerCountry.politics.scandal_environmental_severity);

            // Display active scandals summary
            if (playerCountry.politics.active_scandals > 0) {
                std::cout << "[SCANDALS] Active: " << playerCountry.politics.active_scandals
                          << " | Exposure: " << (int)(playerCountry.politics.media_exposure_intensity * 100)
                          << "% | Impact on popularity: -" << (int)(scandal_impact * 100) << "%" << std::endl;
            }
        } else {
            // No active scandals: reset fatigue
            playerCountry.politics.scandal_fatigue_turns = 0;
            playerCountry.politics.media_exposure_intensity *= 0.5; // Fade
        }
    }

    // --- AUTHORITARIAN ENDGAME & DEMOCRATIC RESILIENCE ---
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // Authoritarian consolidation effects
        if (playerCountry.politics.authoritarian_actions_count >= 3) {
            // No more elections in practice
            playerCountry.politics.democratic_backsliding_index += 0.01;
            if (playerCountry.politics.democratic_backsliding_index > 1.0) playerCountry.politics.democratic_backsliding_index = 1.0;
            // Regime fragility: no institutional shock absorbers
            // Any crisis is amplified
            if (playerCountry.economy.in_recession) {
                playerCountry.politics.popular_pressure += 0.05;
                playerCountry.politics.popularity -= 0.02; // Extra penalty
            }
            // International isolation
            playerCountry.infra.fdi_inflow_gdp *= 0.98;
            playerCountry.economy.annual_visitors = (int)(playerCountry.economy.annual_visitors * 0.98);
            // Constant coup risk
            playerCountry.politics.coup_d_etat_prob += 0.005;
        }

        // Regime legitimacy erosion: continuous
        if (playerCountry.politics.regime_legitimacy < 0.3 && playerCountry.politics.authoritarian_actions_count > 0) {
            // Fragile regime: any shock can topple
            if (dist(rng) < 0.05) {
                std::cout << "[!!!] REGIME FRAGILITY: Legitimacy critically low. Internal factions plotting." << std::endl;
                playerCountry.politics.coup_d_etat_prob += 0.03;
            }
        }

        // Democratic resilience: strong institutions resist authoritarianism
        // Judiciary blocks court packing if strong
        if (playerCountry.politics.judicial_independence > 0.7
            && playerCountry.politics.democratic_backsliding_index > 0.4
            && dist(rng) < 0.1) {
            playerCountry.politics.democratic_backsliding_index -= 0.02;
            std::cout << "[DEMOCRACY] Independent judiciary pushes back against executive overreach." << std::endl;
        }

        // Free press exposes manipulation
        if (playerCountry.security.press_freedom > 0.7
            && playerCountry.politics.election_rigged
            && dist(rng) < 0.2) {
            playerCountry.politics.popularity -= 0.05;
            playerCountry.politics.regime_legitimacy -= 0.05;
            std::cout << "[DEMOCRACY] Free press exposes election rigging. Public trust collapses." << std::endl;
        }

        // Civil society strength: high education + press = democratic resilience
        if (playerCountry.welfare.educational_quality > 0.7
            && playerCountry.security.press_freedom > 0.6
            && playerCountry.politics.authoritarian_actions_count > 0
            && dist(rng) < 0.1) {
            playerCountry.politics.protest_intensity += 0.05;
            playerCountry.politics.democratic_backsliding_index -= 0.01;
            std::cout << "[DEMOCRACY] Educated civil society resists authoritarian drift." << std::endl;
        }
    }

    // --- INTERNAL PRESSURE SYSTEM ---
    {
        // Congressional pressure: f(opposition, scandals, low popularity)
        playerCountry.politics.congressional_pressure =
            (1.0 - playerCountry.politics.congressional_support) * 0.4
            + playerCountry.politics.active_scandals * 0.1
            + (1.0 - playerCountry.politics.popularity) * 0.3
            + (playerCountry.politics.legislative_crisis_active ? 0.2 : 0.0);
        if (playerCountry.politics.congressional_pressure > 1.0) playerCountry.politics.congressional_pressure = 1.0;

        // Judicial pressure: f(investigations, rulings, scandal severity)
        playerCountry.politics.judicial_pressure =
            playerCountry.politics.ruling_against_state_prob * 0.3
            + (playerCountry.politics.scandal_corruption_severity + playerCountry.politics.scandal_financial_severity) * 0.3
            + playerCountry.politics.anticorruption_enforcement * (1.0 - playerCountry.politics.impunity) * 0.2;
        if (playerCountry.politics.judicial_pressure > 1.0) playerCountry.politics.judicial_pressure = 1.0;

        // Military pressure: f(ideology mismatch, budget, morale)
        playerCountry.politics.military_pressure =
            (playerCountry.security.military_insubordination_prob) * 0.3
            + (playerCountry.security.military_spending_gdp < 0.015 ? 0.2 : 0.0)
            + (1.0 - playerCountry.security.troop_morale) * 0.2
            + (playerCountry.politics.economic_ideology < 0.3 ? 0.15 : 0.0);
        if (playerCountry.politics.military_pressure > 1.0) playerCountry.politics.military_pressure = 1.0;

        // Popular pressure: f(protests, polarization, economic crisis)
        playerCountry.politics.popular_pressure =
            playerCountry.politics.protest_intensity * 0.4
            + playerCountry.politics.polarization_index * 0.2
            + (playerCountry.economy.in_recession ? 0.15 : 0.0)
            + (playerCountry.welfare.poverty_rate > 0.4 ? 0.15 : 0.0);
        if (playerCountry.politics.popular_pressure > 1.0) playerCountry.politics.popular_pressure = 1.0;

        // International pressure: f(sanctions, human rights, backsliding)
        playerCountry.politics.international_pressure =
            playerCountry.economy.international_sanctions_prob * 0.3
            + playerCountry.welfare.torture_index * 0.2
            + playerCountry.politics.democratic_backsliding_index * 0.3
            + (1.0 - playerCountry.welfare.un_score) * 0.2;
        if (playerCountry.politics.international_pressure > 1.0) playerCountry.politics.international_pressure = 1.0;

        // Total pressure composite
        playerCountry.politics.total_pressure =
            playerCountry.politics.congressional_pressure * 0.25
            + playerCountry.politics.judicial_pressure * 0.15
            + playerCountry.politics.military_pressure * 0.25
            + playerCountry.politics.popular_pressure * 0.20
            + playerCountry.politics.international_pressure * 0.15;

        // Display warnings at thresholds
        if (playerCountry.politics.total_pressure > 0.7) {
            std::cout << "[!!!] PRESSURE CRITICAL: Total " << (int)(playerCountry.politics.total_pressure * 100)
                      << "% | Congress: " << (int)(playerCountry.politics.congressional_pressure * 100)
                      << "% | Military: " << (int)(playerCountry.politics.military_pressure * 100)
                      << "% | People: " << (int)(playerCountry.politics.popular_pressure * 100)
                      << "% | Intl: " << (int)(playerCountry.politics.international_pressure * 100) << "%" << std::endl;
        } else if (playerCountry.politics.total_pressure > 0.5) {
            std::cout << "[!!] PRESSURE RISING: " << (int)(playerCountry.politics.total_pressure * 100) << "%" << std::endl;
        }

        // FORCED RESIGNATION: When total pressure exceeds threshold
        if (playerCountry.politics.total_pressure > 0.9) {
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            if (dist(rng) < (playerCountry.politics.total_pressure - 0.8)) {
                // Determine mechanism of removal
                if (playerCountry.politics.military_pressure > 0.8) {
                    std::cout << "[!!!!!] MILITARY ULTIMATUM: Armed forces demand resignation. Regime falls." << std::endl;
                } else if (playerCountry.politics.congressional_pressure > 0.8) {
                    std::cout << "[!!!!!] CONGRESSIONAL REMOVAL: Legislature forces resignation." << std::endl;
                } else if (playerCountry.politics.popular_pressure > 0.9) {
                    std::cout << "[!!!!!] POPULAR UPRISING: Millions in the streets force resignation." << std::endl;
                } else {
                    std::cout << "[!!!!!] FORCED RESIGNATION: Unsustainable pressure from all sides." << std::endl;
                }
                std::cout << "GAME OVER." << std::endl;
                exit(0);
            }
        }
    }

    // --- IDEOLOGY ALIGNMENT SCORING ---
    {
        // Population preferences drift based on economic conditions
        if (playerCountry.economy.in_recession || playerCountry.welfare.unemployment_rate > 0.1) {
            // During hardship, population shifts left (wants state help)
            playerCountry.politics.population_economic_pref -= 0.02;
            if (playerCountry.politics.population_economic_pref < 0.1) playerCountry.politics.population_economic_pref = 0.1;
        } else if (playerCountry.economy.growth_rate > 0.04) {
            // During prosperity, population tolerates free market
            playerCountry.politics.population_economic_pref += 0.01;
            if (playerCountry.politics.population_economic_pref > 0.9) playerCountry.politics.population_economic_pref = 0.9;
        }

        // Ideology alignment → popularity bonus/penalty
        double econ_gap = std::abs(playerCountry.politics.economic_ideology - playerCountry.politics.population_economic_pref);
        double social_gap = std::abs(playerCountry.politics.social_ideology - playerCountry.politics.population_social_pref);
        double total_gap = (econ_gap + social_gap) / 2.0;
        playerCountry.politics.ideology_alignment_bonus = (0.3 - total_gap) * 0.1; // +0.03 when aligned, -0.02 when misaligned
        playerCountry.politics.popularity += playerCountry.politics.ideology_alignment_bonus;

        // Opposition adapts: moves opposite to maximize political distance
        playerCountry.politics.opposition_ideology += (0.5 - playerCountry.politics.economic_ideology) * 0.05;
        if (playerCountry.politics.opposition_ideology < 0.1) playerCountry.politics.opposition_ideology = 0.1;
        if (playerCountry.politics.opposition_ideology > 0.9) playerCountry.politics.opposition_ideology = 0.9;

        // Polarization increases when ideological distance is high
        double opposition_distance = std::abs(playerCountry.politics.economic_ideology - playerCountry.politics.opposition_ideology);
        if (opposition_distance > 0.5) {
            playerCountry.politics.polarization_index += 0.005;
            if (playerCountry.politics.polarization_index > 1.0) playerCountry.politics.polarization_index = 1.0;
        }

        // Auth/dem axis affects institutional quality
        if (playerCountry.politics.auth_dem_axis > 0.6) {
            playerCountry.politics.democratic_backsliding_index += 0.005;
            if (playerCountry.politics.democratic_backsliding_index > 1.0) playerCountry.politics.democratic_backsliding_index = 1.0;
        }
    }

    // --- POPULARITY DYNAMICS ---
    // Honeymoon effect: fresh mandate gives a buffer that decays each turn
    if (playerCountry.politics.honeymoon_turns_remaining > 0) {
        playerCountry.politics.popularity += 0.01; // Small honeymoon uplift
        playerCountry.politics.honeymoon_turns_remaining--;
    }
    // Track popularity trend (smoothed delta from last turn)
    static double last_popularity = playerCountry.politics.popularity;
    playerCountry.politics.popularity_trend = (playerCountry.politics.popularity - last_popularity) * 0.7
                                            + playerCountry.politics.popularity_trend * 0.3;
    last_popularity = playerCountry.politics.popularity;
    // Crisis floor: even in worst case, some base support remains
    if (playerCountry.politics.popularity < playerCountry.politics.crisis_approval_floor)
        playerCountry.politics.popularity = playerCountry.politics.crisis_approval_floor;
    // Ceiling
    if (playerCountry.politics.popularity > 1.0) playerCountry.politics.popularity = 1.0;

    // --- ADVANCED ELECTION SYSTEM ---
    turnCount++;

    // Campaign activation: 2 turns before election
    if (turnCount % 4 == 2 && !playerCountry.politics.campaign_active) {
        playerCountry.politics.campaign_active = true;
        playerCountry.politics.campaign_turns_remaining = 2;
        playerCountry.politics.campaign_spending = 0.0;
        playerCountry.politics.opponent_campaign_spending = 0.0;
        // Generate opponent
        playerCountry.politics.opponent_popularity = 0.3 + (1.0 - playerCountry.politics.popularity) * 0.3;
        playerCountry.politics.opponent_ideology_economic = 1.0 - playerCountry.politics.economic_ideology; // Mirror
        playerCountry.politics.opponent_campaign_spending = playerCountry.economy.gdp * 0.002; // Base funding
        // Lobby backing for opposition
        double lobby_backing = playerCountry.politics.industrial_power * 0.3
                             + playerCountry.politics.financial_power * 0.3;
        if (playerCountry.politics.popularity < 0.4) lobby_backing *= 1.5; // Lobbies bet on opposition
        playerCountry.politics.opponent_campaign_spending += lobby_backing * 10000000.0;
        std::cout << "\n=== CAMPAIGN SEASON BEGINS (Election in " << playerCountry.politics.campaign_turns_remaining
                  << " turns) ===" << std::endl;
        std::cout << "   Opponent popularity: " << (int)(playerCountry.politics.opponent_popularity * 100)
                  << "% | Opponent funding: $" << (int)(playerCountry.politics.opponent_campaign_spending / 1000000.0)
                  << "M" << std::endl;
    }

    // Campaign turn: polls, spending effects
    if (playerCountry.politics.campaign_active && turnCount % 4 != 0) {
        playerCountry.politics.campaign_turns_remaining--;

        // Campaign spending effects (diminishing returns)
        double player_spend_effect = std::log(1.0 + playerCountry.politics.campaign_spending / 10000000.0) * 0.02;
        double opponent_spend_effect = std::log(1.0 + playerCountry.politics.opponent_campaign_spending / 10000000.0) * 0.02;

        // Economy is the #1 election factor
        double economy_factor = 0.0;
        if (playerCountry.economy.growth_rate > 0.03) economy_factor = 0.05;
        else if (playerCountry.economy.growth_rate < 0) economy_factor = -0.05;

        // Ideology alignment with population
        double ideology_factor = (0.5 - std::abs(playerCountry.politics.economic_ideology
                                                 - playerCountry.politics.population_economic_pref)) * 0.1;

        // Poll simulation
        std::uniform_real_distribution<> poll_noise(-0.03, 0.03);
        playerCountry.politics.poll_margin = playerCountry.politics.popularity
                                           + playerCountry.politics.incumbent_advantage
                                           + player_spend_effect
                                           + economy_factor
                                           + ideology_factor
                                           - playerCountry.politics.opponent_popularity
                                           - opponent_spend_effect
                                           + poll_noise(rng);

        std::cout << "[POLL] Current margin: " << (playerCountry.politics.poll_margin >= 0 ? "+" : "")
                  << (int)(playerCountry.politics.poll_margin * 100) << " points"
                  << " | Economy factor: " << (economy_factor >= 0 ? "+" : "")
                  << (int)(economy_factor * 100) << "%" << std::endl;
    }

    // Election day
    if (turnCount % 4 == 0) {
        playerCountry.politics.campaign_active = false;
        std::cout << "\n=== ELECTION DAY (Year " << turnCount << ") ===" << std::endl;

        // Term limits check
        bool term_blocked = playerCountry.politics.term_limit_active
                         && playerCountry.politics.terms_served >= 2;

        // Final vote calculation
        double economy_bonus = playerCountry.economy.growth_rate > 0 ? playerCountry.economy.growth_rate * 1.5 : playerCountry.economy.growth_rate * 2.0;
        double scandal_penalty = playerCountry.politics.active_scandals * 0.03;
        double campaign_bonus = std::log(1.0 + playerCountry.politics.campaign_spending / 10000000.0) * 0.03;
        double ideology_bonus = (0.5 - std::abs(playerCountry.politics.economic_ideology - playerCountry.politics.population_economic_pref)) * 0.1;
        double rigging_bonus = playerCountry.politics.election_rigged ? 0.15 : 0.0;

        double effective_vote = playerCountry.politics.popularity
                              + playerCountry.politics.incumbent_advantage
                              + economy_bonus
                              + campaign_bonus
                              + ideology_bonus
                              + rigging_bonus
                              - scandal_penalty;

        // Noise
        std::uniform_real_distribution<> vote_noise(-0.03, 0.03);
        effective_vote += vote_noise(rng);

        double opponent_vote = playerCountry.politics.opponent_popularity
                             + std::log(1.0 + playerCountry.politics.opponent_campaign_spending / 10000000.0) * 0.03;

        double margin = effective_vote - opponent_vote;

        std::cout << "Your vote: " << (int)(effective_vote * 100) << "%"
                  << " | Opponent: " << (int)(opponent_vote * 100) << "%"
                  << " | Margin: " << (margin >= 0 ? "+" : "") << (int)(margin * 100)
                  << " | Terms: " << playerCountry.politics.terms_served;
        if (term_blocked) std::cout << " [TERM-LIMITED]";
        if (playerCountry.politics.election_rigged) std::cout << " [RIGGED]";
        std::cout << std::endl;

        if (term_blocked) {
            std::cout << "TERM LIMIT: Constitutional term limits force transition." << std::endl;
            playerCountry.politics.terms_served = 0;
            playerCountry.politics.popularity *= 0.6;
            playerCountry.politics.honeymoon_turns_remaining = 4;
            playerCountry.politics.incumbent_advantage = 0.05;
            playerCountry.politics.mandate_strength = 0.5;
        } else if (margin > 0) {
            playerCountry.politics.elections_won++;
            playerCountry.politics.terms_served++;
            playerCountry.politics.honeymoon_turns_remaining = 4;
            playerCountry.politics.mandate_strength = std::min(1.0, margin * 2.0);
            playerCountry.politics.incumbent_advantage += 0.02;
            if (playerCountry.politics.incumbent_advantage > 0.3) playerCountry.politics.incumbent_advantage = 0.3;

            if (margin > 0.15) {
                std::cout << "LANDSLIDE VICTORY! Strong mandate for bold reforms." << std::endl;
                playerCountry.politics.congressional_support += 0.1;
                if (playerCountry.politics.congressional_support > 1.0) playerCountry.politics.congressional_support = 1.0;
            } else if (margin > 0.05) {
                std::cout << "COMFORTABLE VICTORY. Solid mandate secured." << std::endl;
            } else {
                std::cout << "NARROW VICTORY. Thin margin means fragile mandate." << std::endl;
                playerCountry.politics.mandate_strength = 0.3;
            }
            playerCountry.politics.election_rigged = false; // Reset for next cycle
        } else {
            playerCountry.politics.elections_lost++;
            if (playerCountry.politics.authoritarian_actions_count >= 3) {
                // Authoritarian refuses to accept defeat
                std::cout << "DEFEAT... but you refuse to concede. Constitutional crisis!" << std::endl;
                playerCountry.politics.regime_legitimacy -= 0.2;
                playerCountry.politics.protest_intensity += 0.3;
                playerCountry.economy.international_sanctions_prob += 0.2;
            } else {
                std::cout << "DEFEAT: The people have chosen change. Peaceful transfer of power." << std::endl;
                if (endCondition == EndCondition::NONE) endCondition = EndCondition::ELECTION_LOSS;
                isRunning = false;
            }
        }
    }

    popularitySum += playerCountry.politics.popularity;
    if (playerCountry.politics.apologize_cooldown_turns > 0) playerCountry.politics.apologize_cooldown_turns--;
    if (playerCountry.politics.threaten_streak_count > 0) playerCountry.politics.threaten_streak_count--;
    EventLoader::tick(scriptedEvents, playerCountry, pendingDecisions, rng, turnCount);
    achievements.recordHistory(playerCountry);
    achievements.evaluate(playerCountry, turnCount, endCondition, initialGdp);
    tutorial.onTurnEnd(playerCountry, turnCount);
    checkGameOver();
    if (!isRunning) {
        achievements.evaluate(playerCountry, turnCount, endCondition, initialGdp);
        renderEndScreen();
    }
}

void Game::render() {
    // Clear screen (hacky way for terminal)
    // std::cout << "\033[2J\033[1;1H";

    std::cout << "\nGame Tick Update:" << std::endl;
    playerCountry.printStatus();

    // --- NEIGHBOR STATUS DISPLAY ---
    std::cout << "--- Neighbors ---" << std::endl;
    for (int i = 0; i < (int)playerCountry.neighbors.size(); i++) {
        const auto& n = playerCountry.neighbors[i];
        std::string rel_label = n.diplomatic_relations > 0.5 ? "ALLIED" :
                                n.diplomatic_relations > 0.0 ? "NEUTRAL" :
                                n.diplomatic_relations > -0.5 ? "TENSE" : "HOSTILE";
        std::cout << "[" << i << "] " << n.name
                  << " | GDP: $" << n.gdp / 1000000.0 << "M"
                  << " | Relations: " << rel_label << " (" << (int)(n.diplomatic_relations * 100) << ")"
                  << " | Trade: $" << n.trade_volume / 1000000.0 << "M"
                  << " | Mil: " << (int)(n.military_strength * 100) << "%";
        if (n.at_war) std::cout << " [AT WAR]";
        if (n.has_territorial_claim) std::cout << " [CLAIM]";
        if (n.sanctions_against_us) std::cout << " [SANCTIONS]";
        if (n.in_crisis) std::cout << " [CRISIS]";
        std::cout << std::endl;
    }
    std::cout << "Commands: improve_relations/worsen_relations/trade_deal/threaten <0-2>, negotiate_peace_neighbor" << std::endl;
    std::cout << "Help: type 'help' for command groups or 'status_brief' for a 5-line summary." << std::endl;
    if (!pendingDecisions.empty()) {
        const auto& d = pendingDecisions.front();
        std::cout << "\n*** DECISION REQUIRED: " << d.prompt << " ***" << std::endl;
        std::cout << "Options: ";
        for (size_t i = 0; i < d.options.size(); ++i) {
            std::cout << d.options[i];
            if (i + 1 < d.options.size()) std::cout << " | ";
        }
        std::cout << std::endl;
    }
    std::cout << "----------------------" << std::endl;
}

void Game::checkGameOver() {
    auto& pol = playerCountry.politics;
    auto& sec = playerCountry.security;
    std::uniform_real_distribution<double> roll(0.0, 1.0);

    auto hasDecision = [&](const std::string& id) {
        return DecisionSystem::hasDecision(pendingDecisions, id);
    };
    if (pol.military_pressure > 0.7 && !hasDecision("coup_threat") && !hasDecision("coup_attempt_imminent")) {
        pendingDecisions.push_back({"coup_threat",
            "El alto mando amenaza con tomar el poder. ¿Tu respuesta?",
            {"purge_military", "negotiate_military", "cede_power", "resist"}});
    }
    if (pol.coup_d_etat_prob > 0.4 && pol.civilian_military_control < 0.5
        && !hasDecision("coup_attempt_imminent")) {
        pendingDecisions.push_back({"coup_attempt_imminent",
            "INTELIGENCIA: conspiración militar inminente. ¿Acción inmediata?",
            {"arrest_plotters", "mobilize_loyal_troops", "flee", "fight_back"}});
        pol.coup_attempts_history++;
    }
    double max_scandal = std::max({pol.scandal_corruption_severity, pol.scandal_sex_severity,
        pol.scandal_financial_severity, pol.scandal_violence_severity, pol.scandal_substance_severity,
        pol.scandal_treason_severity, pol.scandal_organized_crime_severity});
    if (max_scandal > 0.6 && !hasDecision("big_scandal")) {
        pendingDecisions.push_back({"big_scandal",
            "Escándalo mayor en portada. ¿Cómo reaccionas?",
            {"cover_up", "resign_minister", "deny", "accept_responsibility"}});
    }
    if (pol.congressional_pressure > 0.75 && !hasDecision("congress_crisis")
        && !pol.impeachment_in_progress) {
        pendingDecisions.push_back({"congress_crisis",
            "El congreso amenaza con destituirte. ¿Qué haces?",
            {"call_referendum", "dissolve_congress", "negotiate_coalition", "resign"}});
    }
    if (!pol.impeachment_in_progress && pol.congressional_pressure > 0.7
        && pol.popularity < 0.4) {
        pol.impeachment_in_progress = true;
        pol.impeachment_turns = 0;
        pol.impeachment_votes = 0.0;
        std::cout << "[!!!] ACUSACIÓN CONSTITUCIONAL presentada en el congreso." << std::endl;
    }
    if (pol.impeachment_in_progress) {
        pol.impeachment_turns++;
        double base_votes = (pol.opposition_seats / 100.0)
                          + (1.0 - pol.coalition_cohesion) * 0.2
                          + pol.active_scandals * 0.05
                          + (0.5 - pol.popularity) * 0.4;
        if (base_votes < 0.0) base_votes = 0.0;
        pol.impeachment_votes = base_votes;
        if (!hasDecision("impeachment_active")) {
            pendingDecisions.push_back({"impeachment_active",
                "Juicio político en marcha (turno " + std::to_string(pol.impeachment_turns)
                + ", votos " + std::to_string((int)(pol.impeachment_votes * 100)) + "%). Acción:",
                {"negotiate_votes", "threaten_dissolution", "accept_trial"}});
        }
        if (pol.impeachment_votes > 0.66) {
            endCondition = EndCondition::IMPEACHMENT;
            isRunning = false;
            return;
        }
        if (pol.impeachment_votes < 0.33 || pol.impeachment_turns >= 5) {
            std::cout << ">> IMPEACHMENT RECHAZADO: el congreso no reúne los votos." << std::endl;
            pol.impeachment_in_progress = false;
            pol.popularity += 0.1;
            pol.congressional_pressure *= 0.3;
            pol.impeachment_votes = 0.0;
            pol.impeachment_turns = 0;
        }
    }
    for (const auto& n : playerCountry.neighbors) {
        if (n.has_territorial_claim && n.diplomatic_relations < -0.5 && !hasDecision("neighbor_ultimatum")) {
            pendingDecisions.push_back({"neighbor_ultimatum",
                "Un vecino hostil emite un ultimátum territorial. ¿Cómo respondes?",
                {"cede_territory", "mobilize", "seek_mediation"}});
            break;
        }
    }
    if (!pol.revolution_active && pol.revolution_prob > 0.6 && pol.popularity < 0.25) {
        pol.revolution_active = true;
        pol.revolution_duration = 0;
        std::cout << "[!!!] REVOLUCIÓN EN LAS CALLES: el régimen pierde el control." << std::endl;
    }
    if (pol.revolution_active) {
        pol.revolution_duration++;
        playerCountry.economy.gdp *= 0.97;
        pol.popularity -= 0.05;
        if (pol.popularity < 0.0) pol.popularity = 0.0;
        playerCountry.infra.maintenance_level -= 0.05;
        if (!hasDecision("active_revolution")) {
            pendingDecisions.push_back({"active_revolution",
                "Revolución en curso (turno " + std::to_string(pol.revolution_duration) + "). ¿Tu decisión?",
                {"concessions", "crackdown", "step_down"}});
        }
        if (pol.revolution_duration >= 5) {
            endCondition = EndCondition::REVOLUTION;
            isRunning = false;
            return;
        }
    }
    if (sec.nuclear_attack_prob > 0.3 && !hasDecision("nuclear_threat")) {
        pendingDecisions.push_back({"nuclear_threat",
            "Riesgo de ataque nuclear inminente. ¿Tu decisión?",
            {"preemptive_strike", "diplomatic_summit", "evacuate_cities"}});
    }

    EndCondition triggered = GameOverChecker::evaluate(playerCountry, rng);
    if (triggered != EndCondition::NONE) {
        endCondition = triggered;
        isRunning = false;
    }
}

void Game::renderEndScreen() {
    std::string header = Localization::currentLanguage().empty()
        ? "GAME OVER" : Localization::tr("ui.game_over_header");
    std::cout << "\n=================== " << header << " ===================" << std::endl;
    std::string label;
    auto fallback = [&](const std::string& key, const std::string& def) {
        return Localization::currentLanguage().empty() ? def : Localization::tr(key);
    };
    switch (endCondition) {
        case EndCondition::COUP_SUCCESS:         label = fallback("end_conditions.COUP_SUCCESS",         "GOLPE MILITAR EXITOSO"); break;
        case EndCondition::IMPEACHMENT:          label = fallback("end_conditions.IMPEACHMENT",          "DESTITUCION POR EL CONGRESO"); break;
        case EndCondition::REVOLUTION:           label = fallback("end_conditions.REVOLUTION",           "REVOLUCION POPULAR"); break;
        case EndCondition::LAWFARE_REMOVAL:      label = fallback("end_conditions.LAWFARE_REMOVAL",      "INHABILITACION JUDICIAL"); break;
        case EndCondition::ELECTION_LOSS:        label = fallback("end_conditions.ELECTION_LOSS",        "DERROTA ELECTORAL"); break;
        case EndCondition::EXILE:                label = fallback("end_conditions.EXILE",                "EXILIO"); break;
        case EndCondition::ASSASSINATION:        label = fallback("end_conditions.ASSASSINATION",        "MAGNICIDIO"); break;
        case EndCondition::NUCLEAR_ANNIHILATION: label = fallback("end_conditions.NUCLEAR_ANNIHILATION", "ANIQUILACION NUCLEAR"); break;
        case EndCondition::TERM_COMPLETED:       label = fallback("end_conditions.TERM_COMPLETED",       "MANDATO COMPLETADO"); break;
        default:                                 label = "FIN"; break;
    }
    std::cout << "Condicion: " << label << std::endl;
    std::cout << "Turnos en el poder: " << turnCount << std::endl;
    double avg = turnCount > 0 ? popularitySum / turnCount : playerCountry.politics.popularity;
    std::cout << "Popularidad media: " << (int)(avg * 100) << "%" << std::endl;
    std::cout << "Escándalos activos al final: " << playerCountry.politics.active_scandals << std::endl;
    std::cout << "Acciones autoritarias: " << playerCountry.politics.authoritarian_actions_count << std::endl;
    std::cout << "Legitimidad del régimen: " << (int)(playerCountry.politics.regime_legitimacy * 100) << "%" << std::endl;
    std::cout << "Backsliding democrático: " << (int)(playerCountry.politics.democratic_backsliding_index * 100) << "%" << std::endl;
    std::cout << "Guerras vividas (turnos): " << playerCountry.security.war_duration << std::endl;
    std::cout << "==================================================" << std::endl;
}

void Game::resolveDecision(const std::string& choice) {
    if (pendingDecisions.empty()) return;
    PendingDecision d = pendingDecisions.front();
    pendingDecisions.erase(pendingDecisions.begin());
    auto& pol = playerCountry.politics;
    auto& sec = playerCountry.security;

    if (d.id == "coup_threat") {
        if (choice == "purge_military") {
            pol.military_pressure *= 0.4;
            sec.troop_morale -= 0.2;
            pol.authoritarian_actions_count++;
            pol.auth_dem_axis += 0.05;
            pol.democratic_backsliding_index += 0.05;
            std::cout << ">> PURGE: oficiales desafectos removidos. Riesgo de coup baja, moral cae." << std::endl;
        } else if (choice == "negotiate_military") {
            pol.military_pressure *= 0.6;
            sec.military_spending_gdp += 0.005;
            std::cout << ">> NEGOCIACIÓN: concesiones al alto mando. Presión cede algo." << std::endl;
        } else if (choice == "cede_power") {
            pol.military_pressure = 0.95;
            pol.coup_success_prob = 0.95;
            std::cout << ">> CESIÓN: el ejército toma el control de facto." << std::endl;
        } else if (choice == "resist") {
            pol.military_pressure += 0.1;
            pol.popularity += 0.05;
            std::cout << ">> RESISTENCIA: discurso desafiante. El pueblo aplaude, el riesgo escala." << std::endl;
        } else {
            std::cout << ">> Opción no válida; decisión queda pendiente." << std::endl;
            pendingDecisions.insert(pendingDecisions.begin(), d);
        }
    } else if (d.id == "coup_attempt_imminent") {
        std::uniform_real_distribution<double> roll(0.0, 1.0);
        if (choice == "arrest_plotters") {
            if (roll(rng) < sec.humint_capability + 0.2) {
                pol.coup_d_etat_prob *= 0.2;
                pol.military_pressure *= 0.3;
                pol.civilian_military_control += 0.1;
                std::cout << ">> ARRESTOS: conspiradores detenidos. La trama se desactiva." << std::endl;
            } else {
                pol.military_pressure = 0.95;
                pol.coup_d_etat_prob = 0.9;
                std::cout << ">> FALLO: los conspiradores escapan y aceleran el golpe." << std::endl;
            }
        } else if (choice == "mobilize_loyal_troops") {
            if (roll(rng) > pol.coup_success_prob) {
                pol.military_pressure *= 0.3;
                pol.coup_d_etat_prob *= 0.5;
                sec.troop_morale -= 0.1;
                std::cout << ">> CONTRAGOLPE EXITOSO: tropas leales repelen la asonada." << std::endl;
            } else {
                endCondition = EndCondition::COUP_SUCCESS;
                isRunning = false;
                std::cout << ">> DERROTA: las tropas leales son superadas. Golpe consumado." << std::endl;
            }
        } else if (choice == "flee") {
            endCondition = EndCondition::EXILE;
            isRunning = false;
            std::cout << ">> EXILIO: dejas el país antes del asalto al palacio." << std::endl;
        } else if (choice == "fight_back") {
            sec.war_casualties += 5000.0;
            pol.civil_war_active = true;
            if (roll(rng) > pol.coup_success_prob - 0.1) {
                pol.military_pressure *= 0.4;
                pol.popularity -= 0.05;
                std::cout << ">> COMBATE: el palacio resiste, hay muertos civiles." << std::endl;
            } else {
                endCondition = EndCondition::ASSASSINATION;
                isRunning = false;
                std::cout << ">> CAÍDA: el palacio cae bajo el asalto." << std::endl;
            }
        } else {
            std::cout << ">> Opción no válida; decisión queda pendiente." << std::endl;
            pendingDecisions.insert(pendingDecisions.begin(), d);
        }
    } else if (d.id == "big_scandal") {
        if (choice == "cover_up") {
            pol.scandal_corruption_severity *= 0.5;
            pol.cover_up_probability -= 0.05;
            pol.judicial_pressure += 0.1;
            std::cout << ">> COVER-UP: severidad cae, pero la justicia sospecha." << std::endl;
        } else if (choice == "resign_minister") {
            pol.scandal_corruption_severity *= 0.3;
            pol.popularity -= 0.03;
            std::cout << ">> RENUNCIA DE MINISTRO: chivo expiatorio absorbe el golpe." << std::endl;
        } else if (choice == "deny") {
            pol.media_exposure_intensity += 0.2;
            pol.polarization_index += 0.05;
            std::cout << ">> NIEGA TODO: la prensa se obsesiona, polarización sube." << std::endl;
        } else if (choice == "accept_responsibility") {
            pol.popularity -= 0.05;
            pol.scandal_corruption_severity *= 0.2;
            pol.regime_legitimacy += 0.05;
            std::cout << ">> ASUME RESPONSABILIDAD: pierde popularidad, gana legitimidad." << std::endl;
        } else {
            std::cout << ">> Opción no válida; decisión queda pendiente." << std::endl;
            pendingDecisions.insert(pendingDecisions.begin(), d);
        }
    } else if (d.id == "congress_crisis") {
        if (choice == "call_referendum") {
            pol.congressional_pressure *= 0.5;
            pol.popular_pressure += 0.1;
            std::cout << ">> REFERÉNDUM: pasa por encima del congreso, juega con el pueblo." << std::endl;
        } else if (choice == "dissolve_congress") {
            pol.congressional_pressure = 0.0;
            pol.auth_dem_axis += 0.15;
            pol.authoritarian_actions_count++;
            pol.democratic_backsliding_index += 0.15;
            pol.regime_legitimacy -= 0.2;
            pol.international_pressure += 0.2;
            std::cout << ">> CONGRESO DISUELTO: movida autoritaria flagrante." << std::endl;
        } else if (choice == "negotiate_coalition") {
            pol.congressional_pressure *= 0.4;
            pol.coalition_cohesion += 0.1;
            pol.legislative_efficiency += 0.05;
            std::cout << ">> COALICIÓN: cede agenda, gana gobernabilidad." << std::endl;
        } else if (choice == "resign") {
            endCondition = EndCondition::IMPEACHMENT;
            isRunning = false;
            std::cout << ">> RENUNCIA: sale antes de que lo echen." << std::endl;
        } else {
            std::cout << ">> Opción no válida; decisión queda pendiente." << std::endl;
            pendingDecisions.insert(pendingDecisions.begin(), d);
        }
    } else if (d.id == "impeachment_active") {
        std::uniform_real_distribution<double> roll(0.0, 1.0);
        if (choice == "negotiate_votes") {
            double cost = pol.lobbying_cost * 20.0;
            if (playerCountry.economy.tax_collection >= cost) {
                playerCountry.economy.tax_collection -= cost;
                pol.impeachment_votes -= 0.15;
                if (pol.impeachment_votes < 0.0) pol.impeachment_votes = 0.0;
                pol.coalition_cohesion += 0.05;
                std::cout << ">> NEGOCIA VOTOS: lobbying intenso, intención de impeachment cae." << std::endl;
            } else {
                std::cout << ">> Sin presupuesto para lobbying." << std::endl;
                pendingDecisions.insert(pendingDecisions.begin(), d);
            }
        } else if (choice == "threaten_dissolution") {
            if (roll(rng) < pol.regime_legitimacy) {
                pol.impeachment_votes -= 0.2;
                pol.auth_dem_axis += 0.05;
                pol.authoritarian_actions_count++;
                std::cout << ">> AMENAZA DE DISOLUCIÓN: congresistas se intimidan." << std::endl;
            } else {
                pol.impeachment_votes += 0.15;
                pol.regime_legitimacy -= 0.1;
                std::cout << ">> AMENAZA FRACASA: congreso se cierra contra ti." << std::endl;
            }
        } else if (choice == "accept_trial") {
            pol.regime_legitimacy += 0.1;
            pol.democratic_backsliding_index -= 0.05;
            if (pol.democratic_backsliding_index < 0.0) pol.democratic_backsliding_index = 0.0;
            std::cout << ">> ACEPTA JUICIO: confías en tu defensa y en las instituciones." << std::endl;
        } else {
            std::cout << ">> Opción no válida; decisión queda pendiente." << std::endl;
            pendingDecisions.insert(pendingDecisions.begin(), d);
        }
    } else if (d.id == "neighbor_ultimatum") {
        if (choice == "cede_territory") {
            for (auto& n : playerCountry.neighbors) {
                if (n.has_territorial_claim) { n.has_territorial_claim = false; n.diplomatic_relations += 0.4; }
            }
            pol.popularity -= 0.15;
            pol.regime_legitimacy -= 0.1;
            std::cout << ">> CESIÓN TERRITORIAL: paz a costa de soberanía." << std::endl;
        } else if (choice == "mobilize") {
            sec.invasion_prob += 0.1;
            sec.troop_morale += 0.05;
            pol.popularity += 0.05;
            std::cout << ">> MOVILIZACIÓN: rally-round-the-flag, pero riesgo de guerra crece." << std::endl;
        } else if (choice == "seek_mediation") {
            sec.diplomatic_prestige += 0.05;
            sec.invasion_prob *= 0.7;
            std::cout << ">> MEDIACIÓN: la comunidad internacional interviene." << std::endl;
        } else {
            std::cout << ">> Opción no válida; decisión queda pendiente." << std::endl;
            pendingDecisions.insert(pendingDecisions.begin(), d);
        }
    } else if (d.id == "active_revolution") {
        std::uniform_real_distribution<double> roll(0.0, 1.0);
        if (choice == "concessions") {
            pol.revolution_prob *= 0.5;
            pol.popularity += 0.05;
            pol.economic_ideology -= 0.05;
            pol.popular_pressure *= 0.6;
            std::cout << ">> CONCESIONES: el régimen gira hacia el pueblo, la calle se enfría." << std::endl;
            if (pol.revolution_prob < 0.3) {
                pol.revolution_active = false;
                pol.revolution_duration = 0;
                std::cout << ">> La revolución se desactiva." << std::endl;
            }
        } else if (choice == "crackdown") {
            if (roll(rng) < sec.humint_capability + 0.2) {
                pol.revolution_prob *= 0.4;
                pol.popular_pressure *= 0.5;
                pol.regime_legitimacy -= 0.1;
                std::cout << ">> REPRESIÓN: protestas dispersadas, pero el régimen se mancha." << std::endl;
                if (pol.revolution_prob < 0.3) {
                    pol.revolution_active = false;
                    pol.revolution_duration = 0;
                }
            } else {
                pol.international_pressure += 0.3;
                pol.revolution_prob += 0.2;
                pol.regime_legitimacy -= 0.3;
                sec.war_casualties += 2000.0;
                std::cout << ">> MASACRE: la represión falla. El mundo condena y la calle se inflama." << std::endl;
            }
        } else if (choice == "step_down") {
            endCondition = EndCondition::REVOLUTION;
            isRunning = false;
            std::cout << ">> RENUNCIA: cedes ante la revolución para evitar masacre." << std::endl;
        } else {
            std::cout << ">> Opción no válida; decisión queda pendiente." << std::endl;
            pendingDecisions.insert(pendingDecisions.begin(), d);
        }
    } else if (d.id == "nuclear_threat") {
        if (choice == "preemptive_strike") {
            sec.nuclear_strike = true;
            sec.nuclear_casualties = 5000000.0;
            sec.diplomatic_prestige = 0.0;
            std::cout << ">> ATAQUE PREVENTIVO: la humanidad cruza un umbral." << std::endl;
        } else if (choice == "diplomatic_summit") {
            sec.nuclear_attack_prob *= 0.3;
            sec.diplomatic_prestige += 0.1;
            std::cout << ">> CUMBRE: riesgo nuclear baja drásticamente." << std::endl;
        } else if (choice == "evacuate_cities") {
            sec.nuclear_attack_prob *= 0.6;
            playerCountry.economy.gdp *= 0.95;
            pol.popularity -= 0.05;
            std::cout << ">> EVACUACIÓN: caos económico pero protección parcial." << std::endl;
        } else {
            std::cout << ">> Opción no válida; decisión queda pendiente." << std::endl;
            pendingDecisions.insert(pendingDecisions.begin(), d);
        }
    } else {
        std::cout << ">> Decisión desconocida descartada." << std::endl;
    }
}

void Game::saveGame(const std::string& path) {
    if (Persistence::save(playerCountry, turnCount, popularitySum, endCondition, path, achievements.serialize()))
        std::cout << ">> SAVE: estado guardado en " << path << std::endl;
    else
        std::cout << ">> SAVE: no se puede escribir " << path << std::endl;
}

void Game::loadGame(const std::string& path) {
    std::string ach_line;
    if (Persistence::load(playerCountry, turnCount, popularitySum, endCondition, path, &ach_line)) {
        achievements.deserialize(ach_line);
        std::cout << ">> LOAD: estado restaurado desde " << path << std::endl;
    } else {
        std::cout << ">> LOAD: no existe " << path << std::endl;
    }
}
