#include "TestFramework.hpp"
#include "ModLoader.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

static void writeMod(const std::string& dir, const std::string& id, const std::string& deps = "") {
    fs::create_directories(dir);
    std::ofstream f(dir + "/mod.yaml");
    f << "id: " << id << "\n";
    f << "name: " << id << " Display\n";
    f << "author: Tester\n";
    f << "version: 1.0\n";
    if (!deps.empty()) f << "dependencies: " << deps << "\n";
}

TEST_CASE(mod_loader_scan_empty_directory) {
    fs::create_directories("/tmp/hp_test_mods_empty");
    auto mods = ModLoader::scan("/tmp/hp_test_mods_empty");
    CHECK_EQ(mods.size(), (size_t)0);
    fs::remove_all("/tmp/hp_test_mods_empty");
}

TEST_CASE(mod_loader_scan_missing_directory) {
    auto mods = ModLoader::scan("/tmp/__never_exists_hp__");
    CHECK_EQ(mods.size(), (size_t)0);
}

TEST_CASE(mod_loader_scan_two_valid_mods) {
    std::string base = "/tmp/hp_test_mods_two";
    fs::remove_all(base);
    writeMod(base + "/argentina_2023", "argentina_2023");
    writeMod(base + "/cuba_modern",    "cuba_modern");
    auto mods = ModLoader::scan(base);
    CHECK_EQ(mods.size(), (size_t)2);
    fs::remove_all(base);
}

TEST_CASE(mod_loader_validates_no_conflicts) {
    std::vector<ModLoader::ModMetadata> mods;
    ModLoader::ModMetadata a; a.id = "a"; mods.push_back(a);
    ModLoader::ModMetadata b; b.id = "b"; mods.push_back(b);
    CHECK(ModLoader::validateNoConflicts(mods));
    ModLoader::ModMetadata c; c.id = "a"; mods.push_back(c); // dup
    CHECK(!ModLoader::validateNoConflicts(mods));
}

TEST_CASE(mod_loader_detects_missing_deps) {
    std::vector<ModLoader::ModMetadata> mods;
    ModLoader::ModMetadata a; a.id = "a"; a.dependencies = {"core"}; mods.push_back(a);
    auto miss = ModLoader::missingDependencies(mods);
    CHECK_EQ(miss.size(), (size_t)1);
    CHECK(miss[0] == "core");
}
