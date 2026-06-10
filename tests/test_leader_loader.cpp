#include "TestFramework.hpp"
#include "LeaderLoader.hpp"
#include "Country.hpp"
#include <fstream>
#include <cstdio>

static const char* TMP = "/tmp/hp_test_leader.yaml";
static void write(const std::string& s) { std::ofstream f(TMP); f << s; }

TEST_CASE(leader_applies_signed_modifiers) {
    write(
        "id: test_leader\n"
        "name_es: Test\n"
        "country_origin: Nowhere\n"
        "starting_modifiers:\n"
        "  popularity: +0.20\n"
        "  press_freedom: -0.30\n"
        "  union_strength: +0.10\n"
    );
    Country c;
    double pop0 = c.politics.popularity;
    double press0 = c.security.press_freedom;
    double union0 = c.welfare.union_strength;
    LeaderLoader::Metadata meta;
    CHECK(LeaderLoader::load(TMP, c, meta));
    CHECK_NEAR(c.politics.popularity, pop0 + 0.20, 0.001);
    CHECK_NEAR(c.security.press_freedom, press0 - 0.30, 0.001);
    CHECK_NEAR(c.welfare.union_strength, union0 + 0.10, 0.001);
    std::remove(TMP);
}

TEST_CASE(leader_clamps_to_unit_range) {
    write(
        "id: x\n"
        "name_es: X\n"
        "starting_modifiers:\n"
        "  popularity: +5.0\n"   // debería clampear a 1.0
        "  diplomatic_prestige: -5.0\n"
    );
    Country c;
    LeaderLoader::Metadata meta;
    LeaderLoader::load(TMP, c, meta);
    CHECK(c.politics.popularity <= 1.0);
    CHECK(c.security.diplomatic_prestige >= 0.0);
    std::remove(TMP);
}

TEST_CASE(leader_unknown_field_ignored) {
    write(
        "id: x\n"
        "name_es: X\n"
        "starting_modifiers:\n"
        "  popularity: +0.10\n"
        "  oil_dependency_modifier: +0.99\n"  // no existe en Country
    );
    Country c;
    double pop0 = c.politics.popularity;
    LeaderLoader::Metadata meta;
    CHECK(LeaderLoader::load(TMP, c, meta));
    CHECK_NEAR(c.politics.popularity, pop0 + 0.10, 0.001);
    std::remove(TMP);
}

TEST_CASE(leader_missing_file_returns_false) {
    Country c;
    LeaderLoader::Metadata meta;
    CHECK(LeaderLoader::load("/tmp/__no_such_file_hp_leader__", c, meta) == false);
}
