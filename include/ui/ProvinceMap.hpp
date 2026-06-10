#ifndef PROVINCE_MAP_HPP
#define PROVINCE_MAP_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "ui/IsoCamera.hpp"
#include "ui/IsoWorld.hpp"
#include "Country.hpp"

// ProvinceMap: subdivide la silueta del pais (via tiles del IsoWorld) en
// 8-12 provincias usando asignacion al seed mas cercano (Voronoi discreto).
// Cada provincia tiene nombre, satisfaccion y color tinte.
class ProvinceMap {
public:
    struct Province {
        std::string name;
        int seedTileIdx;   // tile que es seed
        float satisfaction;
        sf::Color tint;
    };

    void configure(const IsoWorld& world, const Country& c, int numProvinces = 9);
    void update(float dt, const Country& c);
    void onMouseMove(sf::Vector2f mouse, const IsoCamera& cam);

    void draw(sf::RenderWindow& win, const sf::Font& font,
              const IsoCamera& cam, const IsoWorld& world) const;

    int hovered() const { return hovered_; }
    std::string hoveredTooltip() const;
    size_t provinceCount() const { return provinces_.size(); }

private:
    std::vector<Province> provinces_;
    std::vector<int> tileProvince_;  // indice de provincia por tile (mismo orden que world.tiles())
    int hovered_ = -1;
    sf::Vector2f hoverMouse_{-1.f, -1.f};
    float t_ = 0.f;
};

#endif
