#ifndef ACHIEVEMENT_TRACKER_HPP
#define ACHIEVEMENT_TRACKER_HPP

#include <string>
#include <unordered_set>
#include <vector>
#include <iostream>
#include "Country.hpp"
#include "EndCondition.hpp"

class AchievementTracker {
public:
    struct Definition {
        std::string id;
        std::string name_es;
        std::string description_es;
    };

    void evaluate(const Country& c, int turn, EndCondition end, double initial_gdp);
    bool isUnlocked(const std::string& id) const;
    std::vector<std::string> unlockedList() const;
    static const std::vector<Definition>& catalog();

    // Persistence (one comma-separated line of ids)
    std::string serialize() const;
    void deserialize(const std::string& line);

private:
    std::unordered_set<std::string> unlocked;
    void tryUnlock(const std::string& id);
};

#endif
