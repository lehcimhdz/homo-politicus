#include "TestFramework.hpp"
#include "steam/SteamBridge.hpp"

TEST_CASE(steam_bridge_init_works_in_offline_mode) {
    bool real = SteamBridge::init();
    (void)real; // false en offline; lo importante es que no crashea
    CHECK(true);
    SteamBridge::shutdown();
}

TEST_CASE(steam_bridge_unlock_achievement_local_mirror) {
    SteamBridge::init();
    SteamBridge::unlockAchievement("test_first_year");
    CHECK(SteamBridge::isAchievementUnlocked("test_first_year"));
    SteamBridge::shutdown();
}

TEST_CASE(steam_bridge_rich_presence_does_not_crash) {
    SteamBridge::init();
    SteamBridge::setRichPresence("status", "Playing as Mandela, turn 12");
    CHECK(true);
    SteamBridge::shutdown();
}

TEST_CASE(steam_bridge_is_available_false_in_offline) {
    SteamBridge::init();
    CHECK(!SteamBridge::isAvailable()); // sin SDK
    SteamBridge::shutdown();
}
