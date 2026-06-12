#ifndef UI_BRIDGE_HPP
#define UI_BRIDGE_HPP

#include <memory>
#include <string>
#include "Country.hpp"
#include "EndCondition.hpp"

// Bridge entre la UI grafica y el engine. Posee una instancia minima de estado.
// Sprint 10: solo Country + turnCount; siguientes sprints agregan decisiones,
// achievements, scenario loader, etc.
class UIBridge {
public:
    UIBridge();
    void tick();                            // Avanza un turno (placeholder simple)
    void resetCountry();
    const Country& country() const { return c_; }
    Country& mutableCountry() { return c_; }
    int turn() const { return turn_; }
    EndCondition endCondition() const { return end_; }
    bool isGameOver() const { return end_ != EndCondition::NONE; }

private:
    Country c_;
    int turn_ = 0;
    EndCondition end_ = EndCondition::NONE;
};

#endif
