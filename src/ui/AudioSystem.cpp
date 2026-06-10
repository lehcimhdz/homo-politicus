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
    registerAmbient("ambient_calm",    Ambient::Calm);
    registerAmbient("ambient_tension", Ambient::Tension);
    registerAmbient("ambient_crisis",  Ambient::Crisis);
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

void AudioSystem::registerAmbient(const std::string& id, Ambient type, float durationSec) {
    size_t n = (size_t)(kSampleRate * durationSec);
    std::vector<std::int16_t> samples(n);
    const float twoPi = 2.0f * 3.14159265f;
    for (size_t i = 0; i < n; ++i) {
        float t = (float)i / (float)kSampleRate;
        float mix = 0.f;
        if (type == Ambient::Calm) {
            // Pad C-mayor (C3, E3, G3, C4) sostenido con leve vibrato.
            float vib = 1.f + 0.003f * std::sin(twoPi * 0.25f * t);
            mix += std::sin(twoPi * 130.81f * t * vib) * 0.35f;
            mix += std::sin(twoPi * 164.81f * t * vib) * 0.28f;
            mix += std::sin(twoPi * 196.00f * t * vib) * 0.22f;
            mix += std::sin(twoPi * 261.63f * t * vib) * 0.18f;
        } else if (type == Ambient::Tension) {
            // Pad disonante (C, Eb, F#, Bb) + percusion sutil cada 2s.
            mix += std::sin(twoPi * 130.81f * t) * 0.30f;
            mix += std::sin(twoPi * 155.56f * t) * 0.26f;
            mix += std::sin(twoPi * 185.00f * t) * 0.22f;
            mix += std::sin(twoPi * 233.08f * t) * 0.18f;
            // Pulso de percusion cada 2s, sustain 80ms.
            float beatT = std::fmod(t, 2.0f);
            if (beatT < 0.08f) {
                float env = 1.f - (beatT / 0.08f);
                mix += std::sin(twoPi * 60.f * t) * env * 0.5f;
            }
        } else { // Crisis
            // Drones graves + interferencia (noise modulado).
            mix += std::sin(twoPi * 55.f  * t) * 0.40f;
            mix += std::sin(twoPi * 82.4f * t) * 0.30f;
            mix += std::sin(twoPi * 110.f * t) * 0.20f;
            // Pseudo-noise determinista para interferencia.
            unsigned seed = (unsigned)(i * 1664525u + 1013904223u);
            float noise = ((float)((seed >> 8) & 0xFFFF) / 32768.f - 1.f);
            mix += noise * 0.12f * (0.5f + 0.5f * std::sin(twoPi * 0.3f * t));
        }
        // Normalizar al rango [-1,1] y aplicar volumen master del buffer.
        if (mix > 1.f) mix = 1.f;
        if (mix < -1.f) mix = -1.f;
        samples[i] = (std::int16_t)(mix * 16000.f);
    }
    auto buf = std::make_unique<sf::SoundBuffer>();
    if (!buf->loadFromSamples(samples.data(), samples.size(), 1, kSampleRate, {sf::SoundChannel::Mono})) return;
    auto snd = std::make_unique<sf::Sound>(*buf);
    snd->setLooping(true);
    snd->setVolume(0.f);
    ambientBuffers_[id] = std::move(buf);
    ambientSounds_[id]  = std::move(snd);
    ambientCurVol_[id]    = 0.f;
    ambientTargetVol_[id] = 0.f;
    (void)type;
}

void AudioSystem::setAmbient(Ambient a) {
    target_ = a;
    auto setKey = [&](const std::string& key, float v) {
        ambientTargetVol_[key] = v;
        auto it = ambientSounds_.find(key);
        if (it != ambientSounds_.end() && enabled_ && v > 0.f && it->second->getStatus() != sf::Sound::Status::Playing) {
            it->second->play();
        }
    };
    setKey("ambient_calm",    a == Ambient::Calm    ? 1.f : 0.f);
    setKey("ambient_tension", a == Ambient::Tension ? 1.f : 0.f);
    setKey("ambient_crisis",  a == Ambient::Crisis  ? 1.f : 0.f);
}

void AudioSystem::updateAmbient(float dt) {
    if (!enabled_) return;
    for (auto& kv : ambientSounds_) {
        const std::string& id = kv.first;
        float cur = ambientCurVol_[id];
        float tgt = ambientTargetVol_[id];
        float diff = tgt - cur;
        float step = dt * 0.8f; // ~1.25s para completar fade
        if (std::abs(diff) <= step) cur = tgt;
        else cur += (diff > 0 ? step : -step);
        ambientCurVol_[id] = cur;
        kv.second->setVolume(cur * ambientBaseVolume_);
        if (cur <= 0.001f && tgt <= 0.001f && kv.second->getStatus() == sf::Sound::Status::Playing) {
            kv.second->stop();
        }
    }
}
