#ifndef SCENARIO_LOADER_HPP
#define SCENARIO_LOADER_HPP

#include <string>
#include "Country.hpp"

namespace ScenarioLoader {
    // Lee YAML de escenario y aplica el bloque initial_country sobre c.
    // Devuelve metadata{name, year, difficulty} si éxito.
    struct Metadata {
        std::string id;
        std::string name_es;
        std::string difficulty;
        int start_year = 0;
    };
    bool load(const std::string& path, Country& c, Metadata& meta);
}

#endif
