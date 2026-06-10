#ifndef ISO_BUILDING_HPP
#define ISO_BUILDING_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "ui/IsoCamera.hpp"
#include "ui/CountrySilhouette.hpp"
#include "ui/IsoWorld.hpp"
#include "Country.hpp"

// IsoBuilding: prisma rectangular renderizado en isometric (3 caras visibles).
// Una capital + N ciudades secundarias colocadas en tiles urban dentro de la silueta.
class IsoBuilding {
public:
    enum class Kind { Capital, City, Tower };

    struct Building {
        float gx, gy;  // grid coords (continuos para placement fino)
        float w, d;    // ancho y profundidad en tiles
        float h;       // altura en "stacked" units
        Kind kind;
        sf::Color baseColor;
    };

    void configure(const IsoWorld& world, const CountrySilhouette& sil,
                   sf::Vector2f homePos, float homeRadius,
                   const Country& country);

    void draw(sf::RenderWindow& win, const IsoCamera& cam,
              const Country& country, float nightAmount) const;

    void updateForCountry(const Country& country);

    const std::vector<Building>& buildings() const { return buildings_; }

    // Devuelve building center proyectado a pantalla (para roads, NPCs).
    sf::Vector2f screenAt(size_t idx, const IsoCamera& cam) const;

private:
    std::vector<Building> buildings_;

    static void drawPrism(sf::RenderWindow& win, const IsoCamera& cam,
                          float gx, float gy, float w, float d, float h,
                          sf::Color base, float nightAmount);
};

#endif
