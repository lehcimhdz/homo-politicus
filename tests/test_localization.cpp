#include "TestFramework.hpp"
#include "Localization.hpp"

static const char* LOCALES_DIR = "/Users/michelcano/Documents/Repositorios/homo-politicus-game/content/locales";

TEST_CASE(localization_loads_es_yaml) {
    CHECK(Localization::load(LOCALES_DIR, "es"));
    CHECK(Localization::currentLanguage() == "es");
}

TEST_CASE(localization_translates_known_key) {
    Localization::load(LOCALES_DIR, "es");
    std::string s = Localization::tr("end_conditions.COUP_SUCCESS");
    CHECK(s.find("Golpe") != std::string::npos || s.find("GOLPE") != std::string::npos);
}

TEST_CASE(localization_missing_key_returns_marker) {
    Localization::load(LOCALES_DIR, "es");
    std::string s = Localization::tr("nonexistent.key.xyz");
    CHECK(s.find("missing") != std::string::npos);
}

TEST_CASE(localization_switch_to_en) {
    CHECK(Localization::load(LOCALES_DIR, "en"));
    CHECK(Localization::currentLanguage() == "en");
    std::string s = Localization::tr("end_conditions.COUP_SUCCESS");
    CHECK(s.find("oup") != std::string::npos);
}

TEST_CASE(localization_unknown_language_returns_false) {
    bool result = Localization::load(LOCALES_DIR, "zz");
    CHECK(!result);
}

TEST_CASE(localization_ui_keys_present) {
    Localization::load(LOCALES_DIR, "es");
    std::string s = Localization::tr("ui.game_over_header");
    CHECK(s.find("missing") == std::string::npos);
}
