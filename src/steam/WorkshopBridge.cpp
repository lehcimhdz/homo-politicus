#include "steam/WorkshopBridge.hpp"
#include "steam/SteamBridge.hpp"
#include <iostream>

#ifdef HP_STEAM_ENABLED
#include "steam_api.h"
#endif

namespace WorkshopBridge {

static bool g_init = false;

bool init() {
    if (!SteamBridge::isAvailable()) {
        std::cout << "[Workshop] modo offline (sin SDK)" << std::endl;
    }
    g_init = true;
    return true;
}

void shutdown() { g_init = false; }

bool subscribe(const std::string& itemId) {
#ifdef HP_STEAM_ENABLED
    if (SteamBridge::isAvailable()) {
        SteamUGC()->SubscribeItem(std::stoull(itemId));
        return true;
    }
#endif
    std::cout << "[Workshop offline] subscribe " << itemId << std::endl;
    return false;
}

bool unsubscribe(const std::string& itemId) {
#ifdef HP_STEAM_ENABLED
    if (SteamBridge::isAvailable()) {
        SteamUGC()->UnsubscribeItem(std::stoull(itemId));
        return true;
    }
#endif
    (void)itemId;
    return false;
}

std::vector<WorkshopItem> subscribedItems() {
    std::vector<WorkshopItem> out;
#ifdef HP_STEAM_ENABLED
    if (SteamBridge::isAvailable()) {
        auto count = SteamUGC()->GetNumSubscribedItems();
        std::vector<PublishedFileId_t> ids(count);
        SteamUGC()->GetSubscribedItems(ids.data(), count);
        for (auto id : ids) {
            WorkshopItem w;
            w.id = std::to_string(id);
            out.push_back(w);
        }
    }
#endif
    return out;
}

bool uploadMod(const std::string& localPath, const std::string& title) {
#ifdef HP_STEAM_ENABLED
    if (!SteamBridge::isAvailable()) return false;
    // TODO: CreateItem → SetItemContent → SetItemTitle → SubmitItemUpdate
    (void)localPath; (void)title;
    return false;
#else
    std::cout << "[Workshop offline] upload " << title << " from " << localPath << std::endl;
    return false;
#endif
}

}
