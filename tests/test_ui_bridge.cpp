#include "TestFramework.hpp"
#include "ui/UIBridge.hpp"

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

TEST_CASE(uibridge_tick_grows_gdp_by_growth_rate) {
    UIBridge b;
    double gdp0 = b.country().economy.gdp;
    b.tick();
    CHECK(b.country().economy.gdp > gdp0);
}

TEST_CASE(uibridge_reset_returns_to_initial_state) {
    UIBridge b;
    b.tick(); b.tick(); b.tick();
    b.resetCountry();
    CHECK_EQ(b.turn(), 0);
}
