#include "TestFramework.hpp"
#include "ui/TutorialOverlay.hpp"

TEST_CASE(tutorial_overlay_starts_hidden) {
    TutorialOverlay t;
    CHECK(!t.visible());
    CHECK(!t.completed());
}

TEST_CASE(tutorial_overlay_start_makes_visible_at_mission_1) {
    TutorialOverlay t;
    t.start();
    CHECK(t.visible());
    CHECK_EQ(t.currentMission(), 1);
}

TEST_CASE(tutorial_overlay_advance_increments_mission) {
    TutorialOverlay t;
    t.start();
    t.advance();
    CHECK_EQ(t.currentMission(), 2);
    t.advance();
    CHECK_EQ(t.currentMission(), 3);
}

TEST_CASE(tutorial_overlay_completes_after_5_advances) {
    TutorialOverlay t;
    t.start();
    for (int i = 0; i < 5; ++i) t.advance();
    CHECK(!t.visible());
    CHECK(t.completed());
}

TEST_CASE(tutorial_overlay_skip_marks_completed) {
    TutorialOverlay t;
    t.start();
    t.skip();
    CHECK(!t.visible());
    CHECK(t.completed());
}

TEST_CASE(tutorial_overlay_skip_callback_fires) {
    TutorialOverlay t;
    t.start();
    bool fired = false;
    t.setSkipCallback([&]() { fired = true; });
    t.skip();
    CHECK(fired);
}
