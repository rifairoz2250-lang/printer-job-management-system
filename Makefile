# ==============================================================================
# Makefile
# Project: Printer Job Management System using Queue in C
# Target Audience: BCA Data Science / Computer Science College Students
# Compiler: GCC or Clang (Standard C99 / C11)
# ==============================================================================

CC ?= gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -Iinclude -O2
LDFLAGS =

# Source files
SRC_DIR = src
INC_DIR = include
TEST_DIR = tests
OBJ_DIR = obj

SRCS = $(SRC_DIR)/queue.c $(SRC_DIR)/printer.c $(SRC_DIR)/server.c $(SRC_DIR)/main.c $(TEST_DIR)/test_queue.c
OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))

TARGET = printer_system
TEST_BIN = test_runner
SHARED_LIB = libprinter_queue.dylib

.PHONY: all clean run cli test web py shared help

all: $(TARGET) shared

# Compile main application
$(TARGET): $(SRCS)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o bin/$(TARGET) $(SRCS) $(LDFLAGS)
	@cp bin/$(TARGET) ./$(TARGET)
	@which codesign > /dev/null 2>&1 && codesign -s - -f ./$(TARGET) > /dev/null 2>&1 || true
	@echo "Build complete! Executable created: ./$(TARGET)"

# Compile C shared library for python/backend bindings
shared:
	$(CC) -shared -fPIC -I$(INC_DIR) -O2 $(SRC_DIR)/queue.c $(SRC_DIR)/printer.c -o $(SHARED_LIB)
	@which codesign > /dev/null 2>&1 && codesign -s - -f ./$(SHARED_LIB) > /dev/null 2>&1 || true
	@echo "Shared library built: ./$(SHARED_LIB)"

# Run automated test suite
test: $(TARGET)
	@echo "\nRunning Automated Queue Verification Tests..."
	./$(TARGET) --test

# Run Web Dashboard mode (port 8080)
run: $(TARGET)
	@echo "\nStarting C POSIX HTTP Web Server on http://localhost:8080..."
	./$(TARGET) --web 8080

web: run

# Run Python ctypes backend server
py: shared
	python3 server.py 8080

# Run Interactive Terminal CLI mode
cli: $(TARGET)
	./$(TARGET) --cli

# Clean generated binaries and artifacts
clean:
	rm -rf bin obj $(TARGET) $(TEST_BIN) *.o
	@echo "Clean completed."

help:
	@echo "Available make targets:"
	@echo "  make         - Compile the complete application (./printer_system)"
	@echo "  make test    - Run all 7 automated test scenarios"
	@echo "  make run     - Launch the modern Web Dashboard on http://localhost:8080"
	@echo "  make cli     - Run the terminal CLI interactive menu"
	@echo "  make clean   - Remove binaries and build artifacts"
