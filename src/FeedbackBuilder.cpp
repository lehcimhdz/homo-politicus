#include "FeedbackBuilder.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>

static std::string fmt_signed_pct(double pct) {
    std::ostringstream oss;
    if (pct >= 0) oss << "+";
    oss << std::fixed << std::setprecision(1) << pct << "%";
    return oss.str();
}

FeedbackBuilder::FeedbackBuilder(const std::string& action_label) : header(">> " + action_label) {}

FeedbackBuilder& FeedbackBuilder::addDelta(const std::string& label, double from, double to, const std::string& unit) {
    std::ostringstream oss;
    double delta = to - from;
    oss << "   " << std::left << std::setw(18) << (label + ":")
        << std::fixed << std::setprecision(unit == "%" ? 0 : 2)
        << (from * (unit == "%" ? 100.0 : 1.0)) << unit << " -> "
        << (to * (unit == "%" ? 100.0 : 1.0)) << unit
        << "   (" << fmt_signed_pct(delta * (unit == "%" ? 100.0 : 1.0)) << ")";
    deltas.push_back(oss.str());
    return *this;
}

FeedbackBuilder& FeedbackBuilder::addAbsoluteDelta(const std::string& label, double from, double to, const std::string& prefix, const std::string& suffix) {
    std::ostringstream oss;
    double delta = to - from;
    double scale = (suffix == "M") ? 1e6 : (suffix == "B") ? 1e9 : 1.0;
    oss << "   " << std::left << std::setw(18) << (label + ":")
        << prefix << (long long)(from / scale) << suffix << " -> "
        << prefix << (long long)(to / scale) << suffix
        << "   (" << (delta >= 0 ? "+" : "")
        << prefix << (long long)(delta / scale) << suffix << ")";
    deltas.push_back(oss.str());
    return *this;
}

FeedbackBuilder& FeedbackBuilder::addNote(const std::string& text) {
    notes.push_back("   " + text);
    return *this;
}

FeedbackBuilder& FeedbackBuilder::addRisk(const std::string& event_name, double prob_increase) {
    if (prob_increase < 0.03) return *this;
    std::ostringstream oss;
    oss << "   - " << event_name << " (+" << (int)(prob_increase * 100) << "%)";
    risks.push_back(oss.str());
    return *this;
}

FeedbackBuilder& FeedbackBuilder::addPrediction(const std::string& text) {
    prediction = "   Proximo turno: " + text;
    return *this;
}

std::string FeedbackBuilder::build() const {
    std::ostringstream out;
    out << header << "\n";
    for (const auto& d : deltas) out << d << "\n";
    for (const auto& n : notes) out << n << "\n";
    if (!risks.empty()) {
        out << "   Riesgos nuevos:\n";
        for (const auto& r : risks) out << r << "\n";
    }
    if (!prediction.empty()) out << prediction << "\n";
    return out.str();
}

void FeedbackBuilder::print() const {
    std::cout << build();
}
