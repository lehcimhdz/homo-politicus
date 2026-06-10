#ifndef ACCESSIBILITY_SETTINGS_HPP
#define ACCESSIBILITY_SETTINGS_HPP

#include <SFML/Graphics.hpp>

// Accessibility: color blindness profiles, font scaling, high contrast.
class AccessibilitySettings {
public:
    enum class ColorMode {
        Normal,
        Protanopia,      // ciego al rojo
        Deuteranopia,    // ciego al verde
        Tritanopia,      // ciego al azul
        HighContrast
    };

    AccessibilitySettings();
    void setColorMode(ColorMode m) { colorMode_ = m; }
    ColorMode colorMode() const { return colorMode_; }

    void setFontScale(float s) { fontScale_ = s; }
    float fontScale() const { return fontScale_; }

    // Devuelve color ajustado según el modo activo.
    sf::Color adjustColor(sf::Color original) const;
    unsigned adjustFontSize(unsigned baseSize) const;

private:
    ColorMode colorMode_ = ColorMode::Normal;
    float fontScale_ = 1.0f;
};

#endif
