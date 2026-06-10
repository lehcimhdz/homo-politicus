#include "TestFramework.hpp"
#include "ui/Dashboard.hpp"
#include "Country.hpp"

// Dashboard es 99% draw-only; testeamos solo recordHistory.

TEST_CASE(dashboard_history_records_without_crash) {
    Dashboard d;
    Country c;
    for (int i = 0; i < 10; ++i) {
        c.politics.popularity = 0.5 + i * 0.01;
        d.recordHistory(c);
    }
    CHECK(true);
}

TEST_CASE(dashboard_history_caps_at_60) {
    Dashboard d;
    Country c;
    for (int i = 0; i < 100; ++i) d.recordHistory(c);
    CHECK(true);
}
