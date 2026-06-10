#include "AchievementTracker.hpp"
#include "steam/SteamBridge.hpp"
#include <sstream>

const std::vector<AchievementTracker::Definition>& AchievementTracker::catalog() {
    static std::vector<Definition> defs = {
        {"first_year",          "Primer año",                "Sobrevive 12 turnos"},
        {"decade_in_power",     "Década en el poder",        "Sobrevive 40 turnos"},
        {"historic",            "Histórico",                 "Sobrevive 100 turnos"},
        {"economic_miracle",    "Milagro económico",         "GDP +50% respecto al inicio"},
        {"asian_tiger",         "Tigre asiático",            "GDP +100% en 40 turnos"},
        {"inflation_tamed",     "Inflación controlada",      "Inflación <3% durante 20 turnos"},
        {"default_d",           "Default soberano",          "Credit rating D alcanzado"},
        {"swf_titan",           "Superávit",                 "SWF mayor a $100M"},
        {"caudillo",            "Caudillo",                  "10+ acciones autoritarias"},
        {"constitutionalist",   "Constitucionalista",        "Termina sin acciones autoritarias"},
        {"soft_power",          "Soft power",                "Prestigio diplomático > 0.9"},
        {"regional_allies",     "Aliados regionales",        "3+ vecinos con relaciones > 0.5"},
        {"non_aligned",         "Anti-imperialista",         "non_aligned_index > 0.8"},
        {"ended_by_coup",       "Golpe exitoso",             "End condition COUP_SUCCESS"},
        {"ended_by_nuke",       "Aniquilación nuclear",      "End condition NUCLEAR_ANNIHILATION"},
        {"speedrun_collapse",   "Speedrun colapso",          "Game over antes del turno 10"},
        {"rainbow_coalition",   "Coalición arcoíris",        "Cohesión >0.8 durante 20 turnos"},
        {"crisis_tamer",        "Domador de crisis",         "5 decisiones críticas resueltas"},
        {"nuclear_survivor",    "Sobreviviente nuclear",     "Evitar strike con prob>0.5"},
        {"bloodless",           "Sin sangre",                "Guerra terminada con <1000 bajas"},
        {"lasting_peace",       "Paz duradera",              "30 turnos sin guerra ni civil war"},
        {"peacemaker",          "Pacificador",               "Guerra civil resuelta por negociación"},
        {"teflon",              "Teflón",                    "5 escándalos sin caer bajo 40% popularidad"},
        {"houdini",             "Houdini",                   "3 cover_up exitosos consecutivos"},
        {"martyr",              "Mártir",                    "5 disculpas públicas"},
        {"trade_master",        "Maestro de tratados",       "5 FTA firmados"},
        {"plebiscite_winner",   "Plebiscito",                "Convocar y ganar referéndum"},
        {"voluntary_martyr",    "Mártir popular",            "Renuncia con popularidad >70%"},
        {"golden_exile",        "Exilio dorado",             "EXILE con SWF >$50M"},
        {"ended_by_lawfare",    "Inhabilitación",            "End condition LAWFARE_REMOVAL"},
        {"ended_by_revolution", "Revolución sobrevivida",    "REVOLUTION con 5+ turnos resistiendo"},
        {"argentina_1976_clean","Argentina 1976 limpia",     "Argentina 76 sin acciones autoritarias"},
        {"cuba_1959_open",      "Cuba 1959 abierta",         "Cuba 59 con trade balance positivo"},
        {"venezuela_2013_stable","Venezuela 2013 estable",   "Venezuela 13 sin hiperinflación"},
        {"chile_1973_survives", "Chile 1973 sobrevive",      "Chile 73 pasa el 11 de septiembre"},
        {"singapur_1965_leap",  "Singapur 1965 primer mundo","Singapur 65 con GDP/cap >10K"},
        {"landslide_reelection","Reelección plebiscitaria",  "Ganar elección con >70%"},
        {"high_min_popularity", "Inquebrantable",            "Nunca bajar de 40% popularidad"},
        {"climate_champion",    "Campeón climático",         "Renewables >50% del mix"},
        {"tech_visionary",      "Visionario tech",           "Innovation index >0.85"},
    };
    return defs;
}

void AchievementTracker::recordHistory(const Country& c) {
    if (c.politics.popularity < history.min_popularity) history.min_popularity = c.politics.popularity;
    if (c.economy.inflation > history.max_inflation) history.max_inflation = c.economy.inflation;
    if (c.security.nuclear_attack_prob > history.max_nuclear_attack_prob)
        history.max_nuclear_attack_prob = c.security.nuclear_attack_prob;
    history.consecutive_low_inflation_turns = (c.economy.inflation < 0.03)
        ? history.consecutive_low_inflation_turns + 1 : 0;
    history.consecutive_high_cohesion_turns = (c.politics.coalition_cohesion > 0.8)
        ? history.consecutive_high_cohesion_turns + 1 : 0;
    history.consecutive_no_war_turns = (!c.security.war_active && !c.politics.civil_war_active)
        ? history.consecutive_no_war_turns + 1 : 0;
}

