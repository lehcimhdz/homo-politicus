#include "DecisionSystem.hpp"

namespace DecisionSystem {

bool hasDecision(const std::vector<PendingDecision>& queue, const std::string& id) {
    for (const auto& d : queue) if (d.id == id) return true;
    return false;
}

void enqueueOnce(std::vector<PendingDecision>& queue, const PendingDecision& d) {
    if (!hasDecision(queue, d.id)) queue.push_back(d);
}

void skipToBack(std::vector<PendingDecision>& queue) {
    if (queue.empty()) return;
    PendingDecision d = queue.front();
    queue.erase(queue.begin());
    queue.push_back(d);
}

} // namespace DecisionSystem
