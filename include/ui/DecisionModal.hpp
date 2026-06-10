#ifndef DECISION_MODAL_HPP
#define DECISION_MODAL_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include "DecisionSystem.hpp"

// DecisionModal: overlay bloqueante con titulo, contexto, opciones + skip.
// Sprint 14: visible cuando hay una PendingDecision en la cola.
class DecisionModal {
public:
    using ChoiceCallback = std::function<void(const std::string& choiceId)>;
    using SkipCallback = std::function<void()>;

    DecisionModal();
    void show(const PendingDecision& d);
    void hide();
    bool visible() const { return visible_; }
    void setChoiceCallback(ChoiceCallback cb) { choiceCb_ = std::move(cb); }
    void setSkipCallback(SkipCallback cb) { skipCb_ = std::move(cb); }
    void onClick(sf::Vector2f mouse);
    void onMouseMove(sf::Vector2f mouse);
    void draw(sf::RenderWindow& win, const sf::Font& font) const;

    const std::string& hoveredOption() const { return hoveredOption_; }

private:
    bool visible_ = false;
    PendingDecision current_;
    std::string hoveredOption_;
    ChoiceCallback choiceCb_;
    SkipCallback skipCb_;
};

#endif
