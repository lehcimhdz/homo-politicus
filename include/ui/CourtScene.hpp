#ifndef COURT_SCENE_HPP
#define COURT_SCENE_HPP

#include <SFML/Graphics.hpp>
#include "Country.hpp"

// CourtScene: vista del palacio con lider en podio + 3 asesores parados.
// Idle breathing y opcional burbuja de dialogo si hay decision activa.
class CourtScene {
public:
    void update(float dt);
    void setDialog(const std::string& text) { dialog_ = text; }
    void clearDialog() { dialog_.clear(); }
    void draw(sf::RenderWindow& win, const sf::Font& font,
              float x, float y, float w, float h, const Country& c,
              int gameSeed = 0) const;

private:
    float t_ = 0.f;
    std::string dialog_;
};

#endif
