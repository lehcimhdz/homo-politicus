CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Iinclude
SRC = src/main.cpp src/Game.cpp src/Country.cpp src/EventManager.cpp src/Persistence.cpp src/GameOverChecker.cpp src/DecisionSystem.cpp src/FeedbackBuilder.cpp src/MiniYaml.cpp src/ScenarioLoader.cpp src/AchievementTracker.cpp src/LeaderLoader.cpp src/ExpressionEvaluator.cpp src/EventLoader.cpp
TARGET = HomoPoliticus

TEST_SRC = src/Country.cpp src/EventManager.cpp src/Persistence.cpp src/GameOverChecker.cpp src/DecisionSystem.cpp src/FeedbackBuilder.cpp src/MiniYaml.cpp src/ScenarioLoader.cpp src/AchievementTracker.cpp src/LeaderLoader.cpp src/ExpressionEvaluator.cpp src/EventLoader.cpp tests/test_main.cpp tests/test_persistence.cpp tests/test_gameover.cpp tests/test_invariants.cpp tests/test_decisions.cpp tests/test_feedback.cpp tests/test_scenario_loader.cpp tests/test_achievements.cpp tests/test_leader_loader.cpp tests/test_expression.cpp tests/test_event_loader.cpp
TEST_BIN = tests_bin

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $(TEST_SRC) -o $(TEST_BIN)

clean:
	rm -f $(TARGET) $(TEST_BIN)
