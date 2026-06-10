# Steam Integration

## Modos

- **Offline (default):** `make ui` compila sin SDK. Achievements solo en mirror local.
- **Steam:** `make ui STEAM=1` (requiere SDK descargada manualmente).

## Setup (cuando tengas la cuenta Steam)

1. Crear cuenta Steamworks Partner (\$100 USD one-time).
2. Crear App ID nueva. Anotar.
3. Descargar Steamworks SDK 1.59+ a `third_party/steamworks/sdk/`.
4. Crear archivo `steam_appid.txt` en raíz con el App ID.
5. Modificar Makefile target `ui`:

```make
ifeq ($(STEAM),1)
  CXXFLAGS += -DHP_STEAM_ENABLED -Ithird_party/steamworks/sdk/public
  SFML_LDFLAGS += -Lthird_party/steamworks/sdk/redistributable_bin/osx -lsteam_api
endif
```

6. Definir achievements en Steamworks Partner site (mismos IDs que `AchievementTracker::catalog()`):
   - `first_year`, `decade_in_power`, `historic`, `economic_miracle`, `inflation_tamed`,
     `default_d`, `swf_titan`, `caudillo`, `constitutionalist`, `soft_power`, etc.
7. Cargar imágenes de cada achievement (256x256 PNG).

## Features implementadas

- `SteamBridge::init/shutdown`: lifecycle.
- `SteamBridge::unlockAchievement(id)`: llamado automáticamente desde `AchievementTracker::tryUnlock`.
- `SteamBridge::setRichPresence(key, value)`: ej. "Playing as Mandela, turn 12".
- `SteamBridge::cloudSync/cloudFetch`: sync de `savegame_*.txt`.

## Testing

```bash
make test  # 4 tests cubriendo modo offline
```

Tests reales con SDK requieren Steam abierto y cuenta de developer.
