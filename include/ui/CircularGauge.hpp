#ifndef CIRCULAR_GAUGE_HPP
#define CIRCULAR_GAUGE_HPP

#include <SFML/Graphics.hpp>
#include <string>

// CircularGauge: dial circular tipo CK3 con animacion eased y gradient rojo->verde.
// El sector dibujado va desde la posicion 12-en-punto en sentido horario.
class CircularGauge {
public:
    void setTarget(float v);     // 0..1 (clamped)
    void setImmediate(float v);  // salta sin animacion
    float current() const { return displayValue_; }
    void update(float dt);

    void draw(sf::RenderWindow& win, const sf::Font& font,
              float cx, float cy, float rOuter, float rInner,
              const std::string& centerLabel, unsigned labelSize = 18) const;

    static sf::Color colorForValue(float v);

private:
    float target_ = 0.f;
    float displayValue_ = 0.f;
};

#endif
