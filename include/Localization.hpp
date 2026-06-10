#ifndef LOCALIZATION_HPP
#define LOCALIZATION_HPP

#include <string>

namespace Localization {
    // Carga el archivo `<locales_dir>/<lang>.yaml` y lo deja como idioma activo.
    // Si falla, mantiene el idioma anterior.
    bool load(const std::string& locales_dir, const std::string& lang);

    // Traduce una clave dot-path. Si no existe, retorna `[missing: <key>]`.
    std::string tr(const std::string& key);

    // Idioma activo actual ("es" / "en" / "").
    const std::string& currentLanguage();
}

#endif
