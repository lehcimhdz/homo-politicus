#include "Tutorial.hpp"
#include <iostream>
#include <sstream>

Tutorial::Tutorial() {}

void Tutorial::start() {
    if (completed) return;
    active = true;
    mission = 1;
    missionStartTurn = 0;
    printMission(mission);
}

void Tutorial::skip() {
    active = false;
    completed = true;
    std::cout << ">> Tutorial saltado. Podes repetirlo con 'tutorial_reset'." << std::endl;
}

void Tutorial::reset() {
    completed = false;
    active = true;
    mission = 1;
    missionStartTurn = 0;
    std::cout << ">> Tutorial reiniciado." << std::endl;
    printMission(mission);
}

void Tutorial::advanceMission() {
    mission++;
    if (mission > 5) {
        active = false;
        completed = true;
        std::cout << "\n=================================================" << std::endl;
        std::cout << "  TUTORIAL COMPLETADO" << std::endl;
        std::cout << "  Ahora tenes 3 caminos:" << std::endl;
        std::cout << "    Sandbox: jugar libre sin meta." << std::endl;
        std::cout << "    Misiones: objetivos especificos por dificultad." << std::endl;
        std::cout << "    Historico: revivir uno de los 8 escenarios reales." << std::endl;
        std::cout << "  Buena suerte, presidente." << std::endl;
        std::cout << "=================================================" << std::endl;
    } else {
        std::cout << "\n>> MISION " << (mission - 1) << " COMPLETADA <<" << std::endl;
        printMission(mission);
    }
}

void Tutorial::printMission(int m) const {
    std::cout << "\n=== TUTORIAL: MISION " << m << "/5 ===" << std::endl;
    switch (m) {
        case 1:
            std::cout << "Objetivo: pasar 3 turnos sin que tu popularidad caiga bajo 55%." << std::endl;
            std::cout << "Tip: tipea 'next' para pasar de turno. Observa que cambia." << std::endl;
            break;
        case 2:
            std::cout << "Objetivo: reducir la inflacion bajo 4% en 5 turnos." << std::endl;
            std::cout << "Tip: probá 'tax+' o 'interest+' y observa el trade-off." << std::endl;
            break;
        case 3:
            std::cout << "Objetivo: mejorar relaciones con Easteria (vecino 1) a +0.5." << std::endl;
            std::cout << "Tip: 'improve_relations 1' o 'trade_deal 1'." << std::endl;
            break;
        case 4:
            std::cout << "Objetivo: sobrevivir 5 turnos sin caer bajo 35% popularidad." << std::endl;
            std::cout << "Tip: usa 'cover_up', 'scapegoat', 'apologize' o 'counter_narrative'." << std::endl;
            break;
        case 5:
            std::cout << "Objetivo: cuando aparezca una DECISION, resolvela sin perder el regimen." << std::endl;
            std::cout << "Tip: 'cede_power' es game over instantaneo. Pensa antes de elegir." << std::endl;
            break;
    }
    std::cout << "(Para saltar: 'tutorial_skip')" << std::endl;
    std::cout << "==============================" << std::endl;
}

void Tutorial::onTurnEnd(const Country& c, int turnCount) {
    if (!active || completed) return;
    int elapsed = turnCount - missionStartTurn;
    bool met = false;
    switch (mission) {
        case 1:
            if (elapsed >= 3 && c.politics.popularity >= 0.55) met = true;
            break;
        case 2:
            if (c.economy.inflation < 0.04) met = true;
            else if (elapsed >= 5) {
                std::cout << ">> Mision 2: tiempo agotado. Reintentando." << std::endl;
                missionStartTurn = turnCount;
            }
            break;
        case 3:
            for (const auto& n : c.neighbors) {
                if (n.name.find("Easteria") != std::string::npos && n.diplomatic_relations >= 0.5) met = true;
            }
            break;
        case 4:
            if (elapsed >= 5 && c.politics.popularity >= 0.35) met = true;
            else if (c.politics.popularity < 0.35) {
                std::cout << ">> Mision 4: caiste muy bajo. Reintentando." << std::endl;
                missionStartTurn = turnCount;
            }
            break;
        case 5:
            // Se considera completa cuando hay 1 decision resuelta y el jugador sigue
            if (elapsed >= 3 && c.politics.popularity > 0.3) met = true;
            break;
    }
    if (met) {
        missionStartTurn = turnCount;
        advanceMission();
    }
}

std::string Tutorial::serialize() const {
    std::ostringstream oss;
    oss << (completed ? 1 : 0) << "|" << mission << "|" << missionStartTurn;
    return oss.str();
}

void Tutorial::deserialize(const std::string& s) {
    if (s.empty()) return;
    int c; int m; int ms;
    char sep;
    std::istringstream iss(s);
    iss >> c >> sep >> m >> sep >> ms;
    completed = (c == 1);
    mission = m;
    missionStartTurn = ms;
    active = !completed;
}
