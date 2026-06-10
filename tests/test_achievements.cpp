#include "TestFramework.hpp"
#include "AchievementTracker.hpp"
#include "Country.hpp"

TEST_CASE(achievement_first_year_at_turn_12) {
    AchievementTracker t;
    Country c;
    t.evaluate(c, 12, EndCondition::NONE, c.economy.gdp);
    CHECK(t.isUnlocked("first_year"));
    CHECK(!t.isUnlocked("decade_in_power"));
}

TEST_CASE(achievement_decade_in_power) {
    AchievementTracker t;
    Country c;
    t.evaluate(c, 40, EndCondition::NONE, c.economy.gdp);
    CHECK(t.isUnlocked("decade_in_power"));
    CHECK(t.isUnlocked("first_year"));
}

TEST_CASE(achievement_economic_miracle) {
    AchievementTracker t;
    Country c;
    double initial = c.economy.gdp;
    c.economy.gdp = initial * 1.6;
    t.evaluate(c, 5, EndCondition::NONE, initial);
    CHECK(t.isUnlocked("economic_miracle"));
}

TEST_CASE(achievement_default_d) {
    AchievementTracker t;
    Country c;
    c.economy.credit_rating = CreditRating::D;
    t.evaluate(c, 1, EndCondition::NONE, c.economy.gdp);
    CHECK(t.isUnlocked("default_d"));
}

TEST_CASE(achievement_caudillo) {
    AchievementTracker t;
    Country c;
    c.politics.authoritarian_actions_count = 10;
    t.evaluate(c, 1, EndCondition::NONE, c.economy.gdp);
    CHECK(t.isUnlocked("caudillo"));
}

TEST_CASE(achievement_constitutionalist_only_at_end) {
    AchievementTracker t;
    Country c;
    c.politics.authoritarian_actions_count = 0;
    t.evaluate(c, 5, EndCondition::NONE, c.economy.gdp);
    CHECK(!t.isUnlocked("constitutionalist"));
    t.evaluate(c, 5, EndCondition::TERM_COMPLETED, c.economy.gdp);
    CHECK(t.isUnlocked("constitutionalist"));
}

TEST_CASE(achievement_ended_by_coup) {
    AchievementTracker t;
    Country c;
    t.evaluate(c, 8, EndCondition::COUP_SUCCESS, c.economy.gdp);
    CHECK(t.isUnlocked("ended_by_coup"));
    CHECK(t.isUnlocked("speedrun_collapse"));
}

TEST_CASE(achievement_persistence_roundtrip) {
    AchievementTracker t;
    Country c;
    t.evaluate(c, 12, EndCondition::NONE, c.economy.gdp);
    std::string serialized = t.serialize();
    AchievementTracker t2;
    t2.deserialize(serialized);
    CHECK(t2.isUnlocked("first_year"));
}
