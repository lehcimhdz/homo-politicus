#ifndef MANDATE_TIMELINE_HPP
#define MANDATE_TIMELINE_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// MandateTimeline: timeline horizontal del mandato.
// Cada turno aparece como un punto; eventos clave (elecciones, decisiones,
// escandalos) se marcan con label y color.
class MandateTimeline {
public:
    struct Event {
        int turn;
        std::string label;
        sf::Color color;
    };

    void addEvent(int turn, const std::string& label, sf::Color color);
    void clear() { events_.clear(); }
    size_t eventCount() const { return events_.size(); }

    void draw(sf::RenderWindow& win, const sf::Font& font,
              float x, float y, float w, float h,
              int currentTurn, int totalTurns = 60) const;

private:
    std::vector<Event> events_;
};

#endif
