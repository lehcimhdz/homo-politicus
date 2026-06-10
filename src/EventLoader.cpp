#include "EventLoader.hpp"
#include "ExpressionEvaluator.hpp"
#include <iostream>

namespace EventLoader {

const std::vector<ScriptedEvent>& builtin() {
    static std::vector<ScriptedEvent> events = {
        {
            "commodity_boom",
            "Boom de commodities",
            "country.economy.commodity_supercycle > 0.2 AND country.economy.mining_concessions > 5",
            0.10, 8, -1000,
            "Los precios internacionales del cobre, hierro y litio se han disparado. Las cotizaciones mineras llenan tu tesoreria sin esfuerzo.",
            { {"economy.state_royalties", "*1.6"}, {"economy.gdp", "*1.04"} },
            {"commodity_boom_decision", "Tenes royalty extra. Que haces?", {"save_to_swf","spend_on_social","industrial_policy"}},
            true
        },
        {
            "hyperinflation",
            "Espiral hiperinflacionaria",
            "country.economy.inflation > 0.5 AND country.economy.monetary_emission > 0.3",
            0.05, 20, -1000,
            "Los precios cambian dos veces por semana. Los billetes se vuelven inutiles antes de gastarse.",
            { {"economy.inflation", "+2.0"}, {"economy.growth_rate", "-0.08"}, {"politics.popularity", "-0.30"} },
            {},
            false
        },
        {
            "urban_terrorist_attack",
            "Atentado terrorista urbano",
            "country.security.attack_detection_prob < 0.5",
            0.03, 20, -1000,
            "Bomba en zona comercial. 47 muertos. El pais esta en shock.",
            { {"politics.popularity", "-0.10"} },
            {"terrorist_response_decision", "Como respondes al atentado?", {"emergency_powers","investigate_thorough","scapegoat_minority"}},
            true
        },
        {
            "severe_drought",
            "Sequia severa multianual",
            "country.infra.climate_vulnerability_index > 0.4",
            0.08, 15, -1000,
            "Tres anos sin lluvias normales. Cosechas perdidas, ganaderia diezmada.",
            { {"infra.drought_active", "true"}, {"economy.gdp", "*0.96"} },
            {},
            false
        },
        {
            "global_crash",
            "Crash bursatil global",
            "country.economy.foreign_trade > 0.5",
            0.02, 30, -1000,
            "Wall Street, Frankfurt y Shanghai se desploman simultaneamente. Tu moneda se devalua.",
            { {"economy.gdp", "*0.94"}, {"economy.growth_rate", "-0.05"}, {"economy.international_reserves", "*0.85"} },
            {"global_crash_decision", "Crisis global. Tu respuesta?", {"emergency_stimulus","austerity","capital_controls"}},
            true
        },
    };
    return events;
}

static void applyEffect(Country& c, const std::string& path, const std::string& op_value) {
    auto applyOp = [](double& target, const std::string& op_value) {
        if (op_value.empty()) return;
        if (op_value[0] == '*') { try { target *= std::stod(op_value.substr(1)); } catch(...){} }
        else if (op_value[0] == '+') { try { target += std::stod(op_value.substr(1)); } catch(...){} }
        else if (op_value[0] == '-') { try { target -= std::stod(op_value.substr(1)); } catch(...){} }
    };
    if (path == "economy.gdp") applyOp(c.economy.gdp, op_value);
    else if (path == "economy.inflation") applyOp(c.economy.inflation, op_value);
    else if (path == "economy.growth_rate") applyOp(c.economy.growth_rate, op_value);
    else if (path == "economy.state_royalties") applyOp(c.economy.state_royalties, op_value);
    else if (path == "economy.international_reserves") applyOp(c.economy.international_reserves, op_value);
    else if (path == "politics.popularity") applyOp(c.politics.popularity, op_value);
    else if (path == "infra.drought_active") c.infra.drought_active = (op_value == "true");
}

void tick(std::vector<ScriptedEvent>& events, Country& c,
          std::vector<PendingDecision>& queue, std::mt19937& rng, int currentTurn) {
    std::uniform_real_distribution<double> roll(0.0, 1.0);
    for (auto& e : events) {
        if (currentTurn - e.last_fired_turn < e.cooldown_turns) continue;
        if (!ExpressionEvaluator::evaluate(e.trigger_expr, c)) continue;
        if (roll(rng) >= e.prob_per_turn) continue;
        std::cout << "\n[EVENTO] " << e.name_es << std::endl;
        std::cout << "         " << e.flavor_text_es << std::endl;
        for (const auto& [path, val] : e.effects) applyEffect(c, path, val);
        if (e.has_branch) queue.push_back(e.branch);
        e.last_fired_turn = currentTurn;
    }
}

} // namespace EventLoader
