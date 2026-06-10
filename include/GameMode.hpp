#ifndef GAME_MODE_HPP
#define GAME_MODE_HPP

#include <string>
#include <vector>

enum class GameMode {
    Sandbox,        // Sin objetivos, juega libre
    Missions,       // Objetivos especificos por dificultad
    Historical      // Escenarios reales con eventos forzados
};

struct ModeConfig {
    GameMode mode = GameMode::Sandbox;
    std::string scenarioId;        // solo Historical
    std::string difficulty;        // "estable", "fragil", "en_crisis", "estado_fallido"
    bool extremeEventsEnabled = true;
    int targetTurns = 0;           // 0 = sin limite
};

struct MissionObjective {
    std::string id;
    std::string description_es;
    std::string condition;         // expresion estilo "country.economy.gdp > 1e9 AT turn 24"
    bool achieved = false;
};

namespace GameModeRegistry {
    std::vector<MissionObjective> objectivesFor(GameMode mode, const std::string& scenarioId);
    std::string name(GameMode m);
    std::string description(GameMode m);
}

#endif
