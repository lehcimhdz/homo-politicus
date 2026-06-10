#include "Telemetry.hpp"
#include <fstream>

namespace Telemetry {

static bool g_enabled = false;
static std::unordered_map<std::string, int> g_counters;

void enable(bool e) { g_enabled = e; }
bool isEnabled() { return g_enabled; }

void recordCommand(const std::string& cmdId) {
    if (!g_enabled) return;
    g_counters["cmd:" + cmdId]++;
}

void recordEvent(const std::string& eventId) {
    if (!g_enabled) return;
    g_counters["evt:" + eventId]++;
}

void recordEndCondition(int endCond, int turnCount, int score) {
    if (!g_enabled) return;
    g_counters["end:" + std::to_string(endCond)]++;
    g_counters["turns_total"] += turnCount;
    g_counters["score_total"] += score;
    g_counters["games_total"]++;
}

std::unordered_map<std::string, int> snapshot() { return g_counters; }
void reset() { g_counters.clear(); }

}
