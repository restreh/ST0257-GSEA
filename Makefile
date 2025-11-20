# Makefile for GSEA (Gestión Segura y Eficiente de Archivos)
# Compatible with Windows (MinGW/MSVC) and POSIX systems

# Compiler settings
CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17
LDFLAGS = -lpthread

# Detect OS
ifeq ($(OS),Windows_NT)
SHELL := cmd.exe
EXECUTABLE = gsea.exe
RM = cmd /C del /Q
RMDIR = cmd /C rmdir /S /Q
MKDIR = cmd /C if not exist
MKDIR_END = mkdir
LDFLAGS =
PATH_SEP = \\
FIX_PATH = $(subst /,\,$1)
else
EXECUTABLE = gsea
RM = rm -f
RMDIR = rm -rf
MKDIR = mkdir -p
MKDIR_END =
PATH_SEP = /
FIX_PATH = $1
endif

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SOURCES = $(SRC_DIR)/main.cpp \
	$(SRC_DIR)/file_manager.cpp \
	$(SRC_DIR)/huffman.cpp \
	$(SRC_DIR)/aes.cpp \
	$(SRC_DIR)/worker.cpp \
	$(SRC_DIR)/arg_parser.cpp

# Object files
OBJECTS = $(OBJ_DIR)/main.o \
	$(OBJ_DIR)/file_manager.o \
	$(OBJ_DIR)/huffman.o \
	$(OBJ_DIR)/aes.o \
	$(OBJ_DIR)/worker.o \
	$(OBJ_DIR)/arg_parser.o

# Target executable
TARGET = $(BIN_DIR)/$(EXECUTABLE)

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
ifeq ($(OS),Windows_NT)
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)" 2>nul || echo Directory exists
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)" 2>nul || echo Directory exists
else
	@$(MKDIR) $(OBJ_DIR)
	@$(MKDIR) $(BIN_DIR)
endif

# Ensure directories exist before building objects
$(OBJ_DIR) $(BIN_DIR):
ifeq ($(OS),Windows_NT)
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)" 2>nul || true
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)" 2>nul || true
else
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)
endif

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo Build complete: $(TARGET)

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
ifeq ($(OS),Windows_NT)
	@$(RMDIR) $(call FIX_PATH,$(OBJ_DIR)) 2>nul || echo Nothing to clean
	@$(RMDIR) $(call FIX_PATH,$(BIN_DIR)) 2>nul || echo Nothing to clean
	@echo Clean complete
else
	@$(RMDIR) $(OBJ_DIR) 2>/dev/null || true
	@$(RMDIR) $(BIN_DIR) 2>/dev/null || true
	@echo Clean complete
endif

# Rebuild everything
rebuild: clean all

.PHONY: all clean rebuild directories

