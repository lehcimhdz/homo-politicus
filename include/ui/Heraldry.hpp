#ifndef HERALDRY_HPP
#define HERALDRY_HPP

#include <SFML/Graphics.hpp>
#include <string>

// Heraldry: escudo procedural derivado de un seed (hash de nombre o codigo).
// Forma (escudo/circular/triangular), 2 colores (campo / division) y simbolo central.
class Heraldry {
public:
    enum class Shape { Shield, Circle, Triangle };
    enum class Symbol { Eagle, Star, Sword, Sun };

    static void draw(sf::RenderWindow& win, float cx, float cy, float radius, unsigned seed);
    static unsigned seedFromString(const std::string& s);
};

#endif
