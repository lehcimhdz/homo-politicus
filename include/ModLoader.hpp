#ifndef MOD_LOADER_HPP
#define MOD_LOADER_HPP

#include <string>
#include <vector>

// ModLoader: detecta mods en `mods/<modname>/` y los carga.
// Cada mod tiene un `mod.yaml` en su raíz con metadata.
namespace ModLoader {

    struct ModMetadata {
        std::string id;
        std::string name;
        std::string author;
        std::string version;
        std::string description;
        std::vector<std::string> dependencies;
        bool enabled = true;
    };

    // Escanea un directorio (típicamente `~/.config/HomoPoliticus/mods/`).
    // Devuelve lista de mods detectados.
    std::vector<ModMetadata> scan(const std::string& modsDir);

    // Verifica que no haya conflictos de IDs.
    bool validateNoConflicts(const std::vector<ModMetadata>& mods);

    // Detecta dependencias faltantes.
    std::vector<std::string> missingDependencies(const std::vector<ModMetadata>& mods);
}

#endif
