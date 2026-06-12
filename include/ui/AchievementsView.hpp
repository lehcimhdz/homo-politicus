#ifndef ACHIEVEMENTS_VIEW_HPP
#define ACHIEVEMENTS_VIEW_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "AchievementTracker.hpp"

// AchievementsView: tab [5] con 40 logros en grid + estados, rareza, iconos,
// progreso parcial, filtros y tooltips.
class AchievementsView {
public:
    enum class Rarity  { Common, Uncommon, Rare, Legendary };
    enum class Category{ Politics, Economy, Crisis, Milestone };
    enum class Filter  { All, Unlocked, Locked };

    void configure(const AchievementTracker& tracker);
    void update(const AchievementTracker& tracker);
    void onMouseMove(sf::Vector2f mouse);
    void onClick(sf::Vector2f mouse);
    void setFilter(Filter f) { filter_ = f; }
    Filter filter() const { return filter_; }

    void draw(sf::RenderWindow& win, const sf::Font& font,
              float x, float y, float w, float h) const;

    int hovered() const { return hovered_; }
    int totalCount() const { return (int)entries_.size(); }
    int unlockedCount() const;

private:
    struct Entry {
        std::string id;
        std::string name;
        std::string description;
        Rarity rarity;
        Category category;
        bool unlocked;
        // Progreso parcial (current, target). 0/0 = no aplica.
        int progressCurrent = 0;
        int progressTarget = 0;
    };
    std::vector<Entry> entries_;
    int hovered_ = -1;
    Filter filter_ = Filter::All;
    sf::Vector2f mouse_{-1.f, -1.f};

    Rarity   rarityFor(const std::string& id) const;
    Category categoryFor(const std::string& id) const;
    static sf::Color rarityColor(Rarity r);
    static const char* rarityLabel(Rarity r);
    static void drawIcon(sf::RenderWindow& win, Category cat,
                          float cx, float cy, float size, sf::Color color);
};

#endif
