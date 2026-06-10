#include "TestFramework.hpp"
#include "AchievementTracker.hpp"
#include "Country.hpp"

TEST_CASE(achievements_catalog_has_40) {
    CHECK(AchievementTracker::catalog().size() >= 40);
}

TEST_CASE(achievements_history_records_min_popularity) {
    AchievementTracker t;
    Country c;
    c.politics.popularity = 0.55;
    t.recordHistory(c);
    c.politics.popularity = 0.25;
    t.recordHistory(c);
    c.politics.popularity = 0.45;
    t.recordHistory(c);
    t.evaluate(c, 15, EndCondition::NONE, c.economy.gdp);
    CHECK(!t.isUnlocked("high_min_popularity"));
}

TEST_CASE(achievements_high_min_popularity_when_kept_high) {
    AchievementTracker t;
    Country c;
    c.politics.popularity = 0.65;
    for (int i = 0; i < 13; ++i) t.recordHistory(c);
    t.evaluate(c, 13, EndCondition::NONE, c.economy.gdp);
    CHECK(t.isUnlocked("high_min_popularity"));
}

TEST_CASE(achievements_inflation_tamed_requires_20_consecutive) {
    AchievementTracker t;
    Country c;
    c.economy.inflation = 0.02;
    for (int i = 0; i < 20; ++i) t.recordHistory(c);
    t.evaluate(c, 20, EndCondition::NONE, c.economy.gdp);
    CHECK(t.isUnlocked("inflation_tamed"));
}

TEST_CASE(achievements_houdini_3_consecutive_coverups) {
    AchievementTracker t;
    Country c;
    t.noteCoverUpSuccess();
    t.noteCoverUpSuccess();
    t.noteCoverUpSuccess();
    t.evaluate(c, 5, EndCondition::NONE, c.economy.gdp);
    CHECK(t.isUnlocked("houdini"));
}

TEST_CASE(achievements_martyr_5_apologize) {
    AchievementTracker t;
    Country c;
    for (int i = 0; i < 5; ++i) t.noteApologize();
    t.evaluate(c, 5, EndCondition::NONE, c.economy.gdp);
    CHECK(t.isUnlocked("martyr"));
}

TEST_CASE(achievements_trade_master_5_ftas) {
    AchievementTracker t;
    Country c;
    for (int i = 0; i < 5; ++i) t.noteFTASigned();
    t.evaluate(c, 1, EndCondition::NONE, c.economy.gdp);
    CHECK(t.isUnlocked("trade_master"));
}

TEST_CASE(achievements_nuclear_survivor) {
    AchievementTracker t;
    Country c;
    c.security.nuclear_attack_prob = 0.8;
    t.recordHistory(c);
    c.security.nuclear_attack_prob = 0.1;
    c.security.nuclear_strike = false;
    t.evaluate(c, 5, EndCondition::NONE, c.economy.gdp);
    CHECK(t.isUnlocked("nuclear_survivor"));
}

TEST_CASE(achievements_climate_champion) {
    AchievementTracker t;
    Country c;
    c.infra.renewables_percentage = 0.55;
    t.evaluate(c, 1, EndCondition::NONE, c.economy.gdp);
    CHECK(t.isUnlocked("climate_champion"));
}

TEST_CASE(achievements_ended_by_lawfare) {
    AchievementTracker t;
    Country c;
    t.evaluate(c, 5, EndCondition::LAWFARE_REMOVAL, c.economy.gdp);
    CHECK(t.isUnlocked("ended_by_lawfare"));
}
