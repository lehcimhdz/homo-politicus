#ifndef TUTORIAL_HPP
#define TUTORIAL_HPP

#include <string>
#include <vector>
#include "Country.hpp"

class Tutorial {
public:
    Tutorial();
    bool isActive() const { return active && !completed; }
    bool isCompleted() const { return completed; }
    int currentMission() const { return mission; }

    // Llamado al inicio del juego
    void start();
    // Llamado al final de cada turno
    void onTurnEnd(const Country& c, int turnCount);
    // Saltar tutorial
    void skip();
    // Resetear (forzar repetir)
    void reset();

    // Persistencia
    std::string serialize() const;
    void deserialize(const std::string& s);

private:
    bool active = false;
    bool completed = false;
    int mission = 1; // 1..5
    int missionStartTurn = 0;
    void advanceMission();
    void printMission(int m) const;
};

#endif
