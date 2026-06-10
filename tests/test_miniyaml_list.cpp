#include "TestFramework.hpp"
#include "MiniYaml.hpp"
#include <fstream>
#include <cstdio>

static const char* TMP = "/tmp/hp_test_list.yaml";
static void write(const std::string& s) { std::ofstream f(TMP); f << s; }

TEST_CASE(miniyaml_list_two_items_basic) {
    write(
        "events:\n"
        "  - id: foo\n"
        "    name_es: Foo Event\n"
        "  - id: bar\n"
        "    name_es: Bar Event\n"
    );
    std::vector<MiniYaml::KV> items;
    CHECK(MiniYaml::loadList(TMP, "events", items));
    CHECK_EQ(items.size(), (size_t)2);
    CHECK(items[0]["id"] == "foo");
    CHECK(items[0]["name_es"] == "Foo Event");
    CHECK(items[1]["id"] == "bar");
    CHECK(items[1]["name_es"] == "Bar Event");
    std::remove(TMP);
}

TEST_CASE(miniyaml_list_with_nested_blocks) {
    write(
        "events:\n"
        "  - id: complex\n"
        "    trigger:\n"
        "      expression: \"country.economy.inflation > 0.5\"\n"
        "    probability_per_turn: 0.05\n"
    );
    std::vector<MiniYaml::KV> items;
    CHECK(MiniYaml::loadList(TMP, "events", items));
    CHECK_EQ(items.size(), (size_t)1);
    CHECK(items[0]["id"] == "complex");
    CHECK(items[0]["trigger.expression"].find("inflation") != std::string::npos);
    CHECK(items[0]["probability_per_turn"] == "0.05");
    std::remove(TMP);
}

TEST_CASE(miniyaml_list_three_items_distinct) {
    write(
        "events:\n"
        "  - id: a\n"
        "    name_es: A\n"
        "  - id: b\n"
        "    name_es: B\n"
        "  - id: c\n"
        "    name_es: C\n"
    );
    std::vector<MiniYaml::KV> items;
    CHECK(MiniYaml::loadList(TMP, "events", items));
    CHECK_EQ(items.size(), (size_t)3);
    CHECK(items[0]["id"] == "a");
    CHECK(items[1]["id"] == "b");
    CHECK(items[2]["id"] == "c");
    std::remove(TMP);
}

TEST_CASE(miniyaml_list_root_key_not_found) {
    write(
        "scenarios:\n"
        "  - id: foo\n"
    );
    std::vector<MiniYaml::KV> items;
    CHECK(MiniYaml::loadList(TMP, "events", items));
    CHECK_EQ(items.size(), (size_t)0);
    std::remove(TMP);
}

TEST_CASE(miniyaml_list_loads_real_economic_yaml) {
    std::vector<MiniYaml::KV> items;
    CHECK(MiniYaml::loadList(
        "/Users/michelcano/Documents/Repositorios/homo-politicus-game/content/events/economic.yaml",
        "events", items));
    CHECK(items.size() >= 5);
    bool found_commodity = false;
    for (const auto& kv : items) {
        auto it = kv.find("id");
        if (it != kv.end() && it->second == "commodity_boom") found_commodity = true;
    }
    CHECK(found_commodity);
}
