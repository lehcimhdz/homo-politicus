#ifndef PARTICLE_EMITTER_HPP
#define PARTICLE_EMITTER_HPP

#include <SFML/Graphics.hpp>
#include <vector>

// ParticleEmitter: efectos visuales para eventos criticos.
// Cuatro presets disponibles - cada uno con fisica propia (gravity, vel range, life).
class ParticleEmitter {
public:
    enum class Preset { BoomGold, RedSpark, GraySmoke, Confetti };

    void emit(Preset p, float x, float y, int count = 30);
    void update(float dt);
    void draw(sf::RenderWindow& win) const;
    void clear() { particles_.clear(); }
    size_t count() const { return particles_.size(); }

private:
    struct Particle {
        sf::Vector2f pos;
        sf::Vector2f vel;
        sf::Color color;
        float life = 0.f;
        float lifeMax = 1.f;
        float size = 4.f;
        float gravity = 200.f;
        float angle = 0.f;
        float spin = 0.f;
        bool square = false;
    };
    std::vector<Particle> particles_;
    unsigned rng_ = 0xC0FFEEu;

    float rand01();
    float randRange(float lo, float hi);
};

#endif
