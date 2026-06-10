#include "TestFramework.hpp"
#include "Tutorial.hpp"
#include "Country.hpp"

TEST_CASE(tutorial_starts_at_mission_1) {
    Tutorial t;
    t.start();
    CHECK(t.isActive());
    CHECK_EQ(t.currentMission(), 1);
}

TEST_CASE(tutorial_skip_marks_completed) {
    Tutorial t;
    t.start();
    t.skip();
    CHECK(!t.isActive());
    CHECK(t.isCompleted());
}

TEST_CASE(tutorial_mission_1_completes_after_3_turns_with_high_pop) {
    Tutorial t;
    t.start();
    Country c;
    c.politics.popularity = 0.60;
    t.onTurnEnd(c, 1);
    t.onTurnEnd(c, 2);
    t.onTurnEnd(c, 3);
    CHECK_EQ(t.currentMission(), 2);
}

TEST_CASE(tutorial_mission_2_advances_when_inflation_low) {
    Tutorial t;
    t.start();
    t.skip();
    t.reset();
    t.onTurnEnd(Country(), 0); // arranca M1
    Country c;
    c.politics.popularity = 0.60;
    t.onTurnEnd(c, 3); // termina M1
    c.economy.inflation = 0.03;
    t.onTurnEnd(c, 4); // M2 success
    CHECK(t.currentMission() >= 3);
}

TEST_CASE(tutorial_serialize_deserialize) {
    Tutorial t;
    t.start();
    t.skip();
    std::string s = t.serialize();
    Tutorial t2;
    t2.deserialize(s);
    CHECK(t2.isCompleted());
}
