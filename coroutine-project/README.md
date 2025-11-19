

## 🎯 Overview

This project implements and compares two distinct approaches to user-space coroutines:

1. **Stackless Coroutines**: Ultra-fast context switches using state machine logic
2. **Stackful Coroutines**: Traditional approach using POSIX `ucontext` API

The goal is to measure and visualize the performance difference between these approaches, demonstrating how implementation choices affect context-switch overhead at the nanosecond scale.

## ✨ Features

- 🚀 **Two Coroutine Implementations**
  - Stackless: State machine-based, minimal overhead
  - Ucontext: Full stack preservation, POSIX-compliant

- ⚡ **High-Precision Benchmarking**
  - Nanosecond timing using `clock_gettime(CLOCK_MONOTONIC)`
  - 10 million context switches per test
  - Statistical sampling with mean/min/max analysis

- 📊 **Professional Visualization**
  - Comparative bar charts
  - Performance speedup analysis
  - Multiple plot formats (PNG)

- 🛠️ **Complete Build System**
  - Makefile with multiple targets
  - Automated bash script for full pipeline
  - Modular project structure

## 📁 Project Structure

```
coroutine-project/
├── include/
│   ├── coro_stackless.h      # Stackless coroutine header
│   └── coro_ucontext.h        # Ucontext coroutine header
├── src/
│   ├── coro_stackless.c       # Stackless implementation
│   ├── coro_ucontext.c        # Ucontext implementation
│   └── bench.c                # Benchmark suite
├── scripts/
│   └── plot_results.py        # Python visualization script
├── build/                     # Compiled object files (generated)
├── bin/                       # Executables (generated)
├── Makefile                   # Build configuration
├── run_all.sh                 # Automation script
└── README.md                  # This file
```

## 🔧 Requirements

### System Requirements
- **OS**: Linux (Ubuntu 20.04+ recommended)
- **Compiler**: GCC 9.0+ with C11 support
- **Make**: GNU Make 4.0+
- **Python**: Python 3.8+

### Python Packages
```bash
pip3 install matplotlib numpy
```

### Install All Dependencies (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y build-essential python3 python3-pip
pip3 install matplotlib numpy
```

Or use the Makefile target:
```bash
make install-deps
```

### Building the Project

```bash
# Build all components
make

# Clean and rebuild
make rebuild

# Clean only
make clean

# View all targets
make help
```

### Running Benchmarks

The benchmark executable accepts an optional argument:

```bash
# Run both benchmarks
./bin/bench both

# Run only stackless
./bin/bench stackless

# Run only ucontext
./bin/bench ucontext
```

### Output Files

After running benchmarks, the following files are generated:

- `stackless_results.txt` - Stackless benchmark statistics
- `ucontext_results.txt` - Ucontext benchmark statistics
- `benchmark_plot.png` - Main comparison visualization
- `benchmark_detailed.png` - Detailed analysis with error bars

## 🔬 Implementation Details

### Stackless Coroutines

**Concept**: Uses a state machine approach where each coroutine saves its execution point as an integer and resumes from that point.

**Key Characteristics**:
- No separate stack allocation
- Minimal memory footprint (< 100 bytes per coroutine)
- Ultra-fast context switches (just a function call + state update)
- Limited to non-reentrant code paths

**Implementation Highlights**:
```c
// State machine macros
#define CORO_BEGIN(coro) switch((coro)->resume_point) { case 0:
#define CORO_YIELD(coro) do { (coro)->resume_point = __LINE__; return; case __LINE__:; } while(0)
#define CORO_END(coro) } (coro)->state = CORO_STATE_FINISHED
```

**Context Switch Process**:
1. Save current line number as resume point
2. Return from coroutine function
3. On resume, switch to saved line number
4. Continue execution

**Complexity**: O(1) time, O(1) space

### Stackful Coroutines (Ucontext)

**Concept**: Uses POSIX `ucontext` API to create coroutines with full stack preservation.

**Key Characteristics**:
- Each coroutine has 64KB stack
- Full CPU register saving/restoring
- Supports arbitrary call graphs and recursion
- Higher memory usage and slower switches

**Implementation Highlights**:
```c
// Context switching
swapcontext(&main_context, &coroutine_context);

