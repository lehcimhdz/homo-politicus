#include "Persistence.hpp"
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace Persistence {

bool save(const Country& c, int turnCount, double popularitySum,
          EndCondition endCondition, const std::string& path) {
    std::ofstream f(path);
    if (!f) return false;
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
    return true;
}

bool load(Country& c, int& turnCount, double& popularitySum,
          EndCondition& endCondition, const std::string& path) {
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
    return true;
}

} // namespace Persistence
