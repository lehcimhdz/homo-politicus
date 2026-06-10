#include "TestFramework.hpp"
#include "Country.hpp"
#include "GameOverChecker.hpp"
#include "EndCondition.hpp"
#include "AchievementTracker.hpp"
#include "Persistence.hpp"
#include <random>
#include <cstdio>

// Tests de integración: simular partidas completas y validar invariantes globales.

TEST_CASE(integration_default_game_does_not_crash_after_50_iterations) {
    Country c;
    AchievementTracker tracker;
    std::mt19937 rng(42);
    EndCondition end = EndCondition::NONE;
    for (int turn = 0; turn < 50; ++turn) {
        end = GameOverChecker::evaluate(c, rng);
        if (end != EndCondition::NONE) break;
        tracker.recordHistory(c);
        tracker.evaluate(c, turn, end, c.economy.gdp);
    }
    CHECK(true);
}

TEST_CASE(integration_save_load_after_complex_state) {
    Country c;
    c.politics.popularity = 0.43;
    c.economy.inflation = 6.06;
    c.security.war_active = true;
    c.security.war_duration = 5;
    c.politics.revolution_active = true;
    c.politics.revolution_duration = 2;

    const char* path = "/tmp/hp_integration_save.txt";
    CHECK(Persistence::save(c, 12, 5.0, EndCondition::NONE, path));

    Country c2;
    int t; double s; EndCondition ec;
    CHECK(Persistence::load(c2, t, s, ec, path));
    CHECK_NEAR(c2.politics.popularity, 0.43, 0.01);
    CHECK_NEAR(c2.economy.inflation, 6.06, 0.01);
    CHECK(c2.security.war_active);
    CHECK_EQ(c2.security.war_duration, 5);
    CHECK(c2.politics.revolution_active);
    CHECK_EQ(c2.politics.revolution_duration, 2);
    std::remove(path);
}

TEST_CASE(integration_extreme_state_no_crash) {
    Country c;
    c.politics.popularity = 0.0;
    c.economy.inflation = 50.0;
    c.economy.gdp = 1.0;
    c.politics.military_pressure = 0.99;
    c.politics.congressional_pressure = 0.99;
    c.politics.judicial_pressure = 0.99;
    std::mt19937 rng(1);
    auto end = GameOverChecker::evaluate(c, rng);
    // Debe disparar alguna condición end != NONE
    CHECK(end != EndCondition::NONE);
}
