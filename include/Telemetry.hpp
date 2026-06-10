#ifndef TELEMETRY_HPP
#define TELEMETRY_HPP

#include <string>
#include <unordered_map>

// Telemetría opt-in. Solo registra eventos a archivo local; nunca envía.
// Si el jugador opta por sharing, sube `~/.config/HomoPoliticus/telemetry.csv`.
namespace Telemetry {
    void enable(bool e);
    bool isEnabled();

    void recordCommand(const std::string& cmdId);
    void recordEvent(const std::string& eventId);
    void recordEndCondition(int endCond, int turnCount, int score);

    // Vuelca el contador agregado al log para debug.
    std::unordered_map<std::string, int> snapshot();
    void reset();
}

#endif
