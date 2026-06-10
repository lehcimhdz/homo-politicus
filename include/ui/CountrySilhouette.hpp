#ifndef COUNTRY_SILHOUETTE_HPP
#define COUNTRY_SILHOUETTE_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// CountrySilhouette: carga path SVG mínimo (comandos M, L, Z) y lo renderea como
// fill + outline. Bounding box del path se normaliza a un canvas dado.
class CountrySilhouette {
public:
    bool loadFromFile(const std::string& path);
    bool loaded() const { return !points_.empty(); }
    size_t pointCount() const { return points_.size(); }
    const std::vector<sf::Vector2f>& rawPoints() const { return points_; }
    void draw(sf::RenderWindow& win, float cx, float cy, float radius,
              sf::Color fill, sf::Color outline) const;
    sf::FloatRect screenBBox(float cx, float cy, float radius) const;
    bool containsScreen(sf::Vector2f p, float cx, float cy, float radius) const;

private:
    std::vector<sf::Vector2f> points_;
    sf::Vector2f bboxMin_{0.f, 0.f};
    sf::Vector2f bboxMax_{1.f, 1.f};

    void recomputeBBox();
};

#endif
