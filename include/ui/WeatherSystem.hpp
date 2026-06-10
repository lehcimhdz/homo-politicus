#ifndef WEATHER_SYSTEM_HPP
#define WEATHER_SYSTEM_HPP

#include <SFML/Graphics.hpp>
#include <vector>

// WeatherSystem: clima deterministic por turno (sunny / cloudy / rain / snow)
// con particulas y tinte global aplicado a un rect dado.
class WeatherSystem {
public:
    enum class State { Sunny, Cloudy, Rain, Snow };

    void setTurn(int t);
    State state() const { return state_; }
    const char* label() const;

    void update(float dt, sf::FloatRect area);
    void draw(sf::RenderWindow& win, sf::FloatRect area) const;
    // Tinte global sutil para aplicar como overlay sobre la pantalla.
    sf::Color globalTint() const;

private:
    State state_ = State::Sunny;
    int turn_ = 0;
    float t_ = 0.f;

    struct Particle {
        sf::Vector2f pos;
        sf::Vector2f vel;
        float life;
        bool isFlake;
    };
    std::vector<Particle> particles_;
    unsigned rng_ = 0xCAFEFEEDu;
    float rand01();
    State stateForTurn(int t) const;
};

#endif
