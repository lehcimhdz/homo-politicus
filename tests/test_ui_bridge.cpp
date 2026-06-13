#include "TestFramework.hpp"
#include "ui/UIBridge.hpp"
#include <cmath>

TEST_CASE(uibridge_starts_at_turn_0_with_default_country) {
    UIBridge b;
    CHECK_EQ(b.turn(), 0);
    CHECK(!b.isGameOver());
    CHECK(b.country().economy.gdp > 0.0);
}

TEST_CASE(uibridge_tick_advances_turn) {
    UIBridge b;
    b.tick();
    CHECK_EQ(b.turn(), 1);
    b.tick();
    CHECK_EQ(b.turn(), 2);
}

TEST_CASE(uibridge_tick_changes_economy_state) {
    // Sprint C19.1: ahora corre el motor real con feedback loops, asi que
    // gdp puede subir O bajar segun condiciones. Solo verificamos cambio.
    UIBridge b;
    double gdp0 = b.country().economy.gdp;
    double inf0 = b.country().economy.inflation;
    b.tick();
    bool gdpChanged = std::abs(b.country().economy.gdp - gdp0) > 1.0;
    bool infChanged = std::abs(b.country().economy.inflation - inf0) > 0.0001;
    CHECK(gdpChanged || infChanged);
}

TEST_CASE(uibridge_reset_returns_to_initial_state) {
    UIBridge b;
    b.tick(); b.tick(); b.tick();
    b.resetCountry();
    CHECK_EQ(b.turn(), 0);
}
