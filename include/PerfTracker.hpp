#ifndef PERF_TRACKER_HPP
#define PERF_TRACKER_HPP

#include <chrono>
#include <string>
#include <unordered_map>

// PerfTracker: simple instrumentation para medir hot paths.
// Macro RAII para start/stop timer; agrega tiempos por categoría.
class PerfTracker {
public:
    static PerfTracker& instance();
    void record(const std::string& label, double microseconds);
    std::unordered_map<std::string, double> snapshot();
    void reset();

    class Scope {
    public:
        explicit Scope(const std::string& label);
        ~Scope();
    private:
        std::string label_;
        std::chrono::high_resolution_clock::time_point start_;
    };

private:
    std::unordered_map<std::string, double> totals_;
};

#define PERF_SCOPE(label) PerfTracker::Scope _perf_scope_##__LINE__(label)

#endif
