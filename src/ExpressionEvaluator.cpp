#include "ExpressionEvaluator.hpp"
#include <sstream>
#include <vector>
#include <algorithm>

namespace ExpressionEvaluator {

const std::unordered_map<std::string, std::function<double(const Country&)>>& fieldTable() {
    static std::unordered_map<std::string, std::function<double(const Country&)>> table = {
        {"country.economy.gdp",                 [](const Country& c){ return c.economy.gdp; }},
        {"country.economy.inflation",           [](const Country& c){ return c.economy.inflation; }},
        {"country.economy.growth_rate",         [](const Country& c){ return c.economy.growth_rate; }},
        {"country.economy.monetary_emission",   [](const Country& c){ return c.economy.monetary_emission; }},
        {"country.economy.debt_to_gdp_ratio",   [](const Country& c){ return c.economy.debt_to_gdp_ratio; }},
        {"country.economy.international_reserves", [](const Country& c){ return c.economy.international_reserves; }},
        {"country.economy.foreign_trade",       [](const Country& c){ return c.economy.trade_openness; }},
        {"country.economy.commodity_supercycle",[](const Country& c){ return c.economy.commodity_supercycle; }},
        {"country.economy.mining_concessions",  [](const Country& c){ return (double)c.economy.mining_concessions; }},
        {"country.economy.exchange_rate_stability", [](const Country& c){ return c.economy.exchange_rate_stability; }},
        {"country.economy.capital_flight_risk", [](const Country& c){ return c.infra.capital_flight_risk; }},
        {"country.economy.food_import_dependency", [](const Country& c){ return c.economy.food_import_dependency; }},

        {"country.politics.popularity",         [](const Country& c){ return c.politics.popularity; }},
        {"country.politics.coalition_cohesion", [](const Country& c){ return c.politics.coalition_cohesion; }},
        {"country.politics.regime_legitimacy",  [](const Country& c){ return c.politics.regime_legitimacy; }},
        {"country.politics.media_exposure_intensity", [](const Country& c){ return c.politics.media_exposure_intensity; }},
        {"country.politics.auth_dem_axis",      [](const Country& c){ return c.politics.auth_dem_axis; }},
        {"country.politics.judicial_independence", [](const Country& c){ return c.politics.judicial_independence; }},
        {"country.politics.polarization_index", [](const Country& c){ return c.politics.polarization_index; }},
        {"country.politics.administrative_corruption", [](const Country& c){ return c.politics.administrative_corruption; }},
        {"country.politics.federal_budget_disparity", [](const Country& c){ return c.politics.federal_budget_disparity; }},
        {"country.politics.regional_loyalty",   [](const Country& c){ return c.politics.regional_loyalty; }},
        {"country.politics.regional_gdp_gini",  [](const Country& c){ return c.politics.regional_gdp_gini; }},

        {"country.welfare.unemployment_rate",   [](const Country& c){ return c.welfare.unemployment_rate; }},
        {"country.welfare.union_strength",      [](const Country& c){ return c.welfare.union_strength; }},
        {"country.welfare.poverty_rate",        [](const Country& c){ return c.welfare.poverty_rate; }},
        {"country.welfare.torture_index",       [](const Country& c){ return c.welfare.torture_index; }},
        {"country.welfare.forced_disappearances", [](const Country& c){ return c.welfare.forced_disappearances; }},
        {"country.welfare.health_coverage",     [](const Country& c){ return c.welfare.health_coverage; }},

        {"country.security.attack_detection_prob", [](const Country& c){ return c.security.attack_detection_prob; }},
        {"country.security.terrorism_detection_prob", [](const Country& c){ return c.security.terrorism_detection_prob; }},
        {"country.security.criminal_organizations", [](const Country& c){ return (double)c.security.criminal_organizations; }},
        {"country.security.multilateral_leadership", [](const Country& c){ return c.security.multilateral_leadership; }},
        {"country.security.diplomatic_prestige", [](const Country& c){ return c.security.diplomatic_prestige; }},
        {"country.security.foreign_disinfo_operations", [](const Country& c){ return c.security.foreign_disinfo_operations; }},

        {"country.infra.startup_ecosystem_strength", [](const Country& c){ return c.infra.startup_ecosystem_strength; }},
        {"country.infra.climate_vulnerability_index", [](const Country& c){ return c.infra.climate_vulnerability_index; }},
        {"country.infra.storm_prob",            [](const Country& c){ return c.infra.storm_prob; }},
        {"country.infra.sanitation_coverage",   [](const Country& c){ return c.infra.sanitation_coverage; }},
        {"country.infra.drought_active",        [](const Country& c){ return c.infra.drought_active ? 1.0 : 0.0; }},
    };
    return table;
}

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t");
    return s.substr(a, b - a + 1);
}

static double resolveOperand(const std::string& token, const Country& c, bool& ok) {
    ok = true;
    std::string t = trim(token);
    auto it = fieldTable().find(t);
    if (it != fieldTable().end()) return it->second(c);
    try { return std::stod(t); } catch (...) {}
    if (t == "true") return 1.0;
    if (t == "false") return 0.0;
    ok = false;
    return 0.0;
}

static bool evalComparison(const std::string& expr, const Country& c) {
    static const std::vector<std::string> ops = {">=", "<=", "==", "!=", ">", "<"};
    for (const auto& op : ops) {
        auto pos = expr.find(op);
        if (pos == std::string::npos) continue;
        bool okA, okB;
        double a = resolveOperand(expr.substr(0, pos), c, okA);
        double b = resolveOperand(expr.substr(pos + op.size()), c, okB);
        if (!okA || !okB) return false;
        if (op == ">")  return a >  b;
        if (op == "<")  return a <  b;
        if (op == ">=") return a >= b;
        if (op == "<=") return a <= b;
        if (op == "==") return a == b;
        if (op == "!=") return a != b;
    }
    bool ok;
    double v = resolveOperand(expr, c, ok);
    return ok && v != 0.0;
}

static std::vector<std::string> splitByKeyword(const std::string& s, const std::string& kw) {
    std::vector<std::string> parts;
    size_t start = 0;
    while (true) {
        auto pos = s.find(kw, start);
        if (pos == std::string::npos) { parts.push_back(s.substr(start)); break; }
        bool wordBoundaryBefore = (pos == 0 || s[pos - 1] == ' ');
        bool wordBoundaryAfter = (pos + kw.size() >= s.size() || s[pos + kw.size()] == ' ');
        if (wordBoundaryBefore && wordBoundaryAfter) {
            parts.push_back(s.substr(start, pos - start));
            start = pos + kw.size();
        } else {
            start = pos + kw.size();
        }
    }
    return parts;
}

bool evaluate(const std::string& expression, const Country& c) {
    std::string expr = trim(expression);
    if (expr.empty()) return false;
    auto orParts = splitByKeyword(expr, "OR");
    for (const auto& orPart : orParts) {
        auto andParts = splitByKeyword(trim(orPart), "AND");
        bool allTrue = true;
        for (const auto& andPart : andParts) {
            if (!evalComparison(trim(andPart), c)) { allTrue = false; break; }
        }
        if (allTrue) return true;
    }
    return false;
}

} // namespace ExpressionEvaluator
