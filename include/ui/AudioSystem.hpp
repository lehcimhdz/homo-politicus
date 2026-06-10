#ifndef AUDIO_SYSTEM_HPP
#define AUDIO_SYSTEM_HPP

#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>

// AudioSystem: SFX procedurales (sin necesidad de archivos externos).
// Genera buffers PCM 22050Hz mono usando funciones simples (sine/saw + envelope).
// Sprint 15: 5 SFX core para feedback de UI.
class AudioSystem {
public:
    AudioSystem();
    void play(const std::string& id);
    void setEnabled(bool e) { enabled_ = e; }
    bool isEnabled() const { return enabled_; }
    void setVolume(float v) { volume_ = v; for (auto& s : sounds_) s.second->setVolume(v); }
    float volume() const { return volume_; }

private:
    bool enabled_ = true;
    float volume_ = 70.f;
    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> buffers_;
    std::unordered_map<std::string, std::unique_ptr<sf::Sound>> sounds_;

    void registerBeep(const std::string& id, float freqHz, float durationSec, float decay);
    void registerChord(const std::string& id, float f1, float f2, float f3, float durationSec);
};

#endif
