#ifndef ISO_WORLD_HPP
#define ISO_WORLD_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "ui/IsoCamera.hpp"
#include "ui/CountrySilhouette.hpp"
#include "Country.hpp"

// IsoWorld: grid isometrico de tiles clipped a la silueta del pais.
// Cada tile representa una "region" con bioma y satisfaction propia.
class IsoWorld {
public:
    enum class Biome { Urban, Rural, Mountain, Coast };

    struct Tile {
        int gx, gy;            // grid coords
        Biome biome;
        float satisfaction;    // 0..1, hash-derivada de gx,gy
        sf::Vector2f screenCenter; // cached
        bool inside = false;
    };

    void configure(const CountrySilhouette& sil, const IsoCamera& cam,
                   sf::Vector2f homeScreenPos, float homeRadius,
                   int gridSize = 22);
    void draw(sf::RenderWindow& win, const IsoCamera& cam,
              const Country& country, float nightAmount) const;

    const std::vector<Tile>& tiles() const { return tiles_; }
    int gridSize() const { return gridSize_; }

private:
    std::vector<Tile> tiles_;
    int gridSize_ = 22;

    static sf::Color biomeColor(Biome b);
};

#endif
