#include "TestFramework.hpp"
#include "GameOverChecker.hpp"
#include "Country.hpp"
#include "EndCondition.hpp"
#include <random>

TEST_CASE(gameover_nuclear_annihilation) {
    Country c;
    c.security.nuclear_strike = true;
    c.security.nuclear_casualties = 5000000.0;
    std::mt19937 rng(42);
    CHECK(GameOverChecker::evaluate(c, rng) == EndCondition::NUCLEAR_ANNIHILATION);
}

TEST_CASE(gameover_coup_with_deterministic_rng) {
    Country c;
    c.politics.military_pressure = 0.95;
    c.politics.coup_success_prob = 1.0; // garantizado
    std::mt19937 rng(42);
    CHECK(GameOverChecker::evaluate(c, rng) == EndCondition::COUP_SUCCESS);
}

TEST_CASE(gameover_impeachment) {
    Country c;
    c.politics.congressional_pressure = 0.95;
    c.politics.popularity = 0.1;
    std::mt19937 rng(42);
    CHECK(GameOverChecker::evaluate(c, rng) == EndCondition::IMPEACHMENT);
}

TEST_CASE(gameover_revolution) {
    Country c;
    c.politics.popular_pressure = 0.95;
    c.politics.revolution_prob = 0.7;
    std::mt19937 rng(42);
    CHECK(GameOverChecker::evaluate(c, rng) == EndCondition::REVOLUTION);
}

TEST_CASE(gameover_lawfare) {
    Country c;
    c.politics.judicial_pressure = 0.9;
    c.politics.active_scandals = 3;
    std::mt19937 rng(42);
    CHECK(GameOverChecker::evaluate(c, rng) == EndCondition::LAWFARE_REMOVAL);
}

TEST_CASE(gameover_exile) {
    Country c;
    c.politics.international_pressure = 0.95;
    c.politics.popularity = 0.15;
    std::mt19937 rng(42);
    CHECK(GameOverChecker::evaluate(c, rng) == EndCondition::EXILE);
}

TEST_CASE(gameover_none_at_init) {
    Country c;
    std::mt19937 rng(42);
    CHECK(GameOverChecker::evaluate(c, rng) == EndCondition::NONE);
}

TEST_CASE(gameover_nuclear_low_casualties_no_trigger) {
    Country c;
    c.security.nuclear_strike = true;
    c.security.nuclear_casualties = 500.0; // muy bajo
    std::mt19937 rng(42);
    CHECK(GameOverChecker::evaluate(c, rng) == EndCondition::NONE);
}
