#ifndef DASHBOARD_HPP
#define DASHBOARD_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "Country.hpp"
#include "ui/CircularGauge.hpp"

// Dashboard: grid 2x3 de cards mostrando metricas clave.
// Area destinada: x=200..1030 (830 px), y=60..700 (640 px).
// Cada card: ~390 x 200 con 16 px de gap.
class Dashboard {
public:
    Dashboard();
    void recordHistory(const Country& c);    // Llamar cada tick
    void onMouseMove(sf::Vector2f mouse);     // Actualiza card hovered
    void update(float dt, const Country& c);  // Anima gauges
    int hoveredCard() const { return hoveredCard_; }
    std::string hoveredDetail(const Country& c) const;
    void draw(sf::RenderWindow& win, const sf::Font& font, const Country& c) const;

private:
    static constexpr size_t kHistMax = 60;
    std::vector<double> popHist;
    std::vector<double> gdpHist;
    std::vector<double> inflHist;
    int hoveredCard_ = -1;
    CircularGauge popGauge_;

    void drawCard(sf::RenderWindow& win, const sf::Font& font,
                  float x, float y, float w, float h,
                  const std::string& title) const;

    void drawPopularityCard (sf::RenderWindow& win, const sf::Font& font, float x, float y, const Country& c) const;
    void drawEconomyCard    (sf::RenderWindow& win, const sf::Font& font, float x, float y, const Country& c) const;
    void drawPressuresCard  (sf::RenderWindow& win, const sf::Font& font, float x, float y, const Country& c) const;
    void drawScandalsCard   (sf::RenderWindow& win, const sf::Font& font, float x, float y, const Country& c) const;
    void drawSystemsCard    (sf::RenderWindow& win, const sf::Font& font, float x, float y, const Country& c) const;
    void drawNeighborsCard  (sf::RenderWindow& win, const sf::Font& font, float x, float y, const Country& c) const;
};

#endif
