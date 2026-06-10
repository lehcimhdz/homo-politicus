#include "TestFramework.hpp"
#include "DecisionSystem.hpp"

TEST_CASE(decisions_empty_queue_starts_false) {
    std::vector<PendingDecision> q;
    CHECK(DecisionSystem::hasDecision(q, "coup_threat") == false);
}

TEST_CASE(decisions_enqueue_once_only_adds_one) {
    std::vector<PendingDecision> q;
    PendingDecision d{"coup_threat", "prompt", {"a","b"}};
    DecisionSystem::enqueueOnce(q, d);
    DecisionSystem::enqueueOnce(q, d);
    DecisionSystem::enqueueOnce(q, d);
    CHECK_EQ(q.size(), (size_t)1);
}

TEST_CASE(decisions_has_detects_id) {
    std::vector<PendingDecision> q;
    q.push_back({"big_scandal", "p", {"x"}});
    q.push_back({"nuclear_threat", "p", {"y"}});
    CHECK(DecisionSystem::hasDecision(q, "big_scandal"));
    CHECK(DecisionSystem::hasDecision(q, "nuclear_threat"));
    CHECK(DecisionSystem::hasDecision(q, "unknown_id") == false);
}

TEST_CASE(decisions_skip_to_back_rotates) {
    std::vector<PendingDecision> q;
    q.push_back({"a", "p", {}});
    q.push_back({"b", "p", {}});
    q.push_back({"c", "p", {}});
    DecisionSystem::skipToBack(q);
    CHECK_EQ(q.size(), (size_t)3);
    CHECK(q[0].id == "b");
    CHECK(q[1].id == "c");
    CHECK(q[2].id == "a");
}

TEST_CASE(decisions_skip_to_back_on_empty_is_safe) {
    std::vector<PendingDecision> q;
    DecisionSystem::skipToBack(q);
    CHECK_EQ(q.size(), (size_t)0);
}
