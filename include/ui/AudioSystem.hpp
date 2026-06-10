#ifndef AUDIO_SYSTEM_HPP
#define AUDIO_SYSTEM_HPP

#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>

// AudioSystem: SFX procedurales (sin necesidad de archivos externos).
// Genera buffers PCM 22050Hz mono usando funciones simples (sine/saw + envelope).
// Sprint 15: 5 SFX core para feedback de UI.
// Sprint C1.7: 3 loops ambientales con cross-fade.
class AudioSystem {
public:
    enum class Ambient { None, Calm, Tension, Crisis };

    AudioSystem();
    void play(const std::string& id);
    void setEnabled(bool e) { enabled_ = e; for (auto& s : ambientSounds_) if (!e) s.second->stop(); }
    bool isEnabled() const { return enabled_; }
    void setVolume(float v) { volume_ = v; for (auto& s : sounds_) s.second->setVolume(v); }
    float volume() const { return volume_; }

    void setAmbient(Ambient a);
    Ambient currentAmbient() const { return target_; }
    void updateAmbient(float dt);

private:
    bool enabled_ = true;
    float volume_ = 70.f;
    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> buffers_;
    std::unordered_map<std::string, std::unique_ptr<sf::Sound>> sounds_;
    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> ambientBuffers_;
    std::unordered_map<std::string, std::unique_ptr<sf::Sound>> ambientSounds_;
    std::unordered_map<std::string, float> ambientCurVol_;
    std::unordered_map<std::string, float> ambientTargetVol_;
    Ambient target_ = Ambient::None;
    float ambientBaseVolume_ = 30.f;

    void registerBeep(const std::string& id, float freqHz, float durationSec, float decay);
    void registerChord(const std::string& id, float f1, float f2, float f3, float durationSec);
    void registerAmbient(const std::string& id, Ambient type, float durationSec = 20.f);
};

#endif
