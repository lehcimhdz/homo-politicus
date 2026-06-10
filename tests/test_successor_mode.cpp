#include "TestFramework.hpp"
#include "SuccessorMode.hpp"

TEST_CASE(successor_term_completed_eligible) {
    CHECK(SuccessorMode::isEligible(EndCondition::TERM_COMPLETED));
}

TEST_CASE(successor_election_loss_eligible) {
    CHECK(SuccessorMode::isEligible(EndCondition::ELECTION_LOSS));
}

TEST_CASE(successor_coup_not_eligible) {
    CHECK(!SuccessorMode::isEligible(EndCondition::COUP_SUCCESS));
}

TEST_CASE(successor_resets_popularity_and_pressures) {
    Country c;
    c.politics.popularity = 0.20;
    c.politics.military_pressure = 0.8;
    c.politics.congressional_pressure = 0.7;
    SuccessorMode::applyTo(c, EndCondition::TERM_COMPLETED);
    CHECK_NEAR(c.politics.popularity, 0.55, 0.001);
    CHECK_NEAR(c.politics.military_pressure, 0.4, 0.001);
    CHECK_NEAR(c.politics.congressional_pressure, 0.35, 0.001);
}

TEST_CASE(successor_election_loss_flips_ideology) {
    Country c;
    c.politics.economic_ideology = 0.20; // izquierda
    c.politics.social_ideology = 0.30;
    SuccessorMode::applyTo(c, EndCondition::ELECTION_LOSS);
    CHECK_NEAR(c.politics.economic_ideology, 0.80, 0.001);
    CHECK_NEAR(c.politics.social_ideology, 0.70, 0.001);
}

TEST_CASE(successor_term_completed_keeps_ideology) {
    Country c;
    c.politics.economic_ideology = 0.30;
    SuccessorMode::applyTo(c, EndCondition::TERM_COMPLETED);
    CHECK_NEAR(c.politics.economic_ideology, 0.30, 0.001);
}
