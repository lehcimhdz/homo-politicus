#ifndef MINI_YAML_HPP
#define MINI_YAML_HPP

#include <string>
#include <unordered_map>
#include <vector>

// Mini-YAML para el subset que usamos en escenarios y eventos:
// - bloques key:\n  subkey: valor (indentación = 2 espacios por nivel)
// - valores escalares (number, string, true/false)
// - claves anidadas se aplanan a "section.subkey" -> "valor"
// - listas con "-" indentadas; cada item puede ser scalar o bloque map
// - comentarios (#) y líneas vacías ignorados
//
// Ejemplo lista de mapas:
//   events:
//     - id: foo
//       name_es: Foo
//     - id: bar
//       name_es: Bar
// Produce en items: 2 KVs separados, cada uno con sus claves planas.

namespace MiniYaml {
    using KV = std::unordered_map<std::string, std::string>;
    bool loadFile(const std::string& path, KV& out);

    // Lee un documento con `<rootKey>:` seguido de una lista de items, cada uno
    // un mapa. Retorna un vector con un KV por item (claves planas dentro del item).
    bool loadList(const std::string& path, const std::string& rootKey,
                  std::vector<KV>& items);
}

#endif
