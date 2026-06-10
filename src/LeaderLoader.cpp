#include "LeaderLoader.hpp"
#include "MiniYaml.hpp"

namespace LeaderLoader {

static double parseDelta(const std::string& v) {
    if (v.empty()) return 0.0;
    try {
        if (v[0] == '+') return std::stod(v.substr(1));
        return std::stod(v); // -0.10 funciona, 0.20 también
    } catch (...) { return 0.0; }
}

static void clamp01(double& v) { if (v < 0.0) v = 0.0; if (v > 1.0) v = 1.0; }

static void apply(const MiniYaml::KV& kv, const std::string& field, double& target, bool clamp = true) {
    auto it = kv.find(std::string("starting_modifiers.") + field);
    if (it == kv.end()) return;
    target += parseDelta(it->second);
    if (clamp) clamp01(target);
}

bool load(const std::string& path, Country& c, Metadata& meta) {
    MiniYaml::KV kv;
    if (!MiniYaml::loadFile(path, kv)) return false;
    auto getS = [&](const std::string& k, const std::string& def) {
        auto it = kv.find(k); return it == kv.end() ? def : it->second;
    };
    meta.id = getS("id", "unknown");
    meta.name_es = getS("name_es", "Lider sin nombre");
    meta.country_origin = getS("country_origin", "?");

    apply(kv, "popularity",                c.politics.popularity);
    apply(kv, "polarization_index",        c.politics.polarization_index);
    apply(kv, "auth_dem_axis",             c.politics.auth_dem_axis);
    apply(kv, "economic_ideology",         c.politics.economic_ideology);
    apply(kv, "social_ideology",           c.politics.social_ideology);
    apply(kv, "authoritarianism_prob",     c.politics.authoritarianism_prob);
    apply(kv, "congressional_support",     c.politics.congressional_support);
    apply(kv, "regime_legitimacy",         c.politics.regime_legitimacy);
    apply(kv, "judicial_independence",     c.politics.judicial_independence);
    apply(kv, "administrative_corruption", c.politics.administrative_corruption);
    apply(kv, "anticorruption_enforcement",c.politics.anticorruption_enforcement);
    apply(kv, "coalition_cohesion",        c.politics.coalition_cohesion);
    apply(kv, "democratic_backsliding_index", c.politics.democratic_backsliding_index);

    apply(kv, "press_freedom",             c.security.press_freedom);
    apply(kv, "diplomatic_prestige",       c.security.diplomatic_prestige);
    apply(kv, "soft_power_index",          c.security.soft_power_index);
    apply(kv, "us_alignment",              c.security.us_alignment);
    apply(kv, "china_alignment",           c.security.china_alignment);
    apply(kv, "non_aligned_index",         c.security.non_aligned_index);
    apply(kv, "troop_morale",              c.security.troop_morale);
    apply(kv, "international_pressure",    c.politics.international_pressure);
    apply(kv, "civilian_military_control", c.politics.civilian_military_control);
    apply(kv, "coup_d_etat_prob",          c.politics.coup_d_etat_prob);
    apply(kv, "invasion_prob",             c.security.invasion_prob);
    apply(kv, "torture_index",             c.welfare.torture_index);
    apply(kv, "forced_disappearances",     c.welfare.forced_disappearances);
    apply(kv, "minority_protection",       c.welfare.minority_protection);
    apply(kv, "freedom_of_expression",     c.welfare.freedom_of_expression);
    apply(kv, "union_strength",            c.welfare.union_strength);
    apply(kv, "central_bank_autonomy",     c.economy.central_bank_autonomy);
    return true;
}

} // namespace LeaderLoader
