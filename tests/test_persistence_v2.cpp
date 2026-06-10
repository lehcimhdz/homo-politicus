#include "TestFramework.hpp"
#include "Persistence.hpp"
#include "Country.hpp"
#include "EndCondition.hpp"
#include <cstdio>

static const char* TMP = "/tmp/hp_test_persist_v2.txt";

TEST_CASE(persistence_v2_extended_welfare_roundtrip) {
    Country c;
    c.welfare.literacy_rate = 0.91;
    c.welfare.minority_protection = 0.77;
    c.welfare.un_score = 0.42;
    c.welfare.minimum_wage = 1500.0;
    c.welfare.retirement_age = 67.0;
    CHECK(Persistence::save(c, 5, 2.0, EndCondition::NONE, TMP));

    Country c2;
    int t = 0; double s = 0.0; EndCondition ec = EndCondition::NONE;
    Persistence::load(c2, t, s, ec, TMP);
    CHECK_NEAR(c2.welfare.literacy_rate, 0.91, 0.001);
    CHECK_NEAR(c2.welfare.minority_protection, 0.77, 0.001);
    CHECK_NEAR(c2.welfare.un_score, 0.42, 0.001);
    CHECK_NEAR(c2.welfare.minimum_wage, 1500.0, 0.001);
    CHECK_NEAR(c2.welfare.retirement_age, 67.0, 0.001);
    std::remove(TMP);
}

TEST_CASE(persistence_v2_extended_economy_roundtrip) {
    Country c;
    c.economy.commodity_supercycle = 0.35;
    c.economy.sovereign_wealth_fund = 250000000.0;
    c.economy.debt_to_gdp_ratio = 0.85;
    c.economy.average_tariffs = 0.18;
    CHECK(Persistence::save(c, 0, 0.0, EndCondition::NONE, TMP));

    Country c2;
    int t = 0; double s = 0.0; EndCondition ec = EndCondition::NONE;
    Persistence::load(c2, t, s, ec, TMP);
    CHECK_NEAR(c2.economy.commodity_supercycle, 0.35, 0.001);
    CHECK_NEAR(c2.economy.sovereign_wealth_fund, 250000000.0, 1.0);
    CHECK_NEAR(c2.economy.debt_to_gdp_ratio, 0.85, 0.001);
    CHECK_NEAR(c2.economy.average_tariffs, 0.18, 0.001);
    std::remove(TMP);
}

TEST_CASE(persistence_v2_revolution_and_impeachment_state) {
    Country c;
    c.politics.revolution_active = true;
    c.politics.revolution_duration = 3;
    c.politics.impeachment_in_progress = true;
    c.politics.impeachment_turns = 2;
    c.politics.apologize_cooldown_turns = 2;
    c.politics.threaten_streak_count = 4;
    CHECK(Persistence::save(c, 0, 0.0, EndCondition::NONE, TMP));

    Country c2;
    int t = 0; double s = 0.0; EndCondition ec = EndCondition::NONE;
    Persistence::load(c2, t, s, ec, TMP);
    CHECK(c2.politics.revolution_active);
    CHECK_EQ(c2.politics.revolution_duration, 3);
    CHECK(c2.politics.impeachment_in_progress);
    CHECK_EQ(c2.politics.impeachment_turns, 2);
    CHECK_EQ(c2.politics.apologize_cooldown_turns, 2);
    CHECK_EQ(c2.politics.threaten_streak_count, 4);
    std::remove(TMP);
}

TEST_CASE(persistence_v2_security_infra_roundtrip) {
    Country c;
    c.security.us_alignment = 0.75;
    c.security.china_alignment = 0.30;
    c.security.diplomatic_prestige = 0.65;
    c.infra.renewables_percentage = 0.45;
    c.infra.internet_coverage = 0.92;
    CHECK(Persistence::save(c, 0, 0.0, EndCondition::NONE, TMP));

    Country c2;
    int t = 0; double s = 0.0; EndCondition ec = EndCondition::NONE;
    Persistence::load(c2, t, s, ec, TMP);
    CHECK_NEAR(c2.security.us_alignment, 0.75, 0.001);
    CHECK_NEAR(c2.security.china_alignment, 0.30, 0.001);
    CHECK_NEAR(c2.security.diplomatic_prestige, 0.65, 0.001);
    CHECK_NEAR(c2.infra.renewables_percentage, 0.45, 0.001);
    CHECK_NEAR(c2.infra.internet_coverage, 0.92, 0.001);
    std::remove(TMP);
}
