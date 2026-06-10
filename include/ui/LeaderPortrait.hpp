#ifndef LEADER_PORTRAIT_HPP
#define LEADER_PORTRAIT_HPP

#include <SFML/Graphics.hpp>
#include <string>

// LeaderPortrait: retrato procedural minimalista (sin assets externos).
// Genera silueta de hombros + cabeza + pelo en colores derivados del nombre.
// Marco circular dorado al rededor.
class LeaderPortrait {
public:
    static void draw(sf::RenderWindow& win, const sf::Font& font,
                     const std::string& name, const std::string& title,
                     float cx, float cy, float radius);
    static void drawCompact(sf::RenderWindow& win, const sf::Font& font,
                            const std::string& name, float cx, float cy, float radius);

private:
    static unsigned hashName(const std::string& s);
};

#endif
