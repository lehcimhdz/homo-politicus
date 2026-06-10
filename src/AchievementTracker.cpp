#include "AchievementTracker.hpp"
#include <sstream>

const std::vector<AchievementTracker::Definition>& AchievementTracker::catalog() {
    static std::vector<Definition> defs = {
        {"first_year",          "Primer año",                "Sobrevive 12 turnos"},
        {"decade_in_power",     "Década en el poder",        "Sobrevive 40 turnos"},
        {"historic",            "Histórico",                 "Sobrevive 100 turnos"},
        {"economic_miracle",    "Milagro económico",         "GDP +50% respecto al inicio"},
        {"inflation_tamed",     "Inflación controlada",      "Inflación menor a 3%"},
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
    };
    return defs;
}

void AchievementTracker::tryUnlock(const std::string& id) {
    if (unlocked.insert(id).second) {
        std::cout << "[LOGRO DESBLOQUEADO] " << id << std::endl;
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
    if (end != EndCondition::NONE && turn < 10)   tryUnlock("speedrun_collapse");
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
