#include "TestFramework.hpp"
#include "ui/HotkeyManager.hpp"

TEST_CASE(hotkey_defaults_loaded) {
    HotkeyManager h;
    CHECK(h.commandFor("N") == "next");
    CHECK(h.commandFor("Num1") == "tab_dashboard");
    CHECK(h.commandFor("Escape") == "back");
}

TEST_CASE(hotkey_bind_custom) {
    HotkeyManager h;
    h.bind("F1", "show_help");
    CHECK(h.commandFor("F1") == "show_help");
}

TEST_CASE(hotkey_rebind_overwrites) {
    HotkeyManager h;
    h.bind("N", "custom_action");
    CHECK(h.commandFor("N") == "custom_action");
}

TEST_CASE(hotkey_unbind_removes) {
    HotkeyManager h;
    h.unbind("N");
    CHECK(!h.isBound("N"));
    CHECK(h.commandFor("N").empty());
}

TEST_CASE(hotkey_reset_defaults_restores) {
    HotkeyManager h;
    h.unbind("N");
    h.resetDefaults();
    CHECK(h.commandFor("N") == "next");
}
