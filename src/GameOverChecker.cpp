#include "GameOverChecker.hpp"

namespace GameOverChecker {

EndCondition evaluate(const Country& c, std::mt19937& rng) {
    const auto& pol = c.politics;
    const auto& sec = c.security;
    std::uniform_real_distribution<double> roll(0.0, 1.0);

    if (sec.nuclear_strike && sec.nuclear_casualties > 1000000.0)
        return EndCondition::NUCLEAR_ANNIHILATION;
    if (pol.military_pressure >= 0.85 && roll(rng) < pol.coup_success_prob)
        return EndCondition::COUP_SUCCESS;
    if (pol.popular_pressure >= 0.9 && pol.revolution_prob > 0.5)
        return EndCondition::REVOLUTION;
    if (pol.congressional_pressure >= 0.85 && pol.popularity < 0.3)
        return EndCondition::IMPEACHMENT;
    if (pol.judicial_pressure >= 0.85 && pol.active_scandals > 2)
        return EndCondition::LAWFARE_REMOVAL;
    if (pol.international_pressure >= 0.9 && pol.popularity < 0.2)
        return EndCondition::EXILE;
    if (pol.active_scandals >= 4 && pol.popularity < 0.1 && roll(rng) < 0.02)
        return EndCondition::ASSASSINATION;
    return EndCondition::NONE;
}

} // namespace GameOverChecker
