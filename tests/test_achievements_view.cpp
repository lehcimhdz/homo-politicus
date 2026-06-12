#include "TestFramework.hpp"
#include "ui/AchievementsView.hpp"
#include "AchievementTracker.hpp"

TEST_CASE(achievements_view_configure_with_empty_tracker) {
    AchievementsView v;
    AchievementTracker tracker;
    v.configure(tracker);
    CHECK(v.totalCount() == (int)AchievementTracker::catalog().size());
    CHECK_EQ(v.unlockedCount(), 0);
}

TEST_CASE(achievements_view_setfilter) {
    AchievementsView v;
    AchievementTracker tracker;
    v.configure(tracker);
    v.setFilter(AchievementsView::Filter::Unlocked);
    CHECK(v.filter() == AchievementsView::Filter::Unlocked);
    v.setFilter(AchievementsView::Filter::Locked);
    CHECK(v.filter() == AchievementsView::Filter::Locked);
}

TEST_CASE(achievements_view_on_mouse_move_doesnt_crash) {
    AchievementsView v;
    AchievementTracker tracker;
    v.configure(tracker);
    v.onMouseMove({100.f, 100.f});
    CHECK(v.hovered() == -1); // hovered se computa en draw
}
