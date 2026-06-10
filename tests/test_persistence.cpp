#include "TestFramework.hpp"
#include "Persistence.hpp"
#include "Country.hpp"
#include "EndCondition.hpp"
#include <cstdio>

static const char* TMP_PATH = "/tmp/hp_test_save.txt";

TEST_CASE(save_load_economy_roundtrip) {
    Country c;
    c.economy.gdp = 999.5;
    c.economy.inflation = 0.07;
    c.economy.growth_rate = -0.03;
    c.economy.in_recession = true;
    int turn = 5;
    double sum = 2.1;
    EndCondition ec = EndCondition::NONE;
    CHECK(Persistence::save(c, turn, sum, ec, TMP_PATH));

    Country c2;
    int t2 = 0; double sum2 = 0.0; EndCondition ec2 = EndCondition::NONE;
    CHECK(Persistence::load(c2, t2, sum2, ec2, TMP_PATH));
    CHECK_NEAR(c2.economy.gdp, 999.5, 0.001);
    CHECK_NEAR(c2.economy.inflation, 0.07, 0.0001);
    CHECK_NEAR(c2.economy.growth_rate, -0.03, 0.0001);
    CHECK(c2.economy.in_recession == true);
    CHECK_EQ(t2, 5);
    std::remove(TMP_PATH);
}

TEST_CASE(save_load_politics_roundtrip) {
    Country c;
    c.politics.popularity = 0.42;
    c.politics.congressional_pressure = 0.7;
    c.politics.judicial_pressure = 0.4;
    c.politics.military_pressure = 0.6;
    c.politics.active_scandals = 3;
    c.politics.scandal_corruption_severity = 0.8;
    CHECK(Persistence::save(c, 10, 5.0, EndCondition::NONE, TMP_PATH));

    Country c2;
    int t = 0; double s = 0.0; EndCondition ec = EndCondition::NONE;
    Persistence::load(c2, t, s, ec, TMP_PATH);
    CHECK_NEAR(c2.politics.popularity, 0.42, 0.0001);
    CHECK_NEAR(c2.politics.congressional_pressure, 0.7, 0.0001);
    CHECK_NEAR(c2.politics.judicial_pressure, 0.4, 0.0001);
    CHECK_NEAR(c2.politics.military_pressure, 0.6, 0.0001);
    CHECK_EQ(c2.politics.active_scandals, 3);
    CHECK_NEAR(c2.politics.scandal_corruption_severity, 0.8, 0.0001);
    std::remove(TMP_PATH);
}

TEST_CASE(save_load_flags_roundtrip) {
    Country c;
    c.security.war_active = true;
    c.security.war_duration = 12;
    c.politics.civil_war_active = true;
    c.welfare.pandemic_active = true;
    c.welfare.pandemic_severity = 0.55;
    CHECK(Persistence::save(c, 0, 0.0, EndCondition::COUP_SUCCESS, TMP_PATH));

    Country c2;
    int t = 0; double s = 0.0; EndCondition ec = EndCondition::NONE;
    Persistence::load(c2, t, s, ec, TMP_PATH);
    CHECK(c2.security.war_active);
    CHECK_EQ(c2.security.war_duration, 12);
    CHECK(c2.politics.civil_war_active);
    CHECK(c2.welfare.pandemic_active);
    CHECK_NEAR(c2.welfare.pandemic_severity, 0.55, 0.0001);
    CHECK(ec == EndCondition::COUP_SUCCESS);
    std::remove(TMP_PATH);
}

TEST_CASE(load_missing_file_returns_false) {
    Country c;
    int t = 0; double s = 0.0; EndCondition ec = EndCondition::NONE;
    CHECK(Persistence::load(c, t, s, ec, "/tmp/__definitely_no_such_file_hp__.txt") == false);
}
