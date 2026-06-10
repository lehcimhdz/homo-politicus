#ifndef WORKSHOP_BRIDGE_HPP
#define WORKSHOP_BRIDGE_HPP

#include <string>
#include <vector>

// WorkshopBridge: integración con Steam Workshop.
// Sin SDK: modo offline lista mods locales en el directorio Workshop simulado.
namespace WorkshopBridge {

    struct WorkshopItem {
        std::string id;          // PublishedFileId_t
        std::string title;
        std::string author;
        std::string localPath;   // path tras subscribirse
        int subscribers = 0;
        float rating = 0.f;
    };

    bool init();
    void shutdown();

    // Subscribe a un mod. Sin SDK: no-op.
    bool subscribe(const std::string& itemId);
    bool unsubscribe(const std::string& itemId);

    // Lista mods subscritos del usuario.
    std::vector<WorkshopItem> subscribedItems();

    // Upload mod local al Workshop. Requiere SDK.
    bool uploadMod(const std::string& localPath, const std::string& title);
}

#endif
