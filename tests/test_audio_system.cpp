#include "TestFramework.hpp"
#include "ui/AudioSystem.hpp"

TEST_CASE(audio_system_starts_enabled) {
    AudioSystem a;
    CHECK(a.isEnabled());
}

TEST_CASE(audio_system_toggle_works) {
    AudioSystem a;
    a.setEnabled(false);
    CHECK(!a.isEnabled());
    a.setEnabled(true);
    CHECK(a.isEnabled());
}

TEST_CASE(audio_system_volume_default_70) {
    AudioSystem a;
    CHECK(a.volume() == 70.f);
}

TEST_CASE(audio_system_set_volume) {
    AudioSystem a;
    a.setVolume(50.f);
    CHECK(a.volume() == 50.f);
}

TEST_CASE(audio_system_play_unknown_does_not_crash) {
    AudioSystem a;
    a.play("___nonexistent___");
    CHECK(true);
}
