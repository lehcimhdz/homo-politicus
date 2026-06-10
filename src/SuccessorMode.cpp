#include "SuccessorMode.hpp"

namespace SuccessorMode {

bool isEligible(EndCondition end) {
    return end == EndCondition::TERM_COMPLETED || end == EndCondition::ELECTION_LOSS;
}

void applyTo(Country& c, EndCondition prev) {
    c.politics.popularity = 0.55;
    c.politics.honeymoon_turns_remaining = 4;
    c.politics.congressional_pressure *= 0.5;
    c.politics.judicial_pressure *= 0.5;
    c.politics.military_pressure *= 0.5;
    c.politics.popular_pressure *= 0.5;
    c.politics.international_pressure *= 0.5;
    c.politics.coup_attempts_history = 0;
    c.politics.terms_served = 0;

    if (prev == EndCondition::ELECTION_LOSS) {
        // El opositor gana: invierte ideología
        c.politics.economic_ideology = 1.0 - c.politics.economic_ideology;
        c.politics.social_ideology = 1.0 - c.politics.social_ideology;
    }
}

}
