CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Iinclude
SRC = src/main.cpp src/Game.cpp src/Country.cpp
TARGET = HomoPoliticus

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
