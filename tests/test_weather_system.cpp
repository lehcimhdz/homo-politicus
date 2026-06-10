#include "TestFramework.hpp"
#include "ui/WeatherSystem.hpp"
#include <string>

TEST_CASE(weather_state_cycle_is_deterministic) {
    WeatherSystem w;
    w.setTurn(0);
    auto s0 = w.state();
    w.setTurn(16);
    auto s16 = w.state();
    CHECK(s0 == s16);
}

TEST_CASE(weather_state_changes_with_turn) {
    WeatherSystem w;
    w.setTurn(0);
    auto sunny = w.state();
    w.setTurn(10);
    auto rainy = w.state();
    CHECK(sunny != rainy);
    CHECK(rainy == WeatherSystem::State::Rain);
}

TEST_CASE(weather_label_non_empty) {
    WeatherSystem w;
    for (int t = 0; t < 16; ++t) {
        w.setTurn(t);
        std::string lbl = w.label();
        CHECK(!lbl.empty());
    }
}
