#include "TestFramework.hpp"
#include "ScenarioLoader.hpp"
#include "MiniYaml.hpp"
#include "Country.hpp"
#include <fstream>
#include <cstdio>

static const char* TMP_YAML = "/tmp/hp_test_scenario.yaml";

static void writeTmpYaml(const std::string& content) {
    std::ofstream f(TMP_YAML);
    f << content;
}

TEST_CASE(miniyaml_parses_flat_keys) {
    writeTmpYaml(
        "id: test_one\n"
        "name_es: Hola\n"
        "start_year: 1976\n"
    );
    MiniYaml::KV kv;
    CHECK(MiniYaml::loadFile(TMP_YAML, kv));
    CHECK(kv["id"] == "test_one");
    CHECK(kv["name_es"] == "Hola");
    CHECK(kv["start_year"] == "1976");
    std::remove(TMP_YAML);
}

TEST_CASE(miniyaml_parses_nested_blocks) {
    writeTmpYaml(
        "initial_country:\n"
        "  welfare:\n"
        "    population: 1900000\n"
        "    literacy_rate: 0.5\n"
        "  economy:\n"
        "    gdp: 974000000\n"
    );
    MiniYaml::KV kv;
    CHECK(MiniYaml::loadFile(TMP_YAML, kv));
    CHECK(kv["initial_country.welfare.population"] == "1900000");
    CHECK(kv["initial_country.welfare.literacy_rate"] == "0.5");
    CHECK(kv["initial_country.economy.gdp"] == "974000000");
    std::remove(TMP_YAML);
}

TEST_CASE(miniyaml_skips_comments_and_blanks) {
    writeTmpYaml(
        "# comentario\n"
        "\n"
        "id: with_comments\n"
        "  # otro comentario indentado\n"
    );
    MiniYaml::KV kv;
    CHECK(MiniYaml::loadFile(TMP_YAML, kv));
    CHECK(kv["id"] == "with_comments");
    std::remove(TMP_YAML);
}

TEST_CASE(scenario_loader_applies_initial_country) {
    writeTmpYaml(
        "id: test_scenario\n"
        "name_es: Test\n"
        "difficulty: hard\n"
        "start_year: 1973\n"
        "initial_country:\n"
        "  welfare:\n"
        "    population: 10100000\n"
        "    literacy_rate: 0.91\n"
        "  economy:\n"
        "    gdp: 18000000000\n"
        "    inflation: 6.06\n"
        "  politics:\n"
        "    popularity: 0.43\n"
        "    polarization_index: 0.92\n"
    );
    Country c;
    ScenarioLoader::Metadata meta;
    CHECK(ScenarioLoader::load(TMP_YAML, c, meta));
    CHECK(meta.id == "test_scenario");
    CHECK(meta.name_es == "Test");
    CHECK(meta.start_year == 1973);
    CHECK_EQ(c.welfare.population, 10100000);
    CHECK_NEAR(c.welfare.literacy_rate, 0.91, 0.001);
    CHECK_NEAR(c.economy.gdp, 18000000000.0, 1.0);
    CHECK_NEAR(c.economy.inflation, 6.06, 0.001);
    CHECK_NEAR(c.politics.popularity, 0.43, 0.001);
    CHECK_NEAR(c.politics.polarization_index, 0.92, 0.001);
    std::remove(TMP_YAML);
}

TEST_CASE(scenario_loader_missing_file_returns_false) {
    Country c;
    ScenarioLoader::Metadata meta;
    CHECK(ScenarioLoader::load("/tmp/__no_such_file_hp_scenario__.yaml", c, meta) == false);
}
