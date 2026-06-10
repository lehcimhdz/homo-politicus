#ifndef DECISION_SYSTEM_HPP
#define DECISION_SYSTEM_HPP

#include <string>
#include <vector>

struct PendingDecision {
    std::string id;
    std::string prompt;
    std::vector<std::string> options;
};

namespace DecisionSystem {
    bool hasDecision(const std::vector<PendingDecision>& queue, const std::string& id);
    void enqueueOnce(std::vector<PendingDecision>& queue, const PendingDecision& d);
    void skipToBack(std::vector<PendingDecision>& queue);
}

#endif
