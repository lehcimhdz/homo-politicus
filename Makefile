CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# SFML 3.x (instalado via brew)
SFML_PREFIX = $(shell brew --prefix sfml 2>/dev/null || echo /opt/homebrew)
SFML_CXXFLAGS = -I$(SFML_PREFIX)/include
SFML_LDFLAGS = -L$(SFML_PREFIX)/lib -lsfml-graphics -lsfml-window -lsfml-system
SRC = src/main.cpp src/Game.cpp src/Country.cpp src/EventManager.cpp src/Persistence.cpp src/GameOverChecker.cpp src/DecisionSystem.cpp src/FeedbackBuilder.cpp src/MiniYaml.cpp src/ScenarioLoader.cpp src/AchievementTracker.cpp src/LeaderLoader.cpp src/ExpressionEvaluator.cpp src/EventLoader.cpp src/Advisor.cpp src/Localization.cpp src/Tutorial.cpp
TARGET = HomoPoliticus
UI_TARGET = HomoPoliticusUI
UI_SRC = src/ui/main_ui.cpp src/ui/UIBridge.cpp src/Country.cpp src/EventManager.cpp

TEST_SRC = src/Country.cpp src/EventManager.cpp src/Persistence.cpp src/GameOverChecker.cpp src/DecisionSystem.cpp src/FeedbackBuilder.cpp src/MiniYaml.cpp src/ScenarioLoader.cpp src/AchievementTracker.cpp src/LeaderLoader.cpp src/ExpressionEvaluator.cpp src/EventLoader.cpp src/Advisor.cpp src/Localization.cpp src/Tutorial.cpp tests/test_main.cpp tests/test_persistence.cpp tests/test_gameover.cpp tests/test_invariants.cpp tests/test_decisions.cpp tests/test_feedback.cpp tests/test_scenario_loader.cpp tests/test_achievements.cpp tests/test_leader_loader.cpp tests/test_expression.cpp tests/test_event_loader.cpp tests/test_advisor.cpp tests/test_miniyaml_list.cpp tests/test_event_loader_file.cpp tests/test_localization.cpp tests/test_persistence_v2.cpp tests/test_achievements_v2.cpp tests/test_tutorial.cpp tests/test_ui_bridge.cpp src/ui/UIBridge.cpp
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
	$(CXX) $(CXXFLAGS) $(TEST_SRC) -o $(TEST_BIN)

clean:
	rm -f $(TARGET) $(TEST_BIN) $(UI_TARGET)
