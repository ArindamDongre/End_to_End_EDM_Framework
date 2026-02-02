CXX = g++
CC  = gcc

CXXFLAGS = -std=c++17 -Wall -g
CFLAGS   = -Wall -g

# -------------------------------
# Paths
# -------------------------------
CORE_DIR     = src/core
SHELL_DIR    = src/shell/src
COMPILER_DIR = src/compiler

# -------------------------------
# Output binary
# -------------------------------
TARGET = edm_shell

# -------------------------------
# Core C source files
# -------------------------------
CORE_SRC = \
    src/core/program.c \
    src/core/compiler.c \
    src/core/ir.c \
    src/core/ast.c \
    src/core/irgen.c \
    src/core/semantic.c \
    src/vm/vm_ir.c

# -------------------------------
# Compiler (Flex/Bison)
# -------------------------------
PARSER_SRC = \
    $(COMPILER_DIR)/parser_driver.c \
    $(COMPILER_DIR)/parser.tab.c \
    $(COMPILER_DIR)/lex.yy.c

# -------------------------------
# Shell C++ source files (Person1)
# -------------------------------
SHELL_SRC = \
    $(SHELL_DIR)/main.cpp \
    $(SHELL_DIR)/builtins.cpp \
    $(SHELL_DIR)/executor.cpp \
    $(SHELL_DIR)/parser.cpp

# -------------------------------
# Object files
# -------------------------------
CORE_OBJ   = $(CORE_SRC:.c=.o)
PARSER_OBJ = $(PARSER_SRC:.c=.o)
SHELL_OBJ  = $(SHELL_SRC:.cpp=.o)

# -------------------------------
# Default target
# -------------------------------
all: $(TARGET)

# -------------------------------
# Build final executable
# -------------------------------
$(TARGET): $(CORE_OBJ) $(PARSER_OBJ) $(SHELL_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# -------------------------------
# Flex/Bison rules
# -------------------------------
$(COMPILER_DIR)/parser.tab.c $(COMPILER_DIR)/parser.tab.h: $(COMPILER_DIR)/parser.y
	bison -d -o $(COMPILER_DIR)/parser.tab.c $(COMPILER_DIR)/parser.y

$(COMPILER_DIR)/lex.yy.c: $(COMPILER_DIR)/lexer.l $(COMPILER_DIR)/parser.tab.h
	flex -o $(COMPILER_DIR)/lex.yy.c $(COMPILER_DIR)/lexer.l

# -------------------------------
# Compile C core files
# -------------------------------
%.o: %.c
	$(CC) $(CFLAGS) -I$(CORE_DIR) -I$(COMPILER_DIR) -Isrc/vm -c $< -o $@

# -------------------------------
# Compile C++ shell files
# -------------------------------
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -Isrc/shell/include -I$(CORE_DIR) -c $< -o $@

# -------------------------------
# Clean
# -------------------------------
clean:
	rm -f $(TARGET)
	rm -f $(CORE_DIR)/*.o
	rm -f $(COMPILER_DIR)/*.o
	rm -f $(SHELL_DIR)/*.o
	rm -f $(COMPILER_DIR)/parser.tab.*
	rm -f $(COMPILER_DIR)/lex.yy.c

.PHONY: all clean
