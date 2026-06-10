#ifndef COURT_NETWORK_HPP
#define COURT_NETWORK_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Country.hpp"

// CourtNetwork: diagrama de red politica.
// Lider en el centro, asesores leales cerca, opositores lejos.
// Aristas con grosor = relacion (positiva verde, negativa roja).
class CourtNetwork {
public:
    void configure(const Country& c);
    void update(float dt);
    void onMouseMove(sf::Vector2f mouse);
    void draw(sf::RenderWindow& win, const sf::Font& font,
              float x, float y, float w, float h) const;

    int hovered() const { return hovered_; }
    std::string hoveredDetail() const;

private:
    struct Node {
        std::string name;
        std::string role;
        sf::Vector2f basePos;   // posicion en area normalizada (0..1, 0..1)
        sf::Vector2f screenPos; // computada en draw
        float relation;          // -1..1 (negativo = opositor)
        sf::Color color;
        float radius;
    };
    std::vector<Node> nodes_;
    int hovered_ = -1;
    float t_ = 0.f;
};

#endif
