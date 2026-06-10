#ifndef MINI_YAML_HPP
#define MINI_YAML_HPP

#include <string>
#include <unordered_map>

// Mini-YAML para el subset que usamos en escenarios:
// - bloques key:\n  subkey: valor (indentación = 2 espacios por nivel)
// - valores escalares (number, string, true/false)
// - claves anidadas se aplanan a "section.subkey" -> "valor"
// - listas (-) NO soportadas; comentarios (#) y líneas vacías sí.
//
// Ejemplo:
//   welfare:
//     population: 1900000
//     literacy_rate: 0.50
// Produce:
//   "welfare.population" -> "1900000"
//   "welfare.literacy_rate" -> "0.50"

namespace MiniYaml {
    using KV = std::unordered_map<std::string, std::string>;
    bool loadFile(const std::string& path, KV& out);
}

#endif
