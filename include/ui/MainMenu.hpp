#ifndef MAIN_MENU_HPP
#define MAIN_MENU_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

// MainMenu: pantalla principal previa al juego.
class MainMenu {
public:
    enum class Action { None, NewGame, Continue, Achievements, Settings, Quit };

    using ActionCallback = std::function<void(Action)>;

    MainMenu();
    void setCallback(ActionCallback cb) { cb_ = std::move(cb); }
    void setTitleFont(const sf::Font* f) { titleFont_ = f; }
    void onClick(sf::Vector2f mouse);
    void onMouseMove(sf::Vector2f mouse);
    void draw(sf::RenderWindow& win, const sf::Font& font) const;
    int hoveredIndex() const { return hoveredIndex_; }

private:
    struct Button { Action action; std::string label; };
    std::vector<Button> buttons_;
    int hoveredIndex_ = -1;
    ActionCallback cb_;
    const sf::Font* titleFont_ = nullptr;

    int indexAt(sf::Vector2f mouse) const;
};

#endif
