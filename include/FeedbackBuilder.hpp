#ifndef FEEDBACK_BUILDER_HPP
#define FEEDBACK_BUILDER_HPP

#include <string>
#include <vector>
#include <sstream>

class FeedbackBuilder {
public:
    explicit FeedbackBuilder(const std::string& action_label);
    FeedbackBuilder& addDelta(const std::string& label, double from, double to, const std::string& unit = "%");
    FeedbackBuilder& addAbsoluteDelta(const std::string& label, double from, double to, const std::string& prefix = "$", const std::string& suffix = "M");
    FeedbackBuilder& addNote(const std::string& text);
    FeedbackBuilder& addRisk(const std::string& event_name, double prob_increase);
    FeedbackBuilder& addPrediction(const std::string& text);
    std::string build() const;
    void print() const;

private:
    std::string header;
    std::vector<std::string> deltas;
    std::vector<std::string> notes;
    std::vector<std::string> risks;
    std::string prediction;
};

#endif
