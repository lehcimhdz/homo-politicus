#include "TestFramework.hpp"
#include "ui/MainMenu.hpp"

TEST_CASE(mainmenu_no_hover_at_start) {
    MainMenu m;
    CHECK_EQ(m.hoveredIndex(), -1);
}

TEST_CASE(mainmenu_callback_new_game) {
    MainMenu m;
    bool fired = false;
    MainMenu::Action got = MainMenu::Action::None;
    m.setCallback([&](MainMenu::Action a) { fired = true; got = a; });
    // Primer boton en kBtnY0 = 340, kBtnH_ = 56. y range 340..396.
    m.onClick({640.f, 360.f});
    CHECK(fired);
    CHECK(got == MainMenu::Action::NewGame);
}

TEST_CASE(mainmenu_callback_quit) {
    MainMenu m;
    MainMenu::Action got = MainMenu::Action::None;
    m.setCallback([&](MainMenu::Action a) { got = a; });
    // Boton idx 4 (Salir) en y = 340 + 4*(56+14) = 620..676
    m.onClick({640.f, 640.f});
    CHECK(got == MainMenu::Action::Quit);
}

TEST_CASE(mainmenu_hover_sets_index) {
    MainMenu m;
    m.onMouseMove({640.f, 360.f});
    CHECK_EQ(m.hoveredIndex(), 0);
    m.onMouseMove({100.f, 100.f});
    CHECK_EQ(m.hoveredIndex(), -1);
}

TEST_CASE(mainmenu_click_outside_does_nothing) {
    MainMenu m;
    bool fired = false;
    m.setCallback([&](MainMenu::Action) { fired = true; });
    m.onClick({100.f, 100.f});
    CHECK(!fired);
}
