#include "PerfTracker.hpp"

PerfTracker& PerfTracker::instance() {
    static PerfTracker t;
    return t;
}

void PerfTracker::record(const std::string& label, double microseconds) {
    totals_[label] += microseconds;
}

std::unordered_map<std::string, double> PerfTracker::snapshot() {
    return totals_;
}

void PerfTracker::reset() {
    totals_.clear();
}

PerfTracker::Scope::Scope(const std::string& label)
    : label_(label), start_(std::chrono::high_resolution_clock::now()) {}

PerfTracker::Scope::~Scope() {
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
    PerfTracker::instance().record(label_, (double)us);
}
