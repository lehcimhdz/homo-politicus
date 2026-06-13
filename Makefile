CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# SFML 3.x (instalado via brew)
SFML_PREFIX = $(shell brew --prefix sfml 2>/dev/null || echo /opt/homebrew)
SFML_CXXFLAGS = -I$(SFML_PREFIX)/include
SFML_LDFLAGS = -L$(SFML_PREFIX)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
SRC = src/main.cpp src/Game.cpp src/Country.cpp src/EventManager.cpp src/Persistence.cpp src/GameOverChecker.cpp src/DecisionSystem.cpp src/FeedbackBuilder.cpp src/MiniYaml.cpp src/ScenarioLoader.cpp src/AchievementTracker.cpp src/steam/SteamBridge.cpp src/steam/WorkshopBridge.cpp src/LeaderLoader.cpp src/ExpressionEvaluator.cpp src/EventLoader.cpp src/Advisor.cpp src/llm/MockProvider.cpp src/llm/AnthropicProvider.cpp src/GameMode.cpp src/Telemetry.cpp src/SuccessorMode.cpp src/IronManMode.cpp src/ModLoader.cpp src/PerfTracker.cpp src/TelemetryConsent.cpp src/Localization.cpp src/Tutorial.cpp
TARGET = HomoPoliticus
UI_TARGET = HomoPoliticusUI
UI_SRC = src/ui/main_ui.cpp src/ui/UIBridge.cpp src/ui/Dashboard.cpp src/ui/MapView.cpp src/ui/ActionPanel.cpp src/ui/DecisionModal.cpp src/ui/AudioSystem.cpp src/ui/MainMenu.cpp src/ui/GameOverScreen.cpp src/ui/TutorialOverlay.cpp src/ui/CircularGauge.cpp src/ui/ParticleEmitter.cpp src/ui/CountrySilhouette.cpp src/ui/LeaderPortrait.cpp src/ui/MandateTimeline.cpp src/ui/IsoCamera.cpp src/ui/IsoWorld.cpp src/ui/IsoBuilding.cpp src/ui/IsoNpcs.cpp src/ui/IsoVehicles.cpp src/ui/WeatherSystem.cpp src/ui/CourtNetwork.cpp src/ui/CourtScene.cpp src/ui/ProvinceMap.cpp src/ui/Heraldry.cpp src/ui/AssetManager.cpp src/ui/AchievementsView.cpp src/AchievementTracker.cpp src/steam/SteamBridge.cpp src/Persistence.cpp src/GameMode.cpp src/IronManMode.cpp src/SuccessorMode.cpp src/Advisor.cpp src/llm/MockProvider.cpp src/llm/AnthropicProvider.cpp src/ModLoader.cpp src/DecisionSystem.cpp src/Localization.cpp src/MiniYaml.cpp src/Country.cpp src/EventManager.cpp src/Game.cpp src/GameOverChecker.cpp src/FeedbackBuilder.cpp src/ScenarioLoader.cpp src/steam/WorkshopBridge.cpp src/LeaderLoader.cpp src/ExpressionEvaluator.cpp src/EventLoader.cpp src/Telemetry.cpp src/PerfTracker.cpp src/TelemetryConsent.cpp src/Tutorial.cpp

TEST_SRC = src/Country.cpp src/EventManager.cpp src/Persistence.cpp src/GameOverChecker.cpp src/DecisionSystem.cpp src/FeedbackBuilder.cpp src/MiniYaml.cpp src/ScenarioLoader.cpp src/AchievementTracker.cpp src/steam/SteamBridge.cpp src/steam/WorkshopBridge.cpp src/LeaderLoader.cpp src/ExpressionEvaluator.cpp src/EventLoader.cpp src/Advisor.cpp src/llm/MockProvider.cpp src/llm/AnthropicProvider.cpp src/GameMode.cpp src/Telemetry.cpp src/SuccessorMode.cpp src/IronManMode.cpp src/ModLoader.cpp src/PerfTracker.cpp src/TelemetryConsent.cpp src/Localization.cpp src/Tutorial.cpp src/Game.cpp tests/test_main.cpp tests/test_persistence.cpp tests/test_gameover.cpp tests/test_invariants.cpp tests/test_decisions.cpp tests/test_feedback.cpp tests/test_scenario_loader.cpp tests/test_achievements.cpp tests/test_leader_loader.cpp tests/test_expression.cpp tests/test_event_loader.cpp tests/test_advisor.cpp tests/test_miniyaml_list.cpp tests/test_event_loader_file.cpp tests/test_localization.cpp tests/test_persistence_v2.cpp tests/test_achievements_v2.cpp tests/test_tutorial.cpp tests/test_ui_bridge.cpp tests/test_dashboard.cpp tests/test_mapview.cpp tests/test_actionpanel.cpp tests/test_decision_modal.cpp tests/test_audio_system.cpp tests/test_main_menu.cpp tests/test_gameover_screen.cpp tests/test_tutorial_overlay.cpp tests/test_steam_bridge.cpp tests/test_llm_provider.cpp tests/test_game_mode.cpp tests/test_telemetry.cpp tests/test_invariants_extra.cpp tests/test_successor_mode.cpp tests/test_iron_man_mode.cpp tests/test_hotkey_manager.cpp tests/test_mod_loader.cpp tests/test_workshop_bridge.cpp tests/test_locale_parity.cpp tests/test_perf_tracker.cpp tests/test_accessibility.cpp tests/test_telemetry_consent.cpp tests/test_integration_full_game.cpp src/ui/MainMenu.cpp src/ui/AccessibilitySettings.cpp src/ui/HotkeyManager.cpp src/ui/GameOverScreen.cpp src/ui/TutorialOverlay.cpp src/ui/UIBridge.cpp src/ui/Dashboard.cpp src/ui/MapView.cpp src/ui/ActionPanel.cpp src/ui/DecisionModal.cpp src/ui/AudioSystem.cpp src/ui/CircularGauge.cpp src/ui/ParticleEmitter.cpp src/ui/CountrySilhouette.cpp src/ui/LeaderPortrait.cpp src/ui/MandateTimeline.cpp src/ui/IsoCamera.cpp src/ui/IsoWorld.cpp src/ui/IsoBuilding.cpp src/ui/IsoNpcs.cpp src/ui/IsoVehicles.cpp src/ui/WeatherSystem.cpp src/ui/CourtNetwork.cpp src/ui/CourtScene.cpp src/ui/ProvinceMap.cpp src/ui/Heraldry.cpp src/ui/AssetManager.cpp src/ui/AchievementsView.cpp tests/test_iso_camera.cpp tests/test_weather_system.cpp tests/test_achievements_view.cpp
TEST_BIN = tests_bin

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

test: $(TEST_BIN)
	./$(TEST_BIN)

ui: $(UI_TARGET)

$(UI_TARGET): $(UI_SRC)
	$(CXX) $(CXXFLAGS) $(SFML_CXXFLAGS) $(UI_SRC) -o $(UI_TARGET) $(SFML_LDFLAGS)

$(TEST_BIN): $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $(SFML_CXXFLAGS) $(TEST_SRC) -o $(TEST_BIN) $(SFML_LDFLAGS)

clean:
	rm -f $(TARGET) $(TEST_BIN) $(UI_TARGET)
