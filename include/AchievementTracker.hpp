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

    // History tracking: actualizado cada turno (antes de evaluate)
    struct History {
        double min_popularity = 1.0;
        double max_inflation = 0.0;
        double max_nuclear_attack_prob = 0.0;
        int    consecutive_low_inflation_turns = 0;
        int    consecutive_high_cohesion_turns = 0;
        int    consecutive_no_war_turns = 0;
        int    scandals_endured = 0;       // veces que un escándalo apareció
        int    apologize_count = 0;
        int    consecutive_successful_coverups = 0;
        int    critical_decisions_resolved = 0;
        int    fta_count = 0;
        bool   referendum_won = false;
        bool   civil_war_ended_by_negotiation = false;
    };

    void recordHistory(const Country& c);            // llamar pre-evaluate cada turno
    void noteScandalRevealed();
    void noteApologize();
    void noteCoverUpSuccess();
    void noteCoverUpFail();
    void noteCriticalDecisionResolved();
    void noteFTASigned();
    void noteReferendumWon();
    void noteCivilWarPeacefulEnd();

    void evaluate(const Country& c, int turn, EndCondition end, double initial_gdp);
    bool isUnlocked(const std::string& id) const;
    std::vector<std::string> unlockedList() const;
    static const std::vector<Definition>& catalog();

    std::string serialize() const;
    void deserialize(const std::string& line);

private:
    std::unordered_set<std::string> unlocked;
    History history;
    void tryUnlock(const std::string& id);
};

#endif
