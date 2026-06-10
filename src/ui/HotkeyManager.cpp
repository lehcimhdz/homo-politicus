#include "ui/HotkeyManager.hpp"

HotkeyManager::HotkeyManager() {
    resetDefaults();
}

void HotkeyManager::resetDefaults() {
    bindings_ = {
        {"N",      "next"},
        {"R",      "reset"},
        {"S",      "save"},
        {"L",      "language"},
        {"M",      "mute"},
        {"T",      "tutorial_restart"},
        {"D",      "demo_decision"},
        {"G",      "demo_gameover"},
        {"Space",  "tutorial_advance"},
        {"Escape", "back"},
        {"Num1",   "tab_dashboard"},
        {"Num2",   "tab_map"},
        {"Num3",   "tab_action"},
        {"Num4",   "tab_decisions"},
        {"Num5",   "tab_achievements"},
    };
}

void HotkeyManager::bind(const std::string& key, const std::string& commandId) {
    bindings_[key] = commandId;
}

void HotkeyManager::unbind(const std::string& key) {
    bindings_.erase(key);
}

std::string HotkeyManager::commandFor(const std::string& key) const {
    auto it = bindings_.find(key);
    return it == bindings_.end() ? "" : it->second;
}

bool HotkeyManager::isBound(const std::string& key) const {
    return bindings_.count(key) > 0;
}
