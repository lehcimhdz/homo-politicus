#include "GameMode.hpp"

namespace GameModeRegistry {

std::string name(GameMode m) {
    switch (m) {
        case GameMode::Sandbox:    return "Sandbox";
        case GameMode::Missions:   return "Misiones";
        case GameMode::Historical: return "Historico";
    }
    return "?";
}

std::string description(GameMode m) {
    switch (m) {
        case GameMode::Sandbox:    return "Juego libre sin objetivos forzados.";
        case GameMode::Missions:   return "Objetivos especificos con sistema de estrellas.";
        case GameMode::Historical: return "Recrea escenarios reales (Chile 1973, Cuba 1959, etc).";
    }
    return "";
}

std::vector<MissionObjective> objectivesFor(GameMode mode, const std::string& scenarioId) {
    std::vector<MissionObjective> out;
    if (mode == GameMode::Sandbox) return out;

    if (mode == GameMode::Missions) {
        out.push_back({"first_year",       "Sobrevive 12 turnos",
                       "turn >= 12", false});
        out.push_back({"economic_growth",  "GDP +20% del inicial",
                       "gdp_ratio >= 1.2", false});
        out.push_back({"no_authoritarian", "Sin acciones autoritarias",
                       "auth_actions == 0", false});
        return out;
    }

    if (mode == GameMode::Historical) {
        if (scenarioId == "chile_1973") {
            out.push_back({"survive_sept11", "Sobrevive el 11 de septiembre (turno 9)",
                           "turn >= 9 AND alive", false});
            out.push_back({"no_coup",        "Sin golpe militar exitoso",
                           "end != COUP_SUCCESS", false});
            out.push_back({"low_inflation",  "Inflacion bajo 100% al final",
                           "inflation < 1.0", false});
            return out;
        }
        if (scenarioId == "cuba_1959") {
            out.push_back({"survive_decade", "Sobrevive 40 turnos",
                           "turn >= 40", false});
            out.push_back({"no_invasion",    "Evita invasion exitosa",
                           "end != EXILE", false});
            out.push_back({"health_high",    "Cobertura salud > 0.8",
                           "health_coverage > 0.8", false});
            return out;
        }
        if (scenarioId == "argentina_1976") {
            out.push_back({"low_disappearances", "Forced disappearances < 100",
                           "forced_disappearances < 100", false});
            out.push_back({"survive_term",       "Sobrevive 12 turnos",
                           "turn >= 12", false});
            return out;
        }
        // Default histórico: completar mandato
        out.push_back({"term_completed", "Termina el mandato (turno 16)",
                       "turn >= 16", false});
    }
    return out;
}

}
