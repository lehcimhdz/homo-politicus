#include "TestFramework.hpp"
#include "Localization.hpp"

static const char* LOCALES = "/Users/michelcano/Documents/Repositorios/homo-politicus-game/content/locales";

TEST_CASE(locale_es_loads) {
    CHECK(Localization::load(LOCALES, "es"));
}

TEST_CASE(locale_en_loads) {
    CHECK(Localization::load(LOCALES, "en"));
}

TEST_CASE(locale_pt_loads) {
    CHECK(Localization::load(LOCALES, "pt"));
}

TEST_CASE(locale_fr_loads) {
    CHECK(Localization::load(LOCALES, "fr"));
}

TEST_CASE(locale_de_loads) {
    CHECK(Localization::load(LOCALES, "de"));
}

TEST_CASE(locale_it_loads) {
    CHECK(Localization::load(LOCALES, "it"));
}

TEST_CASE(locale_es_has_critical_keys) {
    Localization::load(LOCALES, "es");
    CHECK(Localization::tr("ui.welcome").find("missing") == std::string::npos);
    CHECK(Localization::tr("end_conditions.COUP_SUCCESS").find("missing") == std::string::npos);
}

TEST_CASE(locale_en_has_critical_keys) {
    Localization::load(LOCALES, "en");
    CHECK(Localization::tr("ui.welcome").find("missing") == std::string::npos);
    CHECK(Localization::tr("end_conditions.IMPEACHMENT").find("missing") == std::string::npos);
}

TEST_CASE(locale_pt_has_critical_keys) {
    Localization::load(LOCALES, "pt");
    CHECK(Localization::tr("ui.welcome").find("missing") == std::string::npos);
    CHECK(Localization::tr("ui.turn_prefix").find("missing") == std::string::npos);
}
