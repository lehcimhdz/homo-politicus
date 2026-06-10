#include "TestFramework.hpp"
#include "ui/ActionPanel.hpp"

TEST_CASE(actionpanel_has_at_least_5_families) {
    ActionPanel ap;
    CHECK(ap.families().size() >= 5);
}

TEST_CASE(actionpanel_each_family_has_actions) {
    ActionPanel ap;
    for (const auto& f : ap.families()) {
        CHECK(!f.actions.empty());
        for (const auto& a : f.actions) {
            CHECK(!a.id.empty());
            CHECK(!a.label.empty());
        }
    }
}

TEST_CASE(actionpanel_callback_fires_on_button_click) {
    ActionPanel ap;
    std::string captured;
    ap.setCallback([&](const std::string& id) { captured = id; });
    ap.setCurrentFamily(0);
    // Primer boton de la primera familia esta en (218+12, 110+36+20) = (230, 166)
    ap.onClick({250.f, 180.f});
    CHECK(!captured.empty());
}

TEST_CASE(actionpanel_tab_click_switches_family) {
    ActionPanel ap;
    // 5 familias en kPanelW=810. tabW=162. Tab idx 2 inicia en x=218+2*162=542.
    ap.onClick({600.f, 120.f});
    CHECK_EQ(ap.currentFamily(), 2);
}

TEST_CASE(actionpanel_hover_sets_tooltip) {
    ActionPanel ap;
    ap.setCurrentFamily(0);
    ap.onMouseMove({250.f, 180.f});
    CHECK(!ap.hoveredActionId().empty());
    CHECK(!ap.hoveredTooltip().empty());
}
