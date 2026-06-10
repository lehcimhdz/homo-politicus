#include "steam/SteamBridge.hpp"
#include <iostream>
#include <unordered_set>
#include <fstream>
#include <filesystem>

#ifdef HP_STEAM_ENABLED
// Cuando se compila con la SDK real (descargada manualmente a third_party/steamworks/):
//   make ui STEAM=1
// El target define HP_STEAM_ENABLED y linkea steam_api.
#include "steam_api.h"
#endif

namespace SteamBridge {

static bool g_init = false;
static std::unordered_set<std::string> g_unlocked; // mirror local de achievements

bool init() {
#ifdef HP_STEAM_ENABLED
    if (!SteamAPI_Init()) {
        std::cerr << "[SteamBridge] SteamAPI_Init falló (Steam abierto? app_id correcto?)" << std::endl;
        return false;
    }
    g_init = true;
    std::cout << "[SteamBridge] inicializado (SDK real)" << std::endl;
    return true;
#else
    std::cout << "[SteamBridge] modo offline (sin SDK)" << std::endl;
    g_init = true;
    return false;
#endif
}

void shutdown() {
    if (!g_init) return;
#ifdef HP_STEAM_ENABLED
    SteamAPI_Shutdown();
#endif
    g_init = false;
}

bool isAvailable() {
#ifdef HP_STEAM_ENABLED
    return g_init && SteamUser() && SteamUser()->BLoggedOn();
#else
    return false;
#endif
}

void unlockAchievement(const std::string& id) {
    if (g_unlocked.count(id)) return;
    g_unlocked.insert(id);
#ifdef HP_STEAM_ENABLED
    if (isAvailable()) {
        SteamUserStats()->SetAchievement(id.c_str());
        SteamUserStats()->StoreStats();
    }
#endif
    std::cout << "[Steam] achievement unlock: " << id << std::endl;
}

bool isAchievementUnlocked(const std::string& id) {
#ifdef HP_STEAM_ENABLED
    if (isAvailable()) {
        bool b = false;
        SteamUserStats()->GetAchievement(id.c_str(), &b);
        return b;
    }
#endif
    return g_unlocked.count(id) > 0;
}

void setRichPresence(const std::string& key, const std::string& value) {
#ifdef HP_STEAM_ENABLED
    if (isAvailable()) SteamFriends()->SetRichPresence(key.c_str(), value.c_str());
#else
    (void)key; (void)value;
#endif
}

void cloudSync(const std::string& localPath) {
#ifdef HP_STEAM_ENABLED
    if (!isAvailable() || !std::filesystem::exists(localPath)) return;
    std::ifstream f(localPath, std::ios::binary | std::ios::ate);
    auto size = f.tellg();
    f.seekg(0);
    std::vector<char> data(size);
    f.read(data.data(), size);
    std::string remote = std::filesystem::path(localPath).filename().string();
    SteamRemoteStorage()->FileWrite(remote.c_str(), data.data(), (std::int32_t)data.size());
#else
    (void)localPath;
#endif
}

void cloudFetch(const std::string& localPath) {
#ifdef HP_STEAM_ENABLED
    if (!isAvailable()) return;
    std::string remote = std::filesystem::path(localPath).filename().string();
    if (!SteamRemoteStorage()->FileExists(remote.c_str())) return;
    auto size = SteamRemoteStorage()->GetFileSize(remote.c_str());
    std::vector<char> data(size);
    SteamRemoteStorage()->FileRead(remote.c_str(), data.data(), size);
    std::ofstream f(localPath, std::ios::binary);
    f.write(data.data(), size);
#else
    (void)localPath;
#endif
}

} // namespace SteamBridge
