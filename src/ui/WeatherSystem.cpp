#include "ui/WeatherSystem.hpp"
#include <cmath>
#include <algorithm>

float WeatherSystem::rand01() {
    rng_ = rng_ * 1664525u + 1013904223u;
    return (float)(rng_ & 0xFFFFFF) / (float)0xFFFFFF;
}

WeatherSystem::State WeatherSystem::stateForTurn(int t) const {
    // Ciclo deterministic de 16 turnos.
    int phase = ((t % 16) + 16) % 16;
    if (phase < 5)       return State::Sunny;
    else if (phase < 9)  return State::Cloudy;
    else if (phase < 13) return State::Rain;
    else                 return State::Snow;
}

void WeatherSystem::setTurn(int t) {
    turn_ = t;
    state_ = stateForTurn(t);
}

const char* WeatherSystem::label() const {
    switch (state_) {
        case State::Sunny:  return "SOLEADO";
        case State::Cloudy: return "NUBLADO";
        case State::Rain:   return "LLUVIA";
        case State::Snow:   return "NIEVE";
    }
    return "";
}

sf::Color WeatherSystem::globalTint() const {
    switch (state_) {
        case State::Sunny:  return sf::Color(255, 230, 180, 0);
        case State::Cloudy: return sf::Color(180, 190, 200, 30);
        case State::Rain:   return sf::Color(80, 100, 140, 40);
        case State::Snow:   return sf::Color(220, 230, 245, 50);
    }
    return sf::Color::Transparent;
}

void WeatherSystem::update(float dt, sf::FloatRect area) {
    t_ += dt;
    // Generar particulas para Rain/Snow.
    if (state_ == State::Rain || state_ == State::Snow) {
        int spawnPerSec = (state_ == State::Rain) ? 200 : 70;
        float toSpawn = spawnPerSec * dt;
        while (toSpawn > 0.f) {
            if (toSpawn < 1.f && rand01() > toSpawn) break;
            toSpawn -= 1.f;
            Particle p;
            p.pos = { area.position.x + rand01() * area.size.x,
                      area.position.y - 5.f };
            if (state_ == State::Rain) {
                p.vel = { -40.f + rand01() * 20.f, 380.f + rand01() * 120.f };
                p.isFlake = false;
                p.life = (area.size.y + 20.f) / p.vel.y;
            } else {
                p.vel = { -20.f + rand01() * 40.f, 60.f + rand01() * 40.f };
                p.isFlake = true;
                p.life = (area.size.y + 20.f) / p.vel.y;
            }
            particles_.push_back(p);
        }
    }
    // Actualizar y eliminar.
    for (auto& p : particles_) {
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        if (p.isFlake) {
            p.pos.x += std::sin(t_ * 3.f + (float)(long long)&p * 0.0001f) * dt * 8.f;
        }
        p.life -= dt;
    }
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
            [&](const Particle& p) { return p.life <= 0.f || p.pos.y > area.position.y + area.size.y; }),
        particles_.end());
    if (particles_.size() > 1000) particles_.resize(1000);
}

void WeatherSystem::draw(sf::RenderWindow& win, sf::FloatRect area) const {
    // Tinte global (rect que cubre el area, semi-transparente).
    sf::Color tint = globalTint();
    if (tint.a > 0) {
        sf::RectangleShape veil({area.size.x, area.size.y});
        veil.setPosition(area.position);
        veil.setFillColor(tint);
        win.draw(veil);
    }
    // Particulas.
    for (const auto& p : particles_) {
        if (p.isFlake) {
            sf::CircleShape flake(2.f);
            flake.setOrigin({2.f, 2.f});
            flake.setPosition(p.pos);
            flake.setFillColor(sf::Color(245, 250, 255, 220));
            win.draw(flake);
        } else {
            sf::RectangleShape drop({1.5f, 8.f});
            drop.setPosition(p.pos);
            drop.setRotation(sf::degrees(8.f));
            drop.setFillColor(sf::Color(160, 200, 240, 180));
            win.draw(drop);
        }
    }
}
