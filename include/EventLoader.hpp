#ifndef EVENT_LOADER_HPP
#define EVENT_LOADER_HPP

#include <string>
#include <vector>
#include <random>
#include "Country.hpp"
#include "DecisionSystem.hpp"

struct ScriptedEvent {
    std::string id;
    std::string name_es;
    std::string trigger_expr;
    double prob_per_turn = 0.05;
    int cooldown_turns = 0;
    int last_fired_turn = -1000;
    std::string flavor_text_es;
    // Efectos en orden, aplicados al disparar
    // Cada par: <path, op-value> donde op puede ser:
    //   "*1.5" multiplica, "+0.1" suma, "true"/"false" set bool
    std::vector<std::pair<std::string, std::string>> effects;
    // Decisión opcional encolada al disparo
    PendingDecision branch;
    bool has_branch = false;
};

namespace EventLoader {
    // Catálogo hardcoded (5 eventos seleccionados de los 30)
    const std::vector<ScriptedEvent>& builtin();
    // Aplica todos los eventos posibles este turno
    void tick(std::vector<ScriptedEvent>& events, Country& c,
              std::vector<PendingDecision>& queue, std::mt19937& rng, int currentTurn);

    // Carga un archivo YAML con lista `events:` y devuelve los eventos parseados.
    std::vector<ScriptedEvent> loadFile(const std::string& path);

    // Carga todos los archivos *.yaml de un directorio.
    std::vector<ScriptedEvent> loadDir(const std::string& dirPath);
}

#endif
