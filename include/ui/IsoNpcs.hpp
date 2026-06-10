#ifndef ISO_NPCS_HPP
#define ISO_NPCS_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "ui/IsoCamera.hpp"
#include "ui/IsoBuilding.hpp"
#include "Country.hpp"

// IsoNpcs: agentes caminando entre edificios.
// Cada uno tiene posicion en grid coords, target = building center,
// se mueve linealmente hacia el target; al llegar elige nuevo target.
class IsoNpcs {
public:
    void configure(const IsoBuilding& buildings, const Country& c);
    void update(float dt);
    void draw(sf::RenderWindow& win, const IsoCamera& cam, float nightAmount) const;
    size_t count() const { return npcs_.size(); }

private:
    struct Npc {
        sf::Vector2f pos;
        sf::Vector2f target;
        float speed;
        sf::Color color;
    };
    std::vector<Npc> npcs_;
    std::vector<sf::Vector2f> waypoints_;
    unsigned rng_ = 0xFEEDFACEu;

    float rand01();
    void pickTarget(Npc& n);
};

#endif
