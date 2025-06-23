# Configuration
BUILD_DIR := build
EXECUTABLE := $(BUILD_DIR)/app/ll1
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
BUILD_TYPE ?= Debug
CMAKE := cmake

# Targets
.PHONY: all build clean rebuild format run help

all: build

build:
	@echo ">> Configuring build ($(BUILD_TYPE))..."
	@$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@echo ">> Building..."
	@$(CMAKE) --build $(BUILD_DIR) -- -j$(shell nproc)

clean:
	@echo ">> Removing build directory..."
	@rm -rf $(BUILD_DIR)

rebuild: clean build

format:
	@echo ">> Formatting .cpp and .hpp files with clang-format..."
	@find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

run: build
	@echo ">> Running $(EXECUTABLE)..."
	@$(EXECUTABLE)

help:
	@echo "Available commands:"
	@echo "  make build         - Configure and compile the project (Debug by default)"
	@echo "  make clean         - Remove the build/ directory"
	@echo "  make rebuild       - Clean and compile again"
	@echo "  make format        - Run clang-format on all .cpp/.hpp files"
	@echo "  make run           - Build (if needed) and run the executable"
	@echo "  make               - Alias for build"
	@echo
	@echo "To change build type:"
	@echo "  make BUILD_TYPE=Release build"
