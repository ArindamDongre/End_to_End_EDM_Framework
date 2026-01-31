CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

# Output binary
TARGET = mysh

# Source files
SHELL_SRC = \
    src/shell/src/main.cpp \
    src/shell/src/parser.cpp \
    src/shell/src/executor.cpp \
    src/shell/src/builtins.cpp

CORE_SRC = \
    src/core/program.c

# Object files
OBJ = $(SHELL_SRC:.cpp=.o) $(CORE_SRC:.c=.o)

# Include paths
INCLUDES = \
    -Isrc/shell/include \
    -Isrc/core

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# C++ files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# C files
%.o: %.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: all
	./$(TARGET)
