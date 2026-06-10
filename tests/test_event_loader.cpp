#include "TestFramework.hpp"
#include "EventLoader.hpp"
#include "Country.hpp"
#include "DecisionSystem.hpp"
#include <random>

TEST_CASE(event_loader_has_five_builtin_events) {
    CHECK_EQ(EventLoader::builtin().size(), (size_t)5);
}

TEST_CASE(event_loader_hyperinflation_fires_when_conditions_met) {
    Country c;
    c.economy.inflation = 0.6;
    c.economy.monetary_emission = 0.4;
    std::vector<ScriptedEvent> events = EventLoader::builtin();
    std::vector<PendingDecision> queue;
    std::mt19937 rng(1);
    for (auto& e : events) e.prob_per_turn = 1.0;
    double pre_inflation = c.economy.inflation;
    EventLoader::tick(events, c, queue, rng, 5);
    CHECK(c.economy.inflation > pre_inflation);
}

TEST_CASE(event_loader_global_crash_enqueues_decision) {
    Country c;
    c.economy.trade_openness = 0.8;
    std::vector<ScriptedEvent> events = EventLoader::builtin();
    std::vector<PendingDecision> queue;
    std::mt19937 rng(1);
    for (auto& e : events) e.prob_per_turn = 1.0;
    EventLoader::tick(events, c, queue, rng, 5);
    bool has_crash_decision = false;
    for (const auto& d : queue) if (d.id == "global_crash_decision") has_crash_decision = true;
    CHECK(has_crash_decision);
}

TEST_CASE(event_loader_cooldown_blocks_repeat) {
    Country c;
    c.economy.inflation = 0.6;
    c.economy.monetary_emission = 0.4;
    std::vector<ScriptedEvent> events = EventLoader::builtin();
    std::vector<PendingDecision> queue;
    std::mt19937 rng(1);
    for (auto& e : events) e.prob_per_turn = 1.0;
    EventLoader::tick(events, c, queue, rng, 5);
    double after_first = c.economy.inflation;
    EventLoader::tick(events, c, queue, rng, 6); // still within cooldown
    CHECK_NEAR(c.economy.inflation, after_first, 0.001);
}
