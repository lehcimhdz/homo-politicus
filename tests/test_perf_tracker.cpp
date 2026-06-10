#include "TestFramework.hpp"
#include "PerfTracker.hpp"
#include <thread>
#include <chrono>

TEST_CASE(perf_tracker_records_scope_duration) {
    PerfTracker::instance().reset();
    {
        PerfTracker::Scope s("test_section");
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    auto snap = PerfTracker::instance().snapshot();
    CHECK(snap.count("test_section") == 1);
    CHECK(snap["test_section"] > 1000); // >1ms
}

TEST_CASE(perf_tracker_accumulates_multiple_records) {
    PerfTracker::instance().reset();
    PerfTracker::instance().record("op", 100.0);
    PerfTracker::instance().record("op", 200.0);
    auto snap = PerfTracker::instance().snapshot();
    CHECK_NEAR(snap["op"], 300.0, 0.001);
}

TEST_CASE(perf_tracker_reset_clears_all) {
    PerfTracker::instance().record("op", 50.0);
    PerfTracker::instance().reset();
    auto snap = PerfTracker::instance().snapshot();
    CHECK_EQ(snap.size(), (size_t)0);
}
