#!/bin/bash

###############################################################################
# run_all.sh
# Complete Automation Script for Coroutine Benchmark Project
#
# This script:
#   1. Checks dependencies
#   2. Compiles the project
#   3. Runs benchmarks
#   4. Generates visualizations
#   5. Displays results
###############################################################################

set -e  # Exit on error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Print banner
print_banner() {
    echo -e "${CYAN}"
    echo "=============================================================="
    echo "  STACKLESS COROUTINE LIBRARY - AUTOMATED BENCHMARK SUITE"
    echo "=============================================================="
    echo -e "${NC}"
}

# Print section header
print_section() {
    echo -e "\n${BLUE}▶ $1${NC}"
    echo "--------------------------------------------------------------"
}

# Print success message
print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

# Print error message
print_error() {
    echo -e "${RED}✗ $1${NC}"
}

# Print warning message
print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

# Check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check dependencies
check_dependencies() {
    print_section "Checking Dependencies"
    
    local missing_deps=0
    
    # Check GCC
    if command_exists gcc; then
        print_success "GCC compiler found ($(gcc --version | head -n1))"
    else
        print_error "GCC compiler not found!"
        missing_deps=1
    fi
    
    # Check Make
    if command_exists make; then
        print_success "Make utility found"
    else
        print_error "Make utility not found!"
        missing_deps=1
    fi
    
    # Check Python3
    if command_exists python3; then
        print_success "Python3 found ($(python3 --version))"
    else
        print_error "Python3 not found!"
        missing_deps=1
    fi
    
    # Check Python packages
    if python3 -c "import matplotlib" 2>/dev/null; then
        print_success "Matplotlib package found"
    else
        print_warning "Matplotlib not found (visualization will fail)"
        print_warning "Install with: pip3 install matplotlib"
    fi
    
    if python3 -c "import numpy" 2>/dev/null; then
        print_success "Numpy package found"
    else
        print_warning "Numpy not found (visualization will fail)"
        print_warning "Install with: pip3 install numpy"
    fi
    
    if [ $missing_deps -eq 1 ]; then
        print_error "Missing required dependencies!"
        echo ""
        echo "To install dependencies on Ubuntu/Debian:"
        echo "  sudo apt-get update"
        echo "  sudo apt-get install build-essential python3 python3-pip"
        echo "  pip3 install matplotlib numpy"
        exit 1
    fi
    
    print_success "All dependencies satisfied"
}

# Clean previous builds
clean_previous() {
    print_section "Cleaning Previous Build"
    
    if [ -d "build" ] || [ -d "bin" ]; then
        make clean >/dev/null 2>&1 || true
        print_success "Previous build artifacts removed"
    else
        print_success "No previous build found"
    fi
}

# Compile project
compile_project() {
    print_section "Compiling Project"
    
    echo "Building coroutine libraries and benchmark suite..."
    if make all; then
        print_success "Compilation successful"
    else
        print_error "Compilation failed!"
        exit 1
    fi
}

# Run stackless benchmark
run_stackless_benchmark() {
    print_section "Running Stackless Coroutine Benchmark"
    
    echo "This may take a minute..."
    if ./bin/bench stackless; then
        print_success "Stackless benchmark completed"
    else
        print_error "Stackless benchmark failed!"
        exit 1
    fi
}

# Run ucontext benchmark
run_ucontext_benchmark() {
    print_section "Running Ucontext Coroutine Benchmark"
    
    echo "This may take a minute..."
    if ./bin/bench ucontext; then
        print_success "Ucontext benchmark completed"
    else
        print_error "Ucontext benchmark failed!"
        exit 1
    fi
}

# Generate visualization
generate_plots() {
    print_section "Generating Visualizations"
    
    if [ ! -f "stackless_results.txt" ] || [ ! -f "ucontext_results.txt" ]; then
        print_error "Result files not found!"
        exit 1
    fi
    
    if python3 scripts/plot_results.py; then
        print_success "Visualization generated successfully"
    else
        print_warning "Visualization generation failed (continuing anyway)"
    fi
}

# Display results summary
display_results() {
    print_section "Results Summary"
    
    if [ -f "stackless_results.txt" ] && [ -f "ucontext_results.txt" ]; then
        echo ""
        echo "📊 STACKLESS COROUTINE:"
        cat stackless_results.txt | while read line; do
            echo "   $line"
        done
        
        echo ""
        echo "📊 UCONTEXT COROUTINE:"
        cat ucontext_results.txt | while read line; do
            echo "   $line"
        done
        
        echo ""
        
        # Calculate speedup
        stackless_mean=$(grep "mean=" stackless_results.txt | cut -d'=' -f2)
        ucontext_mean=$(grep "mean=" ucontext_results.txt | cut -d'=' -f2)
        
        if command_exists bc; then
            speedup=$(echo "scale=2; $ucontext_mean / $stackless_mean" | bc)
            echo -e "${GREEN}🚀 Stackless is ${speedup}× faster than Ucontext!${NC}"
        fi
    else
        print_error "Result files not found!"
    fi
}

# List generated files
list_outputs() {
    print_section "Generated Files"
    
    echo "Results:"
    [ -f "stackless_results.txt" ] && echo "  ✓ stackless_results.txt"
    [ -f "ucontext_results.txt" ] && echo "  ✓ ucontext_results.txt"
    
    echo ""
    echo "Visualizations:"
    [ -f "benchmark_plot.png" ] && echo "  ✓ benchmark_plot.png"
    [ -f "benchmark_detailed.png" ] && echo "  ✓ benchmark_detailed.png"
    
    echo ""
    echo "Executables:"
    [ -f "bin/bench" ] && echo "  ✓ bin/bench"
}

# Main execution
main() {
    print_banner
    
    # Record start time
    start_time=$(date +%s)
    
    # Run all steps
    check_dependencies
    clean_previous
    compile_project
    run_stackless_benchmark
    run_ucontext_benchmark
    generate_plots
    display_results
    list_outputs
    
    # Calculate elapsed time
    end_time=$(date +%s)
    elapsed=$((end_time - start_time))
    
    # Final message
    echo ""
    echo -e "${CYAN}=============================================================="
    echo "  ✅ BENCHMARK SUITE COMPLETED SUCCESSFULLY!"
    echo "  ⏱️  Total execution time: ${elapsed} seconds"
    echo "==============================================================${NC}"
    echo ""
    echo "Next steps:"
    echo "  • View plots: benchmark_plot.png, benchmark_detailed.png"
    echo "  • Read results: stackless_results.txt, ucontext_results.txt"
    echo "  • Run again: ./run_all.sh"
    echo "  • Run specific: ./bin/bench [stackless|ucontext|both]"
    echo ""
}

# Handle Ctrl+C
trap 'echo -e "\n${RED}Benchmark interrupted by user${NC}"; exit 130' INT

# Run main function
main "$@"
