#ifndef TELEMETRY_CONSENT_HPP
#define TELEMETRY_CONSENT_HPP

#include <string>

// TelemetryConsent: gestiona opt-in del usuario.
// Se persiste en ~/.config/HomoPoliticus/telemetry_consent.txt
namespace TelemetryConsent {
    enum class Status { NotAsked, Granted, Denied };

    Status load(const std::string& path);
    bool save(Status s, const std::string& path);
    Status currentStatus();
    void setStatus(Status s);
}

#endif