void AchievementTracker::noteScandalRevealed()         { history.scandals_endured++; }
void AchievementTracker::noteApologize()               { history.apologize_count++; }
void AchievementTracker::noteCoverUpSuccess()          { history.consecutive_successful_coverups++; }
void AchievementTracker::noteCoverUpFail()             { history.consecutive_successful_coverups = 0; }
void AchievementTracker::noteCriticalDecisionResolved(){ history.critical_decisions_resolved++; }
void AchievementTracker::noteFTASigned()               { history.fta_count++; }
void AchievementTracker::noteReferendumWon()           { history.referendum_won = true; }
void AchievementTracker::noteCivilWarPeacefulEnd()     { history.civil_war_ended_by_negotiation = true; }

void AchievementTracker::tryUnlock(const std::string& id) {
    if (unlocked.insert(id).second) {
        std::cout << "[LOGRO DESBLOQUEADO] " << id << std::endl;
        SteamBridge::unlockAchievement(id);
    }
}

void AchievementTracker::evaluate(const Country& c, int turn, EndCondition end, double initial_gdp) {
    if (turn >= 12)  tryUnlock("first_year");
    if (turn >= 40)  tryUnlock("decade_in_power");
    if (turn >= 100) tryUnlock("historic");

    if (initial_gdp > 0 && c.economy.gdp / initial_gdp >= 1.5) tryUnlock("economic_miracle");
    if (c.economy.inflation < 0.03) tryUnlock("inflation_tamed");
    if (c.economy.credit_rating == CreditRating::D) tryUnlock("default_d");
    if (c.economy.sovereign_wealth_fund > 100000000.0) tryUnlock("swf_titan");

    if (c.politics.authoritarian_actions_count >= 10) tryUnlock("caudillo");
    if (end != EndCondition::NONE && c.politics.authoritarian_actions_count == 0) tryUnlock("constitutionalist");

    if (c.security.diplomatic_prestige > 0.9) tryUnlock("soft_power");
    if (c.security.non_aligned_index > 0.8) tryUnlock("non_aligned");

    int allies = 0;
    for (const auto& n : c.neighbors) if (n.diplomatic_relations > 0.5) allies++;
    if (allies >= 3) tryUnlock("regional_allies");

    if (end == EndCondition::COUP_SUCCESS)        tryUnlock("ended_by_coup");
    if (end == EndCondition::NUCLEAR_ANNIHILATION) tryUnlock("ended_by_nuke");
    if (end == EndCondition::LAWFARE_REMOVAL)     tryUnlock("ended_by_lawfare");
    if (end != EndCondition::NONE && turn < 10)   tryUnlock("speedrun_collapse");

    // History-based
    if (initial_gdp > 0 && c.economy.gdp / initial_gdp >= 2.0 && turn <= 40)
        tryUnlock("asian_tiger");
    if (history.consecutive_low_inflation_turns >= 20) tryUnlock("inflation_tamed");
    if (history.consecutive_high_cohesion_turns >= 20) tryUnlock("rainbow_coalition");
    if (history.consecutive_no_war_turns >= 30)        tryUnlock("lasting_peace");
    if (history.critical_decisions_resolved >= 5)      tryUnlock("crisis_tamer");
    if (history.max_nuclear_attack_prob > 0.5 && !c.security.nuclear_strike)
        tryUnlock("nuclear_survivor");
    if (history.scandals_endured >= 5 && history.min_popularity >= 0.40)
        tryUnlock("teflon");
    if (history.consecutive_successful_coverups >= 3) tryUnlock("houdini");
    if (history.apologize_count >= 5)                 tryUnlock("martyr");
    if (history.fta_count >= 5)                       tryUnlock("trade_master");
    if (history.referendum_won)                       tryUnlock("plebiscite_winner");
    if (history.civil_war_ended_by_negotiation)       tryUnlock("peacemaker");
    if (history.min_popularity >= 0.40 && turn >= 12) tryUnlock("high_min_popularity");

    if (c.security.war_active == false && c.security.war_duration > 0 &&
        c.security.war_casualties < 1000.0) tryUnlock("bloodless");

    if (end == EndCondition::TERM_COMPLETED && c.politics.popularity > 0.7)
        tryUnlock("voluntary_martyr");
    if (end == EndCondition::EXILE && c.economy.sovereign_wealth_fund > 50000000.0)
        tryUnlock("golden_exile");
    if (end == EndCondition::REVOLUTION && c.politics.revolution_duration >= 5)
        tryUnlock("ended_by_revolution");

    if (c.infra.renewables_percentage > 0.5)      tryUnlock("climate_champion");
    if (c.infra.innovation_index > 0.85)          tryUnlock("tech_visionary");
}

bool AchievementTracker::isUnlocked(const std::string& id) const {
    return unlocked.count(id) > 0;
}

std::vector<std::string> AchievementTracker::unlockedList() const {
    std::vector<std::string> v(unlocked.begin(), unlocked.end());
    return v;
}

std::string AchievementTracker::serialize() const {
    std::ostringstream oss;
    bool first = true;
    for (const auto& id : unlocked) {
        if (!first) oss << ",";
        oss << id;
        first = false;
    }
    return oss.str();
}

void AchievementTracker::deserialize(const std::string& line) {
    unlocked.clear();
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (!token.empty()) unlocked.insert(token);
    }
}
