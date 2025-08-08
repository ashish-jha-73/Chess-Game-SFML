# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# Source and output
SRC = src/main.cpp src/game.cpp
TARGET = chessgame

# Default target builds and runs
all: $(TARGET)
	@echo "🚀 Running $(TARGET)..."
	@./$(TARGET)
	rm -f $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)
