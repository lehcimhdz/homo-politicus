#include "Persistence.hpp"
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace Persistence {

bool save(const Country& c, int turnCount, double popularitySum,
          EndCondition endCondition, const std::string& path,
          const std::string& achievements_line) {
    std::ofstream f(path);
    if (!f) return false;
    f << "schema_version=2\n";
    if (!achievements_line.empty()) f << "achievements=" << achievements_line << "\n";
    const auto& pol = c.politics;
    const auto& eco = c.economy;
    const auto& sec = c.security;
    const auto& wel = c.welfare;
    f << "turnCount=" << turnCount << "\n";
    f << "popularitySum=" << popularitySum << "\n";
    f << "endCondition=" << (int)endCondition << "\n";
    f << "popularity=" << pol.popularity << "\n";
    f << "gdp=" << eco.gdp << "\n";
    f << "inflation=" << eco.inflation << "\n";
    f << "growth_rate=" << eco.growth_rate << "\n";
    f << "in_recession=" << (eco.in_recession ? 1 : 0) << "\n";
    f << "war_active=" << (sec.war_active ? 1 : 0) << "\n";
    f << "war_duration=" << sec.war_duration << "\n";
    f << "civil_war_active=" << (pol.civil_war_active ? 1 : 0) << "\n";
    f << "pandemic_active=" << (wel.pandemic_active ? 1 : 0) << "\n";
    f << "pandemic_severity=" << wel.pandemic_severity << "\n";
    f << "congressional_pressure=" << pol.congressional_pressure << "\n";
    f << "judicial_pressure=" << pol.judicial_pressure << "\n";
    f << "military_pressure=" << pol.military_pressure << "\n";
    f << "popular_pressure=" << pol.popular_pressure << "\n";
    f << "international_pressure=" << pol.international_pressure << "\n";
    f << "active_scandals=" << pol.active_scandals << "\n";
    f << "scandal_corruption=" << pol.scandal_corruption_severity << "\n";
    f << "scandal_sex=" << pol.scandal_sex_severity << "\n";
    f << "scandal_financial=" << pol.scandal_financial_severity << "\n";
    f << "scandal_violence=" << pol.scandal_violence_severity << "\n";
    f << "scandal_substance=" << pol.scandal_substance_severity << "\n";
    f << "regime_legitimacy=" << pol.regime_legitimacy << "\n";
    f << "democratic_backsliding_index=" << pol.democratic_backsliding_index << "\n";
    f << "authoritarian_actions_count=" << pol.authoritarian_actions_count << "\n";
    f << "auth_dem_axis=" << pol.auth_dem_axis << "\n";
    // Welfare extra
    f << "wel.population=" << c.welfare.population << "\n";
    f << "wel.literacy_rate=" << c.welfare.literacy_rate << "\n";
    f << "wel.unemployment_rate=" << c.welfare.unemployment_rate << "\n";
    f << "wel.health_coverage=" << c.welfare.health_coverage << "\n";
    f << "wel.poverty_rate=" << c.welfare.poverty_rate << "\n";
    f << "wel.union_strength=" << c.welfare.union_strength << "\n";
    f << "wel.minority_protection=" << c.welfare.minority_protection << "\n";
    f << "wel.torture_index=" << c.welfare.torture_index << "\n";
    f << "wel.forced_disappearances=" << c.welfare.forced_disappearances << "\n";
    f << "wel.un_score=" << c.welfare.un_score << "\n";
    f << "wel.radicalism_prob=" << c.welfare.radicalism_prob << "\n";
    f << "wel.freedom_of_expression=" << c.welfare.freedom_of_expression << "\n";
    f << "wel.minimum_wage=" << c.welfare.minimum_wage << "\n";
    f << "wel.retirement_age=" << c.welfare.retirement_age << "\n";
    // Economy extra
    f << "eco.interest_rate=" << c.economy.interest_rate << "\n";
    f << "eco.central_bank_autonomy=" << c.economy.central_bank_autonomy << "\n";
    f << "eco.monetary_emission=" << c.economy.monetary_emission << "\n";
    f << "eco.debt_to_gdp_ratio=" << c.economy.debt_to_gdp_ratio << "\n";
    f << "eco.credit_rating=" << (int)c.economy.credit_rating << "\n";
    f << "eco.sovereign_wealth_fund=" << c.economy.sovereign_wealth_fund << "\n";
    f << "eco.exchange_rate_stability=" << c.economy.exchange_rate_stability << "\n";
    f << "eco.commodity_supercycle=" << c.economy.commodity_supercycle << "\n";
    f << "eco.commodity_prices=" << c.economy.commodity_prices << "\n";
    f << "eco.mining_concessions=" << c.economy.mining_concessions << "\n";
    f << "eco.average_tariffs=" << c.economy.average_tariffs << "\n";
    f << "eco.trade_openness=" << c.economy.trade_openness << "\n";
    f << "eco.recession_quarters=" << c.economy.recession_quarters << "\n";
    // Politics extra
    f << "pol.economic_ideology=" << pol.economic_ideology << "\n";
    f << "pol.social_ideology=" << pol.social_ideology << "\n";
    f << "pol.coup_d_etat_prob=" << pol.coup_d_etat_prob << "\n";
    f << "pol.coup_success_prob=" << pol.coup_success_prob << "\n";
    f << "pol.civilian_military_control=" << pol.civilian_military_control << "\n";
    f << "pol.coalition_cohesion=" << pol.coalition_cohesion << "\n";
    f << "pol.opposition_seats=" << pol.opposition_seats << "\n";
    f << "pol.judicial_independence=" << pol.judicial_independence << "\n";
    f << "pol.revolution_prob=" << pol.revolution_prob << "\n";
    f << "pol.revolution_active=" << (pol.revolution_active ? 1 : 0) << "\n";
    f << "pol.revolution_duration=" << pol.revolution_duration << "\n";
    f << "pol.impeachment_in_progress=" << (pol.impeachment_in_progress ? 1 : 0) << "\n";
    f << "pol.impeachment_turns=" << pol.impeachment_turns << "\n";
    f << "pol.apologize_cooldown_turns=" << pol.apologize_cooldown_turns << "\n";
    f << "pol.threaten_streak_count=" << pol.threaten_streak_count << "\n";
    // Security extra
    f << "sec.troop_morale=" << c.security.troop_morale << "\n";
    f << "sec.diplomatic_prestige=" << c.security.diplomatic_prestige << "\n";
    f << "sec.press_freedom=" << c.security.press_freedom << "\n";
    f << "sec.us_alignment=" << c.security.us_alignment << "\n";
    f << "sec.china_alignment=" << c.security.china_alignment << "\n";
    f << "sec.non_aligned_index=" << c.security.non_aligned_index << "\n";
    f << "sec.soft_power_index=" << c.security.soft_power_index << "\n";
    f << "sec.military_spending_gdp=" << c.security.military_spending_gdp << "\n";
    f << "sec.invasion_prob=" << c.security.invasion_prob << "\n";
    f << "sec.nuclear_attack_prob=" << c.security.nuclear_attack_prob << "\n";
    f << "sec.attack_detection_prob=" << c.security.attack_detection_prob << "\n";
    // Infra extra
    f << "infra.innovation_index=" << c.infra.innovation_index << "\n";
    f << "infra.fossil_fuel_dependency=" << c.infra.fossil_fuel_dependency << "\n";
    f << "infra.renewables_percentage=" << c.infra.renewables_percentage << "\n";
    f << "infra.internet_coverage=" << c.infra.internet_coverage << "\n";
    f << "infra.co2_emissions=" << c.infra.co2_emissions << "\n";
    return true;
}

bool load(Country& c, int& turnCount, double& popularitySum,
          EndCondition& endCondition, const std::string& path,
          std::string* achievements_line_out) {
    std::ifstream f(path);
    if (!f) return false;
    std::unordered_map<std::string, std::string> kv;
    std::string line;
    while (std::getline(f, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        kv[line.substr(0, eq)] = line.substr(eq + 1);
    }
    auto getD = [&](const std::string& k, double def) { auto it = kv.find(k); return it == kv.end() ? def : std::stod(it->second); };
    auto getI = [&](const std::string& k, int def) { auto it = kv.find(k); return it == kv.end() ? def : std::stoi(it->second); };
    auto getB = [&](const std::string& k, bool def) { auto it = kv.find(k); return it == kv.end() ? def : (it->second == "1"); };
    auto& pol = c.politics;
    auto& eco = c.economy;
    auto& sec = c.security;
    auto& wel = c.welfare;
    turnCount = getI("turnCount", turnCount);
    popularitySum = getD("popularitySum", popularitySum);
    endCondition = (EndCondition)getI("endCondition", 0);
    pol.popularity = getD("popularity", pol.popularity);
    eco.gdp = getD("gdp", eco.gdp);
    eco.inflation = getD("inflation", eco.inflation);
    eco.growth_rate = getD("growth_rate", eco.growth_rate);
    eco.in_recession = getB("in_recession", eco.in_recession);
    sec.war_active = getB("war_active", sec.war_active);
    sec.war_duration = getI("war_duration", sec.war_duration);
    pol.civil_war_active = getB("civil_war_active", pol.civil_war_active);
    wel.pandemic_active = getB("pandemic_active", wel.pandemic_active);
    wel.pandemic_severity = getD("pandemic_severity", wel.pandemic_severity);
    pol.congressional_pressure = getD("congressional_pressure", pol.congressional_pressure);
    pol.judicial_pressure = getD("judicial_pressure", pol.judicial_pressure);
    pol.military_pressure = getD("military_pressure", pol.military_pressure);
    pol.popular_pressure = getD("popular_pressure", pol.popular_pressure);
    pol.international_pressure = getD("international_pressure", pol.international_pressure);
    pol.active_scandals = getI("active_scandals", pol.active_scandals);
    pol.scandal_corruption_severity = getD("scandal_corruption", pol.scandal_corruption_severity);
    pol.scandal_sex_severity = getD("scandal_sex", pol.scandal_sex_severity);
    pol.scandal_financial_severity = getD("scandal_financial", pol.scandal_financial_severity);
    pol.scandal_violence_severity = getD("scandal_violence", pol.scandal_violence_severity);
    pol.scandal_substance_severity = getD("scandal_substance", pol.scandal_substance_severity);
    pol.regime_legitimacy = getD("regime_legitimacy", pol.regime_legitimacy);
    pol.democratic_backsliding_index = getD("democratic_backsliding_index", pol.democratic_backsliding_index);
    pol.authoritarian_actions_count = getI("authoritarian_actions_count", pol.authoritarian_actions_count);
    pol.auth_dem_axis = getD("auth_dem_axis", pol.auth_dem_axis);
    // Welfare extra
    c.welfare.population = getI("wel.population", c.welfare.population);
    c.welfare.literacy_rate = getD("wel.literacy_rate", c.welfare.literacy_rate);
    c.welfare.unemployment_rate = getD("wel.unemployment_rate", c.welfare.unemployment_rate);
    c.welfare.health_coverage = getD("wel.health_coverage", c.welfare.health_coverage);
    c.welfare.poverty_rate = getD("wel.poverty_rate", c.welfare.poverty_rate);
    c.welfare.union_strength = getD("wel.union_strength", c.welfare.union_strength);
    c.welfare.minority_protection = getD("wel.minority_protection", c.welfare.minority_protection);
    c.welfare.torture_index = getD("wel.torture_index", c.welfare.torture_index);
    c.welfare.forced_disappearances = getD("wel.forced_disappearances", c.welfare.forced_disappearances);
    c.welfare.un_score = getD("wel.un_score", c.welfare.un_score);
    c.welfare.radicalism_prob = getD("wel.radicalism_prob", c.welfare.radicalism_prob);
    c.welfare.freedom_of_expression = getD("wel.freedom_of_expression", c.welfare.freedom_of_expression);
    c.welfare.minimum_wage = getD("wel.minimum_wage", c.welfare.minimum_wage);
    c.welfare.retirement_age = getD("wel.retirement_age", c.welfare.retirement_age);
    // Economy extra
    c.economy.interest_rate = getD("eco.interest_rate", c.economy.interest_rate);
    c.economy.central_bank_autonomy = getD("eco.central_bank_autonomy", c.economy.central_bank_autonomy);
    c.economy.monetary_emission = getD("eco.monetary_emission", c.economy.monetary_emission);
    c.economy.debt_to_gdp_ratio = getD("eco.debt_to_gdp_ratio", c.economy.debt_to_gdp_ratio);
    int rating = getI("eco.credit_rating", (int)c.economy.credit_rating);
    if (rating >= 0 && rating <= (int)CreditRating::D) c.economy.credit_rating = (CreditRating)rating;
    c.economy.sovereign_wealth_fund = getD("eco.sovereign_wealth_fund", c.economy.sovereign_wealth_fund);
    c.economy.exchange_rate_stability = getD("eco.exchange_rate_stability", c.economy.exchange_rate_stability);
    c.economy.commodity_supercycle = getD("eco.commodity_supercycle", c.economy.commodity_supercycle);
    c.economy.commodity_prices = getD("eco.commodity_prices", c.economy.commodity_prices);
    c.economy.mining_concessions = getI("eco.mining_concessions", c.economy.mining_concessions);
    c.economy.average_tariffs = getD("eco.average_tariffs", c.economy.average_tariffs);
    c.economy.trade_openness = getD("eco.trade_openness", c.economy.trade_openness);
    c.economy.recession_quarters = getI("eco.recession_quarters", c.economy.recession_quarters);
    // Politics extra
    pol.economic_ideology = getD("pol.economic_ideology", pol.economic_ideology);
    pol.social_ideology = getD("pol.social_ideology", pol.social_ideology);
    pol.coup_d_etat_prob = getD("pol.coup_d_etat_prob", pol.coup_d_etat_prob);
    pol.coup_success_prob = getD("pol.coup_success_prob", pol.coup_success_prob);
    pol.civilian_military_control = getD("pol.civilian_military_control", pol.civilian_military_control);
    pol.coalition_cohesion = getD("pol.coalition_cohesion", pol.coalition_cohesion);
    pol.opposition_seats = getI("pol.opposition_seats", pol.opposition_seats);
    pol.judicial_independence = getD("pol.judicial_independence", pol.judicial_independence);
    pol.revolution_prob = getD("pol.revolution_prob", pol.revolution_prob);
    pol.revolution_active = getB("pol.revolution_active", pol.revolution_active);
    pol.revolution_duration = getI("pol.revolution_duration", pol.revolution_duration);
    pol.impeachment_in_progress = getB("pol.impeachment_in_progress", pol.impeachment_in_progress);
    pol.impeachment_turns = getI("pol.impeachment_turns", pol.impeachment_turns);
    pol.apologize_cooldown_turns = getI("pol.apologize_cooldown_turns", pol.apologize_cooldown_turns);
    pol.threaten_streak_count = getI("pol.threaten_streak_count", pol.threaten_streak_count);
    // Security extra
    c.security.troop_morale = getD("sec.troop_morale", c.security.troop_morale);
    c.security.diplomatic_prestige = getD("sec.diplomatic_prestige", c.security.diplomatic_prestige);
    c.security.press_freedom = getD("sec.press_freedom", c.security.press_freedom);
    c.security.us_alignment = getD("sec.us_alignment", c.security.us_alignment);
    c.security.china_alignment = getD("sec.china_alignment", c.security.china_alignment);
    c.security.non_aligned_index = getD("sec.non_aligned_index", c.security.non_aligned_index);
    c.security.soft_power_index = getD("sec.soft_power_index", c.security.soft_power_index);
    c.security.military_spending_gdp = getD("sec.military_spending_gdp", c.security.military_spending_gdp);
    c.security.invasion_prob = getD("sec.invasion_prob", c.security.invasion_prob);
    c.security.nuclear_attack_prob = getD("sec.nuclear_attack_prob", c.security.nuclear_attack_prob);
    c.security.attack_detection_prob = getD("sec.attack_detection_prob", c.security.attack_detection_prob);
    // Infra extra
    c.infra.innovation_index = getD("infra.innovation_index", c.infra.innovation_index);
    c.infra.fossil_fuel_dependency = getD("infra.fossil_fuel_dependency", c.infra.fossil_fuel_dependency);
    c.infra.renewables_percentage = getD("infra.renewables_percentage", c.infra.renewables_percentage);
    c.infra.internet_coverage = getD("infra.internet_coverage", c.infra.internet_coverage);
    c.infra.co2_emissions = getD("infra.co2_emissions", c.infra.co2_emissions);

    if (achievements_line_out) {
        auto it = kv.find("achievements");
        if (it != kv.end()) *achievements_line_out = it->second;
    }
    return true;
}

} // namespace Persistence
