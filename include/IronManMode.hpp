#ifndef IRON_MAN_MODE_HPP
#define IRON_MAN_MODE_HPP

#include <string>

// IronManMode: una partida hardcore sin save/load manual.
// Solo se permite autosave silencioso (para crash recovery).
// Logros exclusivos solo desbloqueables aquí.
namespace IronManMode {
    void enable();
    void disable();
    bool isActive();

    // Bloquea save manual cuando está activo
    bool isManualSaveAllowed();
    bool isManualLoadAllowed();

    // Autosave silencioso (solo para crash recovery)
    std::string autosavePath();
}

#endif