// Stack allocation
char *stack = malloc(CORO_STACK_SIZE);
context.uc_stack.ss_sp = stack;
context.uc_stack.ss_size = CORO_STACK_SIZE;
```

**Context Switch Process**:
1. Save all CPU registers
2. Save stack pointer
3. Load target coroutine's registers
4. Switch stack pointer
5. Resume execution

**Complexity**: O(k) time where k = number of registers, O(n) space where n = stack size

### Benchmarking Methodology

**Timing**: Uses `clock_gettime(CLOCK_MONOTONIC)` for nanosecond precision
**Warmup**: 100,000 iterations to warm CPU caches
**Measurement**: 10 million context switches per benchmark
**Sampling**: 10 independent runs for statistical reliability

**Metrics Calculated**:
- Mean time per context switch
- Minimum time (best case)
- Maximum time (worst case)
- Range (variability)

## 📊 Benchmark Results

### Expected Performance Characteristics

**Typical Results** (on modern x86-64 CPU):

| Implementation | Mean Time | Performance |
|---------------|-----------|-------------|
| Stackless     | ~5-15 ns  | ⚡⚡⚡⚡⚡ |
| Ucontext      | ~50-200 ns| ⚡⚡ |

**Speedup**: Stackless is typically **5-20× faster** than ucontext

### Why is Stackless Faster?

1. **No Register Operations**: Stackless doesn't save/restore CPU registers
2. **No Stack Switching**: Stack pointer remains unchanged
3. **Cache Friendly**: Minimal memory access, better cache locality
4. **Simple Logic**: Just a function call and integer update

### Trade-offs

| Aspect | Stackless | Ucontext |
|--------|-----------|----------|
| Speed | ⚡⚡⚡⚡⚡ Very Fast | ⚡⚡ Moderate |
| Memory | 💚 Low (~100B) | 💛 High (~64KB) |
| Flexibility | ⚠️ Limited | ✅ Full |
| Debugging | 🔧 Harder | 🔧 Easier |
| Use Case | High-frequency | General purpose |

## 🎓 Performance Analysis

### Understanding the Results

**Stackless Performance**:
- Best for: Event loops, state machines, parsers
- Overhead: Function call + state variable update
- Bottleneck: Cache misses on state variable

**Ucontext Performance**:
- Best for: General coroutines, arbitrary call graphs
- Overhead: Register save/restore + stack switch
- Bottleneck: Memory bandwidth for register/stack operations

### Real-World Applications

**Stackless Coroutines**:
- Protothreads in embedded systems
- Async/await in some languages
- High-performance parsers
- Game engine task systems

**Stackful Coroutines**:
- Go goroutines
- Lua coroutines
- Ruby fibers
- Python generators (hybrid approach)


## 🔍 Troubleshooting

### Common Issues

**Issue**: `ucontext.h` not found
```bash
# Solution: Install development headers
sudo apt-get install libc6-dev
```

**Issue**: Matplotlib not found
```bash
# Solution: Install Python packages
pip3 install --user matplotlib numpy
```

**Issue**: Benchmark crashes or hangs
```bash
# Solution: Reduce iterations in bench.c
# Change NUM_SWITCHES to 1000000 (1 million)
```

**Issue**: Permission denied on run_all.sh
```bash
# Solution: Make executable
chmod +x run_all.sh
```

**Issue**: Inconsistent benchmark results
- Close other applications to reduce system noise
- Run multiple times and average results
- Check CPU governor: `cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`
- Set to performance mode: `sudo cpupower frequency-set -g performance`

## 📚 References

### Academic Papers
1. Duff's Device and Coroutines (Simon Tatham, 2000)
2. "Protothreads: Simplifying Event-Driven Programming of Memory-Constrained Embedded Systems"
3. "Revisiting Coroutines" (Ana Lúcia de Moura & Roberto Ierusalimschy, 2004)

### Documentation
- POSIX `ucontext` man pages: `man 3 makecontext`
- C11 Standard (ISO/IEC 9899:2011)
- Linux kernel coroutine implementations

### Related Projects
- Protothreads: http://dunkels.com/adam/pt/
- libco: https://github.com/higan-emu/libco
- Boost.Context: https://www.boost.org/doc/libs/release/libs/context/

## 🎯 Learning Outcomes

By completing this project, you will understand:

1. **Operating System Concepts**
   - Context switching mechanisms
   - User-space vs kernel-space threading
   - Stack management

2. **Performance Engineering**
   - Nanosecond-level benchmarking
   - Cache effects on performance
   - Statistical analysis of measurements

3. **C Programming**
   - Advanced macros and preprocessor tricks
   - POSIX APIs (ucontext)
   - Memory management

4. **Software Engineering**
   - Modular code organization
   - Build systems (Make)
   - Automation scripting

