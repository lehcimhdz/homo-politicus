#include "IronManMode.hpp"

namespace IronManMode {

static bool g_active = false;

void enable() { g_active = true; }
void disable() { g_active = false; }
bool isActive() { return g_active; }

bool isManualSaveAllowed() { return !g_active; }
bool isManualLoadAllowed() { return !g_active; }

std::string autosavePath() { return "ironman_autosave.txt"; }

}
