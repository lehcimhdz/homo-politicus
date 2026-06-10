#include "TelemetryConsent.hpp"
#include <fstream>

namespace TelemetryConsent {

static Status g_status = Status::NotAsked;

Status load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return Status::NotAsked;
    std::string val;
    f >> val;
    if (val == "granted") g_status = Status::Granted;
    else if (val == "denied") g_status = Status::Denied;
    else g_status = Status::NotAsked;
    return g_status;
}

bool save(Status s, const std::string& path) {
    std::ofstream f(path);
    if (!f) return false;
    if (s == Status::Granted) f << "granted";
    else if (s == Status::Denied) f << "denied";
    else f << "notasked";
    g_status = s;
    return true;
}

Status currentStatus() { return g_status; }
void setStatus(Status s) { g_status = s; }

}
