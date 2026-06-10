#ifndef STEAM_BRIDGE_HPP
#define STEAM_BRIDGE_HPP

#include <string>

// SteamBridge: interfaz unica para Steamworks. Implementacion mock por defecto
// (lo necesario para CI / dev sin SDK). Cuando se enlaza con steam_api.dylib,
// las llamadas se delegan al SDK real. Activa con HP_STEAM_ENABLED=1 al compilar.
namespace SteamBridge {
    bool init();                                        // init SDK; false si offline
    void shutdown();
    void unlockAchievement(const std::string& id);      // setAchievement + StoreStats
    bool isAchievementUnlocked(const std::string& id);
    void setRichPresence(const std::string& key, const std::string& value);
    void cloudSync(const std::string& localPath);       // upload save
    void cloudFetch(const std::string& localPath);      // download save
    bool isAvailable();                                  // SDK linkeado y user logueado
}

#endif
