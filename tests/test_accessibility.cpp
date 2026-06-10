#include "TestFramework.hpp"
#include "ui/AccessibilitySettings.hpp"

TEST_CASE(accessibility_default_normal_mode) {
    AccessibilitySettings a;
    CHECK(a.colorMode() == AccessibilitySettings::ColorMode::Normal);
}

TEST_CASE(accessibility_default_font_scale_is_1) {
    AccessibilitySettings a;
    CHECK_NEAR(a.fontScale(), 1.0f, 0.001);
}

TEST_CASE(accessibility_font_size_scales_correctly) {
    AccessibilitySettings a;
    a.setFontScale(1.5f);
    CHECK_EQ(a.adjustFontSize(16), (unsigned)24);
}

TEST_CASE(accessibility_high_contrast_returns_black_or_white) {
    AccessibilitySettings a;
    a.setColorMode(AccessibilitySettings::ColorMode::HighContrast);
    sf::Color dark = a.adjustColor(sf::Color(40, 40, 40));
    sf::Color light = a.adjustColor(sf::Color(220, 220, 220));
    CHECK((dark.r == 0 && dark.g == 0 && dark.b == 0));
    CHECK((light.r == 255 && light.g == 255 && light.b == 255));
}

TEST_CASE(accessibility_normal_returns_original) {
    AccessibilitySettings a;
    sf::Color orig(120, 80, 200);
    sf::Color adjusted = a.adjustColor(orig);
    CHECK(adjusted.r == orig.r);
    CHECK(adjusted.g == orig.g);
    CHECK(adjusted.b == orig.b);
}
