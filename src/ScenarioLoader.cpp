#include "ScenarioLoader.hpp"
#include "MiniYaml.hpp"

namespace ScenarioLoader {

static double getD(const MiniYaml::KV& kv, const std::string& key, double def) {
    auto it = kv.find(key);
    if (it == kv.end()) return def;
    try { return std::stod(it->second); } catch (...) { return def; }
}
static int getI(const MiniYaml::KV& kv, const std::string& key, int def) {
    auto it = kv.find(key);
    if (it == kv.end()) return def;
    try { return std::stoi(it->second); } catch (...) { return def; }
}
static bool getB(const MiniYaml::KV& kv, const std::string& key, bool def) {
    auto it = kv.find(key);
    if (it == kv.end()) return def;
    return it->second == "true" || it->second == "1";
}
static std::string getS(const MiniYaml::KV& kv, const std::string& key, const std::string& def) {
    auto it = kv.find(key);
    if (it == kv.end()) return def;
    return it->second;
}

bool load(const std::string& path, Country& c, Metadata& meta) {
    MiniYaml::KV kv;
    if (!MiniYaml::loadFile(path, kv)) return false;
    meta.id = getS(kv, "id", "unknown");
    meta.name_es = getS(kv, "name_es", "Escenario sin nombre");
    meta.difficulty = getS(kv, "difficulty", "??");
    meta.start_year = getI(kv, "start_year", 0);

    auto P = [&](const std::string& k) { return std::string("initial_country.") + k; };

    c.welfare.population = getI(kv, P("welfare.population"), c.welfare.population);
    c.welfare.poverty_rate = getD(kv, P("welfare.poverty_rate"), c.welfare.poverty_rate);
    c.welfare.literacy_rate = getD(kv, P("welfare.literacy_rate"), c.welfare.literacy_rate);
    c.welfare.unemployment_rate = getD(kv, P("welfare.unemployment_rate"), c.welfare.unemployment_rate);
    c.welfare.health_coverage = getD(kv, P("welfare.health_coverage"), c.welfare.health_coverage);
    c.welfare.torture_index = getD(kv, P("welfare.torture_index"), c.welfare.torture_index);
    c.welfare.freedom_of_expression = getD(kv, P("welfare.freedom_of_expression"), c.welfare.freedom_of_expression);
    c.welfare.un_score = getD(kv, P("welfare.un_score"), c.welfare.un_score);

    c.economy.gdp = getD(kv, P("economy.gdp"), c.economy.gdp);
    c.economy.inflation = getD(kv, P("economy.inflation"), c.economy.inflation);
    c.economy.growth_rate = getD(kv, P("economy.growth_rate"), c.economy.growth_rate);
    c.economy.in_recession = getB(kv, P("economy.in_recession"), c.economy.in_recession);
    c.economy.international_reserves = getD(kv, P("economy.international_reserves"), c.economy.international_reserves);
    c.economy.interest_rate = getD(kv, P("economy.interest_rate"), c.economy.interest_rate);
    c.economy.central_bank_autonomy = getD(kv, P("economy.central_bank_autonomy"), c.economy.central_bank_autonomy);
    c.economy.debt_to_gdp_ratio = getD(kv, P("economy.debt_to_gdp_ratio"), c.economy.debt_to_gdp_ratio);
    int rating = getI(kv, P("economy.credit_rating"), (int)c.economy.credit_rating);
    if (rating >= 0 && rating <= (int)CreditRating::D) c.economy.credit_rating = (CreditRating)rating;
    c.economy.tax_collection = getD(kv, P("economy.tax_collection"), c.economy.tax_collection);

    c.politics.popularity = getD(kv, P("politics.popularity"), c.politics.popularity);
    c.politics.auth_dem_axis = getD(kv, P("politics.auth_dem_axis"), c.politics.auth_dem_axis);
    c.politics.economic_ideology = getD(kv, P("politics.economic_ideology"), c.politics.economic_ideology);
    c.politics.social_ideology = getD(kv, P("politics.social_ideology"), c.politics.social_ideology);
    c.politics.authoritarianism_prob = getD(kv, P("politics.authoritarianism_prob"), c.politics.authoritarianism_prob);
    c.politics.coup_d_etat_prob = getD(kv, P("politics.coup_d_etat_prob"), c.politics.coup_d_etat_prob);
    c.politics.coup_success_prob = getD(kv, P("politics.coup_success_prob"), c.politics.coup_success_prob);
    c.politics.civilian_military_control = getD(kv, P("politics.civilian_military_control"), c.politics.civilian_military_control);
    c.politics.administrative_corruption = getD(kv, P("politics.administrative_corruption"), c.politics.administrative_corruption);
    c.politics.polarization_index = getD(kv, P("politics.polarization_index"), c.politics.polarization_index);
    c.politics.judicial_independence = getD(kv, P("politics.judicial_independence"), c.politics.judicial_independence);
    c.security.press_freedom = getD(kv, P("politics.press_freedom"), c.security.press_freedom);
    c.politics.regime_legitimacy = getD(kv, P("politics.regime_legitimacy"), c.politics.regime_legitimacy);
    c.politics.congressional_support = getD(kv, P("politics.congressional_support"), c.politics.congressional_support);
    c.politics.active_scandals = getI(kv, P("politics.active_scandals"), c.politics.active_scandals);
    c.politics.opposition_seats = getI(kv, P("politics.opposition_seats"), c.politics.opposition_seats);

    c.security.troop_morale = getD(kv, P("security.troop_morale"), c.security.troop_morale);
    c.security.military_spending_gdp = getD(kv, P("security.military_spending_gdp"), c.security.military_spending_gdp);
    c.security.non_state_groups = getI(kv, P("security.non_state_groups"), c.security.non_state_groups);
    c.security.invasion_prob = getD(kv, P("security.invasion_prob"), c.security.invasion_prob);
    c.security.diplomatic_prestige = getD(kv, P("security.diplomatic_prestige"), c.security.diplomatic_prestige);
    c.security.us_alignment = getD(kv, P("security.us_alignment"), c.security.us_alignment);
    c.security.china_alignment = getD(kv, P("security.china_alignment"), c.security.china_alignment);
    c.security.non_aligned_index = getD(kv, P("security.non_aligned_index"), c.security.non_aligned_index);

    return true;
}

} // namespace ScenarioLoader
