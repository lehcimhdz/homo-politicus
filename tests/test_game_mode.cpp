#include "TestFramework.hpp"
#include "GameMode.hpp"

TEST_CASE(game_mode_names_distinct) {
    CHECK(GameModeRegistry::name(GameMode::Sandbox) != GameModeRegistry::name(GameMode::Missions));
    CHECK(GameModeRegistry::name(GameMode::Missions) != GameModeRegistry::name(GameMode::Historical));
}

TEST_CASE(game_mode_sandbox_has_no_objectives) {
    auto objs = GameModeRegistry::objectivesFor(GameMode::Sandbox, "");
    CHECK_EQ(objs.size(), (size_t)0);
}

TEST_CASE(game_mode_missions_has_objectives) {
    auto objs = GameModeRegistry::objectivesFor(GameMode::Missions, "");
    CHECK(objs.size() >= 3);
}

TEST_CASE(game_mode_chile_1973_specific_objectives) {
    auto objs = GameModeRegistry::objectivesFor(GameMode::Historical, "chile_1973");
    CHECK(objs.size() >= 2);
    bool found = false;
    for (const auto& o : objs) if (o.id == "survive_sept11") found = true;
    CHECK(found);
}

TEST_CASE(game_mode_unknown_scenario_falls_back) {
    auto objs = GameModeRegistry::objectivesFor(GameMode::Historical, "unknown_xyz");
    CHECK(objs.size() >= 1);
}
