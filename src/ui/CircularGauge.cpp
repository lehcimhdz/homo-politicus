#include "ui/CircularGauge.hpp"
#include <cmath>
#include <algorithm>

void CircularGauge::setTarget(float v) {
    target_ = std::clamp(v, 0.f, 1.f);
}

void CircularGauge::setImmediate(float v) {
    target_ = std::clamp(v, 0.f, 1.f);
    displayValue_ = target_;
}

void CircularGauge::update(float dt) {
    // Ease: alcanza ~95% en ~0.6s (rate=5/s del delta).
    float k = std::min(1.f, dt * 5.f);
    displayValue_ += (target_ - displayValue_) * k;
}

sf::Color CircularGauge::colorForValue(float v) {
    v = std::clamp(v, 0.f, 1.f);
    if (v < 0.5f) {
        // rojo (220,80,80) -> amarillo (240,180,60)
        float t = v / 0.5f;
        return sf::Color(
            (uint8_t)(220 + (240 - 220) * t),
            (uint8_t)( 80 + (180 -  80) * t),
            (uint8_t)( 80 + ( 60 -  80) * t)
        );
    }
    // amarillo (240,180,60) -> verde (80,200,120)
    float t = (v - 0.5f) / 0.5f;
    return sf::Color(
        (uint8_t)(240 + ( 80 - 240) * t),
        (uint8_t)(180 + (200 - 180) * t),
        (uint8_t)( 60 + (120 -  60) * t)
    );
}

static constexpr float kPi = 3.14159265358979f;

void CircularGauge::draw(sf::RenderWindow& win, const sf::Font& font,
                         float cx, float cy, float rOuter, float rInner,
                         const std::string& centerLabel, unsigned labelSize) const {
    const int kSegments = 96;
    // Anillo de fondo (full circle, gris).
    {
        sf::VertexArray bg(sf::PrimitiveType::TriangleStrip);
        for (int i = 0; i <= kSegments; ++i) {
            float a = -kPi / 2.f + (2.f * kPi) * (float)i / (float)kSegments;
            float ca = std::cos(a), sa = std::sin(a);
            bg.append(sf::Vertex{{cx + ca * rOuter, cy + sa * rOuter}, sf::Color(48, 52, 66), {}});
            bg.append(sf::Vertex{{cx + ca * rInner, cy + sa * rInner}, sf::Color(48, 52, 66), {}});
        }
        win.draw(bg);
    }
    // Arco de llenado (color por valor).
    if (displayValue_ > 0.0001f) {
        sf::Color c = colorForValue(displayValue_);
        sf::VertexArray fill(sf::PrimitiveType::TriangleStrip);
        int segs = std::max(1, (int)std::round(kSegments * displayValue_));
        for (int i = 0; i <= segs; ++i) {
            float frac = (float)i / (float)kSegments;
            if (frac > displayValue_) frac = displayValue_;
            float a = -kPi / 2.f + (2.f * kPi) * frac;
            float ca = std::cos(a), sa = std::sin(a);
            fill.append(sf::Vertex{{cx + ca * rOuter, cy + sa * rOuter}, c, {}});
            fill.append(sf::Vertex{{cx + ca * rInner, cy + sa * rInner}, c, {}});
        }
        win.draw(fill);
    }
    // Etiqueta central.
    if (!centerLabel.empty()) {
        sf::Text t(font, centerLabel, labelSize);
        t.setFillColor(sf::Color(225, 228, 240));
        auto lb = t.getLocalBounds();
        t.setOrigin({lb.position.x + lb.size.x / 2.f, lb.position.y + lb.size.y / 2.f});
        t.setPosition({cx, cy});
        win.draw(t);
    }
}
