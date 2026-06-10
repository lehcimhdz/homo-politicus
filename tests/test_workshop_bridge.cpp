#include "TestFramework.hpp"
#include "steam/WorkshopBridge.hpp"

TEST_CASE(workshop_init_works) {
    CHECK(WorkshopBridge::init());
    WorkshopBridge::shutdown();
}

TEST_CASE(workshop_subscribe_offline_returns_false) {
    WorkshopBridge::init();
    bool r = WorkshopBridge::subscribe("123456789");
    CHECK(!r); // sin SDK
    WorkshopBridge::shutdown();
}

TEST_CASE(workshop_subscribed_items_empty_offline) {
    WorkshopBridge::init();
    auto items = WorkshopBridge::subscribedItems();
    CHECK_EQ(items.size(), (size_t)0);
    WorkshopBridge::shutdown();
}

TEST_CASE(workshop_upload_offline_fails_gracefully) {
    WorkshopBridge::init();
    bool r = WorkshopBridge::uploadMod("/tmp/some_mod", "Test Mod");
    CHECK(!r);
    WorkshopBridge::shutdown();
}
