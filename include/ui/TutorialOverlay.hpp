#ifndef TUTORIAL_OVERLAY_HPP
#define TUTORIAL_OVERLAY_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <functional>

// TutorialOverlay: pop-ups guiados encima del juego.
// 5 misiones con texto + tip. Avanza con tecla SPACE o boton Siguiente.
class TutorialOverlay {
public:
    using SkipCallback = std::function<void()>;

    void start();
    void advance();   // siguiente paso
    void skip();
    bool visible() const { return visible_; }
    bool completed() const { return completed_; }
    int currentMission() const { return mission_; }
    void setSkipCallback(SkipCallback cb) { skipCb_ = std::move(cb); }
    void onClick(sf::Vector2f mouse);
    void draw(sf::RenderWindow& win, const sf::Font& font) const;

private:
    bool visible_ = false;
    bool completed_ = false;
    int mission_ = 1; // 1..5
    SkipCallback skipCb_;

    struct MissionText { std::string title; std::string body; std::string tip; };
    static MissionText textFor(int m);
};

#endif
