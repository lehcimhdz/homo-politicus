#include "TestFramework.hpp"
#include "IronManMode.hpp"

TEST_CASE(iron_man_disabled_by_default) {
    IronManMode::disable();
    CHECK(!IronManMode::isActive());
}

TEST_CASE(iron_man_enabled_blocks_save) {
    IronManMode::enable();
    CHECK(IronManMode::isActive());
    CHECK(!IronManMode::isManualSaveAllowed());
    CHECK(!IronManMode::isManualLoadAllowed());
    IronManMode::disable();
}

TEST_CASE(iron_man_disabled_allows_save) {
    IronManMode::disable();
    CHECK(IronManMode::isManualSaveAllowed());
    CHECK(IronManMode::isManualLoadAllowed());
}

TEST_CASE(iron_man_autosave_path_constant) {
    CHECK(IronManMode::autosavePath() == "ironman_autosave.txt");
}
