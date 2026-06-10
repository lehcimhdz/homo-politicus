#include "TestFramework.hpp"
#include "ui/GameOverScreen.hpp"
#include "Country.hpp"

TEST_CASE(gameover_screen_starts_hidden) {
    GameOverScreen g;
    CHECK(!g.visible());
}

TEST_CASE(gameover_screen_show_makes_visible) {
    GameOverScreen g;
    Country c;
    g.show(EndCondition::COUP_SUCCESS, c, 12, 6.0);
    CHECK(g.visible());
}

TEST_CASE(gameover_screen_hide_works) {
    GameOverScreen g;
    Country c;
    g.show(EndCondition::REVOLUTION, c, 5, 1.0);
    g.hide();
    CHECK(!g.visible());
}

TEST_CASE(gameover_screen_score_is_positive_for_term_completed) {
    GameOverScreen g;
    Country c;
    c.politics.popularity = 0.65;
    c.politics.regime_legitimacy = 0.7;
    g.show(EndCondition::TERM_COMPLETED, c, 16, 0.65 * 16);
    CHECK(g.score() > 0);
}

TEST_CASE(gameover_screen_label_matches_condition) {
    GameOverScreen g;
    Country c;
    g.show(EndCondition::NUCLEAR_ANNIHILATION, c, 3, 1.0);
    CHECK(g.conditionLabel().find("NUCLEAR") != std::string::npos);
}

TEST_CASE(gameover_screen_click_new_game_callback) {
    GameOverScreen g;
    Country c;
    g.show(EndCondition::COUP_SUCCESS, c, 5, 2.0);
    GameOverScreen::Action got = GameOverScreen::Action::None;
    g.setCallback([&](GameOverScreen::Action a) { got = a; });
    // Botones en y = kPanelY + kPanelH - 80 = 100 + 600 - 80 = 620
    // kPanelX = (1280-800)/2 = 240. Boton 1 (Nueva): x=240+80=320..600.
    g.onClick({400.f, 640.f});
    CHECK(got == GameOverScreen::Action::NewGame);
}

TEST_CASE(gameover_screen_click_main_menu_callback) {
    GameOverScreen g;
    Country c;
    g.show(EndCondition::COUP_SUCCESS, c, 5, 2.0);
    GameOverScreen::Action got = GameOverScreen::Action::None;
    g.setCallback([&](GameOverScreen::Action a) { got = a; });
    // Boton 2 (Menu): x = 240 + 800 - 80 - 280 = 680..960
    g.onClick({800.f, 640.f});
    CHECK(got == GameOverScreen::Action::MainMenu);
}
