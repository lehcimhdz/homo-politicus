#ifndef SUCCESSOR_MODE_HPP
#define SUCCESSOR_MODE_HPP

#include "Country.hpp"
#include "EndCondition.hpp"

// SuccessorMode: cuando una partida termina por TERM_COMPLETED o ELECTION_LOSS,
// el jugador puede continuar como su sucesor. El sucesor hereda el estado del país
// pero con ajustes (popularidad reseteada, ciertos índices invertidos).
namespace SuccessorMode {
    // Verifica si la condición de fin permite jugar como sucesor.
    bool isEligible(EndCondition end);

    // Aplica al Country los ajustes que representan al sucesor:
    // - popularidad reseteada al 55% (luna de miel)
    // - turnos en poder = 0
    // - escándalos heredados se mantienen (te marcan)
    // - presiones reseteadas al 50% de los valores anteriores
    // - si end == ELECTION_LOSS: ideología flipea (opuesta)
    void applyTo(Country& c, EndCondition prev);
}

#endif
