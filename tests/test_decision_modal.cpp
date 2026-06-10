#include "TestFramework.hpp"
#include "ui/DecisionModal.hpp"

TEST_CASE(decision_modal_starts_hidden) {
    DecisionModal m;
    CHECK(!m.visible());
}

TEST_CASE(decision_modal_show_makes_visible) {
    DecisionModal m;
    m.show({"test_id", "Prompt?", {"a", "b", "c"}});
    CHECK(m.visible());
}

TEST_CASE(decision_modal_hide_works) {
    DecisionModal m;
    m.show({"x", "p", {"a"}});
    m.hide();
    CHECK(!m.visible());
}

TEST_CASE(decision_modal_choice_callback_fires) {
    DecisionModal m;
    m.show({"x", "p", {"a", "b", "c"}});
    std::string captured;
    m.setChoiceCallback([&](const std::string& c) { captured = c; });
    // kModalY=170, startY=350, primer boton y=350..398.
    m.onClick({640.f, 370.f});
    CHECK(captured == "a");
}

TEST_CASE(decision_modal_skip_callback_fires) {
    DecisionModal m;
    m.show({"x", "p", {"a"}});
    bool skipped = false;
    m.setSkipCallback([&]() { skipped = true; });
    // Skip en y=kModalY+kModalH-60 = 170+460-60 = 570
    m.onClick({640.f, 585.f});
    CHECK(skipped);
}

TEST_CASE(decision_modal_hover_sets_hovered_option) {
    DecisionModal m;
    m.show({"x", "p", {"alpha", "beta"}});
    // Primer boton y=350..398
    m.onMouseMove({640.f, 370.f});
    CHECK(m.hoveredOption() == "alpha");
}
