# Configuration
BUILD_DIR ?= build
BUILD_TYPE ?= Debug
STATIC ?= 0
CMAKE := cmake
EXECUTABLE := $(BUILD_DIR)/app/ll1

ifeq ($(filter 1 true TRUE on ON,$(STATIC)),)
STATIC_FLAG := OFF
else
STATIC_FLAG := ON
endif
CMAKE_CONFIG_FLAGS := -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	-DLL1CHECKER_FORCE_STATIC_RUNTIME=$(STATIC_FLAG)

# Targets
.PHONY: all build clean rebuild format run help

all: build

build:
	@echo ">> Configuring build ($(BUILD_TYPE))..."
	@$(CMAKE) -S . -B $(BUILD_DIR) $(CMAKE_CONFIG_FLAGS)
	@echo ">> Building..."
	@$(CMAKE) --build $(BUILD_DIR) -- -j$(shell nproc)

clean:
	@echo ">> Removing build directory..."
	@rm -rf $(BUILD_DIR)

rebuild: clean build

format:
	@echo ">> Formatting .cpp and .hpp files with clang-format..."
	@find src include \( -name "*.cpp" -o -name "*.hpp" \) | grep -v "include/tabulate.hpp" | xargs clang-format -i

run: build
	@echo ">> Running $(EXECUTABLE)..."
	@$(EXECUTABLE)

static: BUILD_TYPE = Release
static: STATIC = 1
static: build

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
	@echo "To force a static Release build:"
	@echo "  make static"
	@echo "  # or make BUILD_TYPE=Release STATIC=1 build"
