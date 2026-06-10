#ifndef ISO_VEHICLES_HPP
#define ISO_VEHICLES_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "ui/IsoCamera.hpp"
#include "ui/IsoBuilding.hpp"
#include "Country.hpp"

// IsoVehicles: carruajes/vehiculos recorriendo caminos entre edificios.
class IsoVehicles {
public:
    void configure(const IsoBuilding& buildings, const Country& c);
    void update(float dt);
    void drawRoads(sf::RenderWindow& win, const IsoCamera& cam) const;
    void drawVehicles(sf::RenderWindow& win, const IsoCamera& cam, float nightAmount) const;
    size_t vehicleCount() const { return vehicles_.size(); }

private:
    struct Road {
        sf::Vector2f a;
        sf::Vector2f b;
    };
    struct Vehicle {
        int roadIdx;
        float t;       // 0..1 a lo largo del road
        float speed;   // dir signed
        sf::Color color;
    };
    std::vector<Road> roads_;
    std::vector<Vehicle> vehicles_;
    unsigned rng_ = 0x1A2B3C4Du;
    float rand01();
};

#endif
