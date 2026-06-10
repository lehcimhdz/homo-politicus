#include "ui/AudioSystem.hpp"
#include <vector>
#include <cmath>

static constexpr unsigned kSampleRate = 22050;

AudioSystem::AudioSystem() {
    registerBeep("turn_advance",      660.0f, 0.06f, 0.85f);
    registerBeep("button_click",      440.0f, 0.04f, 0.70f);
    registerBeep("warning",           220.0f, 0.18f, 0.40f);
    registerChord("achievement",      523.25f, 659.25f, 783.99f, 0.45f); // C-E-G mayor
    registerChord("decision_appears", 392.0f, 466.16f, 587.33f, 0.35f);  // G-A#-D menor (tensión)
    registerBeep("scandal_revealed",  180.0f, 0.25f, 0.30f);
    registerBeep("game_over",         110.0f, 0.60f, 0.20f);
}

void AudioSystem::registerBeep(const std::string& id, float freqHz, float durationSec, float decay) {
    size_t n = (size_t)(kSampleRate * durationSec);
    std::vector<std::int16_t> samples(n);
    for (size_t i = 0; i < n; ++i) {
        float t = (float)i / (float)kSampleRate;
        float env = std::pow(decay, t * 8.0f); // decay exponencial
        float wave = std::sin(2.0f * 3.14159265f * freqHz * t);
        samples[i] = (std::int16_t)(wave * env * 24000.0f);
    }
    auto buf = std::make_unique<sf::SoundBuffer>();
    if (!buf->loadFromSamples(samples.data(), samples.size(), 1, kSampleRate, {sf::SoundChannel::Mono})) return;
    auto snd = std::make_unique<sf::Sound>(*buf);
    snd->setVolume(volume_);
    buffers_[id] = std::move(buf);
    sounds_[id] = std::move(snd);
}

void AudioSystem::registerChord(const std::string& id, float f1, float f2, float f3, float durationSec) {
    size_t n = (size_t)(kSampleRate * durationSec);
    std::vector<std::int16_t> samples(n);
    for (size_t i = 0; i < n; ++i) {
        float t = (float)i / (float)kSampleRate;
        float env = std::pow(0.55f, t * 5.0f);
        float w1 = std::sin(2.0f * 3.14159265f * f1 * t);
        float w2 = std::sin(2.0f * 3.14159265f * f2 * t);
        float w3 = std::sin(2.0f * 3.14159265f * f3 * t);
        float mix = (w1 + w2 + w3) / 3.0f;
        samples[i] = (std::int16_t)(mix * env * 22000.0f);
    }
    auto buf = std::make_unique<sf::SoundBuffer>();
    if (!buf->loadFromSamples(samples.data(), samples.size(), 1, kSampleRate, {sf::SoundChannel::Mono})) return;
    auto snd = std::make_unique<sf::Sound>(*buf);
    snd->setVolume(volume_);
    buffers_[id] = std::move(buf);
    sounds_[id] = std::move(snd);
}

void AudioSystem::play(const std::string& id) {
    if (!enabled_) return;
    auto it = sounds_.find(id);
    if (it == sounds_.end()) return;
    it->second->play();
}
