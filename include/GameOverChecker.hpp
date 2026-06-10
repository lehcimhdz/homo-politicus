#ifndef GAME_OVER_CHECKER_HPP
#define GAME_OVER_CHECKER_HPP

#include <random>
#include "Country.hpp"
#include "EndCondition.hpp"

namespace GameOverChecker {
    // Evalúa el estado del país y devuelve la condición de fin disparada,
    // o EndCondition::NONE si la partida sigue.
    EndCondition evaluate(const Country& c, std::mt19937& rng);
}

#endif
