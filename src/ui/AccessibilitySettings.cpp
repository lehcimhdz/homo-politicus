#include "ui/AccessibilitySettings.hpp"

AccessibilitySettings::AccessibilitySettings() {}

sf::Color AccessibilitySettings::adjustColor(sf::Color o) const {
    switch (colorMode_) {
        case ColorMode::Normal:
            return o;
        case ColorMode::Protanopia: {
            // Sin rojo: rebalancear
            std::uint8_t r = (std::uint8_t)((o.g + o.b) / 2);
            return sf::Color(r, o.g, o.b, o.a);
        }
        case ColorMode::Deuteranopia: {
            // Sin verde
            std::uint8_t g = (std::uint8_t)((o.r + o.b) / 2);
            return sf::Color(o.r, g, o.b, o.a);
        }
        case ColorMode::Tritanopia: {
            // Sin azul
            std::uint8_t b = (std::uint8_t)((o.r + o.g) / 2);
            return sf::Color(o.r, o.g, b, o.a);
        }
        case ColorMode::HighContrast: {
            // Forzar negro o blanco
            int avg = (o.r + o.g + o.b) / 3;
            return avg > 127 ? sf::Color::White : sf::Color::Black;
        }
    }
    return o;
}

unsigned AccessibilitySettings::adjustFontSize(unsigned baseSize) const {
    return (unsigned)((float)baseSize * fontScale_);
}
