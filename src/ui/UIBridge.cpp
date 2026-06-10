#include "ui/UIBridge.hpp"

UIBridge::UIBridge() : c_(), turn_(0), end_(EndCondition::NONE) {}

void UIBridge::tick() {
    if (end_ != EndCondition::NONE) return;
    turn_++;
    // Placeholder: una simulacion minima por turno. El engine completo se
    // wirea en Sprint 13 cuando se ejecuten comandos desde la UI.
    c_.economy.gdp *= (1.0 + c_.economy.growth_rate / 4.0);
    c_.politics.popularity += (0.55 - c_.politics.popularity) * 0.03; // suavizado al baseline
    if (c_.politics.popularity < 0.0) c_.politics.popularity = 0.0;
    if (c_.politics.popularity > 1.0) c_.politics.popularity = 1.0;
}

void UIBridge::resetCountry() {
    c_ = Country();
    turn_ = 0;
    end_ = EndCondition::NONE;
}
