#ifndef UI_BRIDGE_HPP
#define UI_BRIDGE_HPP

#include <memory>
#include <string>
#include <vector>
#include "Country.hpp"
#include "EndCondition.hpp"
#include "Game.hpp"

// Bridge entre la UI grafica y el engine. Sprint C19.1: ahora posee un Game
// real con todos los feedback loops del motor (welfare/economy/politics/
// security/infra) en lugar del placeholder de tick anterior.
class UIBridge {
public:
    UIBridge();
    void tick();
    void resetCountry();
    const Country& country() const { return game_.playerCountryRef(); }
    Country& mutableCountry() { return game_.playerCountryRef(); }
    int turn() const { return game_.turnCountValue(); }
    EndCondition endCondition() const { return game_.endConditionValue(); }
    bool isGameOver() const { return endCondition() != EndCondition::NONE; }
    Game& game() { return game_; }
    const Game& game() const { return game_; }
    // C20.4: drena lineas del motor con marker [!!!] o [INFO] capturadas
    // durante el ultimo tick. Vacia el buffer.
    std::vector<std::string> drainCriticalMessages();

private:
    Game game_;
    std::vector<std::string> criticalBuf_;
};

#endif
