# Makefile for Stackless Coroutine Library Project
# Compiles both coroutine implementations and benchmark suite

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O3 -march=native -std=c11
LDFLAGS = -lrt -pthread

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Source files
STACKLESS_SRC = $(SRC_DIR)/coro_stackless.c
UCONTEXT_SRC = $(SRC_DIR)/coro_ucontext.c
BENCH_SRC = $(SRC_DIR)/bench.c

# Object files
STACKLESS_OBJ = $(BUILD_DIR)/coro_stackless.o
UCONTEXT_OBJ = $(BUILD_DIR)/coro_ucontext.o
BENCH_OBJ = $(BUILD_DIR)/bench.o

# Executables
BENCH_EXEC = $(BIN_DIR)/bench

# Default target
.PHONY: all
all: directories $(BENCH_EXEC)

# Create necessary directories
.PHONY: directories
directories:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

# Compile stackless coroutine library
$(STACKLESS_OBJ): $(STACKLESS_SRC) $(INC_DIR)/coro_stackless.h
	@echo "Compiling stackless coroutine library..."
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $(STACKLESS_SRC) -o $(STACKLESS_OBJ)

# Compile ucontext coroutine library
$(UCONTEXT_OBJ): $(UCONTEXT_SRC) $(INC_DIR)/coro_ucontext.h
	@echo "Compiling ucontext coroutine library..."
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $(UCONTEXT_SRC) -o $(UCONTEXT_OBJ)

# Compile benchmark
$(BENCH_OBJ): $(BENCH_SRC) $(INC_DIR)/coro_stackless.h $(INC_DIR)/coro_ucontext.h
	@echo "Compiling benchmark suite..."
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $(BENCH_SRC) -o $(BENCH_OBJ)

# Link benchmark executable
$(BENCH_EXEC): $(BENCH_OBJ) $(STACKLESS_OBJ) $(UCONTEXT_OBJ)
	@echo "Linking benchmark executable..."
	$(CC) $(CFLAGS) $(BENCH_OBJ) $(STACKLESS_OBJ) $(UCONTEXT_OBJ) -o $(BENCH_EXEC) $(LDFLAGS)
	@echo "✓ Build complete! Executable: $(BENCH_EXEC)"

# Run benchmarks
.PHONY: run
run: all
	@echo "Running benchmarks..."
	@./$(BENCH_EXEC) both

# Run only stackless benchmark
.PHONY: run-stackless
run-stackless: all
	@echo "Running stackless benchmark..."
	@./$(BENCH_EXEC) stackless

# Run only ucontext benchmark
.PHONY: run-ucontext
run-ucontext: all
	@echo "Running ucontext benchmark..."
	@./$(BENCH_EXEC) ucontext

# Generate visualization
.PHONY: plot
plot:
	@echo "Generating visualization..."
	@python3 scripts/plot_results.py

# Run complete benchmark suite with visualization
.PHONY: benchmark
benchmark: all
	@echo "Running complete benchmark suite..."
	@./$(BENCH_EXEC) both
	@echo ""
	@python3 scripts/plot_results.py

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@rm -f stackless_results.txt ucontext_results.txt
	@rm -f benchmark_plot.png benchmark_detailed.png
	@echo "✓ Clean complete"

# Clean and rebuild
.PHONY: rebuild
rebuild: clean all

# Display help
.PHONY: help
help:
	@echo "Stackless Coroutine Library - Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make              - Build all components"
	@echo "  make run          - Build and run all benchmarks"
	@echo "  make run-stackless- Run only stackless benchmark"
	@echo "  make run-ucontext - Run only ucontext benchmark"
	@echo "  make plot         - Generate visualization from results"
	@echo "  make benchmark    - Run benchmarks and generate plots"
	@echo "  make clean        - Remove build artifacts and results"
	@echo "  make rebuild      - Clean and rebuild everything"
	@echo "  make help         - Display this help message"

# Install dependencies (for Ubuntu/Debian)
.PHONY: install-deps
install-deps:
	@echo "Installing dependencies..."
	@sudo apt-get update
	@sudo apt-get install -y build-essential python3 python3-pip
	@pip3 install matplotlib numpy
	@echo "✓ Dependencies installed"
