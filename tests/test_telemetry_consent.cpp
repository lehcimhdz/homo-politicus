#include "TestFramework.hpp"
#include "TelemetryConsent.hpp"
#include <cstdio>

static const char* TMP = "/tmp/hp_test_telemetry_consent.txt";

TEST_CASE(telemetry_consent_default_not_asked) {
    std::remove(TMP);
    TelemetryConsent::setStatus(TelemetryConsent::Status::NotAsked);
    CHECK(TelemetryConsent::currentStatus() == TelemetryConsent::Status::NotAsked);
}

TEST_CASE(telemetry_consent_save_and_load_granted) {
    CHECK(TelemetryConsent::save(TelemetryConsent::Status::Granted, TMP));
    TelemetryConsent::setStatus(TelemetryConsent::Status::NotAsked);
    auto s = TelemetryConsent::load(TMP);
    CHECK(s == TelemetryConsent::Status::Granted);
    std::remove(TMP);
}

TEST_CASE(telemetry_consent_save_and_load_denied) {
    CHECK(TelemetryConsent::save(TelemetryConsent::Status::Denied, TMP));
    auto s = TelemetryConsent::load(TMP);
    CHECK(s == TelemetryConsent::Status::Denied);
    std::remove(TMP);
}

TEST_CASE(telemetry_consent_load_missing_returns_notasked) {
    auto s = TelemetryConsent::load("/tmp/__never_exists_consent_hp__");
    CHECK(s == TelemetryConsent::Status::NotAsked);
}
