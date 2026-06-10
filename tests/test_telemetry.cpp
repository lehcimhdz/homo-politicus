#include "TestFramework.hpp"
#include "Telemetry.hpp"

TEST_CASE(telemetry_disabled_by_default) {
    Telemetry::reset();
    CHECK(!Telemetry::isEnabled());
}

TEST_CASE(telemetry_disabled_records_nothing) {
    Telemetry::reset();
    Telemetry::enable(false);
    Telemetry::recordCommand("tax+");
    auto s = Telemetry::snapshot();
    CHECK_EQ(s.size(), (size_t)0);
}

TEST_CASE(telemetry_enabled_records_commands) {
    Telemetry::reset();
    Telemetry::enable(true);
    Telemetry::recordCommand("tax+");
    Telemetry::recordCommand("tax+");
    Telemetry::recordCommand("next");
    auto s = Telemetry::snapshot();
    CHECK(s["cmd:tax+"] == 2);
    CHECK(s["cmd:next"] == 1);
}

TEST_CASE(telemetry_end_condition_aggregates) {
    Telemetry::reset();
    Telemetry::enable(true);
    Telemetry::recordEndCondition(1, 12, 80);
    Telemetry::recordEndCondition(1, 8, 50);
    auto s = Telemetry::snapshot();
    CHECK(s["games_total"] == 2);
    CHECK(s["turns_total"] == 20);
    CHECK(s["score_total"] == 130);
}
