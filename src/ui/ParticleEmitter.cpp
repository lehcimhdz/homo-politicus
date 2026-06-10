#include "ui/ParticleEmitter.hpp"
#include <algorithm>
#include <cmath>

float ParticleEmitter::rand01() {
    rng_ = rng_ * 1664525u + 1013904223u;
    return (float)(rng_ & 0xFFFFFF) / (float)0xFFFFFF;
}

float ParticleEmitter::randRange(float lo, float hi) {
    return lo + rand01() * (hi - lo);
}

void ParticleEmitter::emit(Preset preset, float x, float y, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = {x, y};
        switch (preset) {
            case Preset::BoomGold: {
                p.vel = {randRange(-160.f, 160.f), randRange(-440.f, -240.f)};
                p.color = sf::Color(240, 200, 60, 255);
                p.lifeMax = randRange(1.2f, 1.7f);
                p.size = randRange(6.f, 10.f);
                p.gravity = 480.f;
                p.spin = randRange(-6.f, 6.f);
                p.square = false;
                break;
            }
            case Preset::RedSpark: {
                p.vel = {randRange(-240.f, 240.f), randRange(-440.f, -120.f)};
                p.color = sf::Color(230, 80, 60, 255);
                p.lifeMax = randRange(0.5f, 0.9f);
                p.size = randRange(2.f, 5.f);
                p.gravity = 320.f;
                p.spin = 0.f;
                p.square = false;
                break;
            }
            case Preset::GraySmoke: {
                p.vel = {randRange(-40.f, 40.f), randRange(-110.f, -50.f)};
                uint8_t g = (uint8_t)randRange(110.f, 170.f);
                p.color = sf::Color(g, g, g, 180);
                p.lifeMax = randRange(1.6f, 2.4f);
                p.size = randRange(12.f, 22.f);
                p.gravity = -30.f;
                p.spin = randRange(-1.f, 1.f);
                p.square = false;
                break;
            }
            case Preset::Confetti: {
                p.vel = {randRange(-240.f, 240.f), randRange(-440.f, -200.f)};
                float h = rand01();
                if (h < 0.25f)      p.color = sf::Color(220, 80, 80);
                else if (h < 0.5f)  p.color = sf::Color(80, 200, 120);
                else if (h < 0.75f) p.color = sf::Color(80, 160, 240);
                else                p.color = sf::Color(240, 200, 60);
                p.lifeMax = randRange(2.0f, 3.0f);
                p.size = randRange(4.f, 7.f);
                p.gravity = 180.f;
                p.spin = randRange(-8.f, 8.f);
                p.square = true;
                break;
            }
        }
        p.life = p.lifeMax;
        particles_.push_back(p);
    }
}

void ParticleEmitter::update(float dt) {
    for (auto& p : particles_) {
        p.life -= dt;
        p.vel.y += p.gravity * dt;
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.angle += p.spin * dt;
    }
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
            [](const Particle& p) { return p.life <= 0.f; }),
        particles_.end());
}

void ParticleEmitter::draw(sf::RenderWindow& win) const {
    for (const auto& p : particles_) {
        float alpha = (p.life / p.lifeMax);
        if (alpha < 0.f) alpha = 0.f; if (alpha > 1.f) alpha = 1.f;
        sf::Color c = p.color;
        c.a = (uint8_t)(c.a * alpha);
        if (p.square) {
            sf::RectangleShape r({p.size, p.size});
            r.setOrigin({p.size / 2.f, p.size / 2.f});
            r.setPosition(p.pos);
            r.setRotation(sf::radians(p.angle));
            r.setFillColor(c);
            win.draw(r);
        } else {
            sf::CircleShape circ(p.size / 2.f);
            circ.setOrigin({p.size / 2.f, p.size / 2.f});
            circ.setPosition(p.pos);
            circ.setFillColor(c);
            win.draw(circ);
        }
    }
}
