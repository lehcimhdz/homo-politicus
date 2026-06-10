#ifndef HOTKEY_MANAGER_HPP
#define HOTKEY_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <functional>

// HotkeyManager: registra atajos de teclado configurables.
// La UI consulta este mapa para mapear keys → comandos.
class HotkeyManager {
public:
    HotkeyManager();
    // Registra binding key→commandId
    void bind(const std::string& key, const std::string& commandId);
    void unbind(const std::string& key);
    std::string commandFor(const std::string& key) const;
    bool isBound(const std::string& key) const;
    const std::unordered_map<std::string, std::string>& all() const { return bindings_; }
    // Restaurar defaults
    void resetDefaults();

private:
    std::unordered_map<std::string, std::string> bindings_;
};

#endif
