# ==========================================
# 1. Variables Definitions
# ==========================================

# The compiler to use (gcc for C)
CC = gcc

# Compiler Flags:
# -Wall     : Enable all standard compiler warnings (helps find bugs).
# -g        : Generate debugging information (for use with gdb/lldb).
# -Iinclude : Look for header files (.h) in the 'include' directory.
CFLAGS = -Wall -g -Iinclude

# Directory paths
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/main

# Source Discovery:
# $(wildcard ...): Automatically finds all .c files in the SRC_DIR.
# Example: src/main.c src/utils.c
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Object Creation:
# $(patsubst ...): Pattern substitution. Replaces the source path and extension.
# It changes 'src/filename.c' to 'build/filename.o'.
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# ==========================================
# 2. Default Rule
# ==========================================

# The first rule is the default (runs when you just type 'make').
# It depends on $(TARGET) to ensure the program is built.
all: $(TARGET)

# ==========================================
# 3. Linker Rule
# ==========================================

# This rule creates the final executable.
# It depends on the object files ($(OBJS)).
$(TARGET): $(OBJS)
	@# Create the build directory if it doesn't exist (-p avoids error if exists)
	@mkdir -p $(BUILD_DIR)
	@# Link all object files to create the executable
	@# $@ is an automatic variable representing the target name (build/main)
	$(CC) $(OBJS) -o $@

# ==========================================
# 4. Compilation Rule
# ==========================================

# Generic rule to compile any .c file into a .o file.
# % acts as a wildcard matching the filename.
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@# Compile the source file ($<) into an object file ($@)
	@# -c : Compile or assemble the source files, but do not link.
	@# $< : The first dependency (the .c file)
	@# $@ : The target file (the .o file)
	$(CC) $(CFLAGS) -c $< -o $@

# ==========================================
# 5. Linter (Static Analysis)
# ==========================================

# Runs clang-tidy on source files to check for code style and errors.
# The '--' separates the source files from the compiler flags needed to parse them.
lint:
	clang-tidy $(SRCS) -- $(CFLAGS)

# ==========================================
# 6. Cleaning
# ==========================================

# Removes the build directory to ensure a fresh compilation.
clean:
	rm -rf $(BUILD_DIR)

# .PHONY tells Make that these targets are not actual files.
# This prevents conflicts if you accidentally create a file named "clean" or "all".
.PHONY: all lint clean