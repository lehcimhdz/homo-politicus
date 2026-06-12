#ifndef ASSET_MANAGER_HPP
#define ASSET_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

// AssetManager: cache simple de texturas con loader por path multi-candidato.
// Singleton perezoso.
class AssetManager {
public:
    static AssetManager& instance();

    // Intenta cargar texture desde primera ruta valida. Devuelve true si OK.
    bool loadTexture(const std::string& key, const std::vector<std::string>& paths);
    bool loadTexture(const std::string& key, const std::string& path);

    // Devuelve textura cacheada o nullptr.
    const sf::Texture* getTexture(const std::string& key) const;
    bool hasTexture(const std::string& key) const;

    // Pre-cargas comunes (portraits + backgrounds).
    void preloadDefaults();

private:
    AssetManager() = default;
    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textures_;
};

#endif
