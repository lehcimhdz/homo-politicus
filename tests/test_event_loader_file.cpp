#include "TestFramework.hpp"
#include "EventLoader.hpp"

static const char* EVENTS_DIR = "/Users/michelcano/Documents/Repositorios/homo-politicus-game/content/events";
static const char* ECONOMIC_YAML = "/Users/michelcano/Documents/Repositorios/homo-politicus-game/content/events/economic.yaml";

TEST_CASE(event_loader_loads_economic_file) {
    auto events = EventLoader::loadFile(ECONOMIC_YAML);
    CHECK(events.size() >= 5);
    bool found = false;
    for (const auto& e : events) {
        if (e.id == "commodity_boom") found = true;
    }
    CHECK(found);
}

TEST_CASE(event_loader_loads_trigger_expression) {
    auto events = EventLoader::loadFile(ECONOMIC_YAML);
    for (const auto& e : events) {
        if (e.id == "hyperinflation") {
            CHECK(e.trigger_expr.find("inflation") != std::string::npos);
            return;
        }
    }
    CHECK(false);
}

TEST_CASE(event_loader_loads_all_directory) {
    auto events = EventLoader::loadDir(EVENTS_DIR);
    CHECK(events.size() >= 25);
}

TEST_CASE(event_loader_missing_directory_returns_empty) {
    auto events = EventLoader::loadDir("/tmp/__no_such_dir_hp_events__");
    CHECK_EQ(events.size(), (size_t)0);
}
