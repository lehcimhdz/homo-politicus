#ifndef GAME_OVER_SCREEN_HPP
#define GAME_OVER_SCREEN_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
#include "EndCondition.hpp"
#include "Country.hpp"

// GameOverScreen: pantalla final con condicion, resumen, score.
class GameOverScreen {
public:
    enum class Action { None, NewGame, MainMenu };
    using ActionCallback = std::function<void(Action)>;

    void show(EndCondition cond, const Country& c, int turn, double popularitySum);
    void hide();
    bool visible() const { return visible_; }
    void setCallback(ActionCallback cb) { cb_ = std::move(cb); }
    void onClick(sf::Vector2f mouse);
    void update(float dt);
    void draw(sf::RenderWindow& win, const sf::Font& font) const;

    // Score calculado, expuesto para tests.
    int score() const { return score_; }
    const std::string& conditionLabel() const { return label_; }

private:
    bool visible_ = false;
    EndCondition cond_ = EndCondition::NONE;
    Country snapshot_;
    int turn_ = 0;
    double popSum_ = 0.0;
    std::string label_;
    int score_ = 0;
    float fadeTime_ = 0.f;
    ActionCallback cb_;

    void computeScore();
    void deriveLabel();
};

#endif
