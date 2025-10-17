#!/usr/bin/env python3
"""
plot_results.py
Benchmark Results Visualization

This script reads benchmark results from text files and creates
professional visualizations comparing stackless vs ucontext coroutines.
"""

import matplotlib.pyplot as plt
import numpy as np
import os
import sys

def read_results(filename):
    """
    Read benchmark results from file
    Returns: dict with mean, min, max values
    """
    if not os.path.exists(filename):
        print(f"Error: {filename} not found!")
        return None
    
    results = {}
    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            if '=' in line:
                key, value = line.split('=')
                results[key] = float(value)
    
    return results

def create_comparison_plot(stackless_data, ucontext_data):
    """
    Create a beautiful comparison plot of benchmark results
    """
    # Set style for professional appearance
    plt.style.use('seaborn-v0_8-darkgrid')
    
    # Create figure with subplots
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    fig.suptitle('Coroutine Context-Switch Performance Comparison', 
                 fontsize=16, fontweight='bold')
    
    # ===== Plot 1: Bar chart comparison =====
    categories = ['Mean', 'Min', 'Max']
    stackless_values = [stackless_data['mean'], stackless_data['min'], 
                       stackless_data['max']]
    ucontext_values = [ucontext_data['mean'], ucontext_data['min'], 
                      ucontext_data['max']]
    
    x = np.arange(len(categories))
    width = 0.35
    
    bars1 = ax1.bar(x - width/2, stackless_values, width, label='Stackless',
                    color='#2ecc71', alpha=0.8, edgecolor='black')
    bars2 = ax1.bar(x + width/2, ucontext_values, width, label='Ucontext',
                    color='#e74c3c', alpha=0.8, edgecolor='black')
    
    ax1.set_ylabel('Time per Context Switch (nanoseconds)', fontsize=11, fontweight='bold')
    ax1.set_title('Context-Switch Time Statistics', fontsize=12, fontweight='bold')
    ax1.set_xticks(x)
    ax1.set_xticklabels(categories)
    ax1.legend(fontsize=10)
    ax1.grid(True, alpha=0.3, axis='y')
    
    # Add value labels on bars
    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            ax1.text(bar.get_x() + bar.get_width()/2., height,
                    f'{height:.1f}',
                    ha='center', va='bottom', fontsize=9, fontweight='bold')
    
    # ===== Plot 2: Speedup visualization =====
    speedup = ucontext_data['mean'] / stackless_data['mean']
    
    ax2.barh(['Stackless', 'Ucontext'], 
             [ucontext_data['mean'], stackless_data['mean']], 
             color=['#2ecc71', '#e74c3c'], alpha=0.8, edgecolor='black')
    
    ax2.set_xlabel('Time per Context Switch (nanoseconds)', fontsize=11, fontweight='bold')
    ax2.set_title('Mean Performance Comparison', fontsize=12, fontweight='bold')
    ax2.grid(True, alpha=0.3, axis='x')
    
    # Add speedup annotation
    ax2.text(0.95, 0.95, f'Stackless is {speedup:.2f}× faster',
             transform=ax2.transAxes, fontsize=11, fontweight='bold',
             verticalalignment='top', horizontalalignment='right',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    
    # Add value labels
    for i, (name, value) in enumerate(zip(['Ucontext', 'Stackless'], 
                                           [ucontext_data['mean'], stackless_data['mean']])):
        ax2.text(value, i, f' {value:.1f} ns', 
                va='center', fontsize=10, fontweight='bold')
    
    plt.tight_layout()
    
    # Save the plot
    plt.savefig('benchmark_plot.png', dpi=300, bbox_inches='tight')
    print("✓ Plot saved as 'benchmark_plot.png'")
    
    # Show the plot
    plt.show()

def create_detailed_plot(stackless_data, ucontext_data):
    """
    Create an additional detailed analysis plot
    """
    fig, ax = plt.subplots(figsize=(10, 6))
    
    # Data for plotting
    methods = ['Stackless\nCoroutine', 'Ucontext\nCoroutine']
    means = [stackless_data['mean'], ucontext_data['mean']]
    mins = [stackless_data['min'], ucontext_data['min']]
    maxs = [stackless_data['max'], ucontext_data['max']]
    
    # Calculate error bars (distance from mean)
    yerr_lower = [means[i] - mins[i] for i in range(len(means))]
    yerr_upper = [maxs[i] - means[i] for i in range(len(means))]
    
    # Create bar plot with error bars
    x_pos = np.arange(len(methods))
    bars = ax.bar(x_pos, means, yerr=[yerr_lower, yerr_upper],
                  color=['#3498db', '#e67e22'], alpha=0.7,
                  error_kw={'linewidth': 2, 'ecolor': 'black', 'capsize': 10},
                  edgecolor='black', linewidth=2)
    
    ax.set_ylabel('Context-Switch Time (nanoseconds)', fontsize=12, fontweight='bold')
    ax.set_title('Context-Switch Performance with Min/Max Range', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(x_pos)
    ax.set_xticklabels(methods, fontsize=11)
    ax.grid(True, alpha=0.3, axis='y')
    
    # Add annotations
    for i, (bar, mean, min_val, max_val) in enumerate(zip(bars, means, mins, maxs)):
        # Mean value
        ax.text(bar.get_x() + bar.get_width()/2., mean,
               f'Mean: {mean:.1f} ns',
               ha='center', va='bottom', fontsize=10, fontweight='bold')
        
        # Range annotation
        range_val = max_val - min_val
        ax.text(bar.get_x() + bar.get_width()/2., max_val + (max(maxs) * 0.05),
               f'Range: {range_val:.1f} ns',
               ha='center', va='bottom', fontsize=9, style='italic')
    
    plt.tight_layout()
    plt.savefig('benchmark_detailed.png', dpi=300, bbox_inches='tight')
    print("✓ Detailed plot saved as 'benchmark_detailed.png'")

def print_analysis(stackless_data, ucontext_data):
    """
    Print detailed analysis of results
    """
    print("\n" + "="*60)
    print(" BENCHMARK ANALYSIS SUMMARY")
    print("="*60)
    
    print("\n📊 STACKLESS COROUTINE:")
    print(f"   Mean:  {stackless_data['mean']:.2f} ns/switch")
    print(f"   Min:   {stackless_data['min']:.2f} ns/switch")
    print(f"   Max:   {stackless_data['max']:.2f} ns/switch")
    print(f"   Range: {stackless_data['max'] - stackless_data['min']:.2f} ns")
    
    print("\n📊 UCONTEXT COROUTINE:")
    print(f"   Mean:  {ucontext_data['mean']:.2f} ns/switch")
    print(f"   Min:   {ucontext_data['min']:.2f} ns/switch")
    print(f"   Max:   {ucontext_data['max']:.2f} ns/switch")
    print(f"   Range: {ucontext_data['max'] - ucontext_data['min']:.2f} ns")
    
    speedup = ucontext_data['mean'] / stackless_data['mean']
    overhead = ucontext_data['mean'] - stackless_data['mean']
    
    print("\n🚀 PERFORMANCE COMPARISON:")
    print(f"   Speedup:           {speedup:.2f}× (Stackless is faster)")
    print(f"   Absolute overhead: {overhead:.2f} ns")
    print(f"   Relative overhead: {((speedup - 1) * 100):.1f}%")
    
    print("\n💡 INTERPRETATION:")
    if speedup > 2:
        print("   • Stackless coroutines show SIGNIFICANT performance advantage")
        print("   • Minimal state machine overhead vs. full context switching")
    else:
        print("   • Stackless coroutines show moderate performance advantage")
    
    print("   • Ucontext requires saving/restoring CPU registers and stack")
    print("   • Stackless only updates a simple state variable")
    print("   • Trade-off: Stackless is faster but less flexible")
    
    print("\n" + "="*60 + "\n")

def main():
    """
    Main visualization function
    """
    print("\n" + "="*60)
    print(" COROUTINE BENCHMARK VISUALIZATION")
    print("="*60 + "\n")
    
    # Read results
    print("Reading benchmark results...")
    stackless_data = read_results('stackless_results.txt')
    ucontext_data = read_results('ucontext_results.txt')
    
    if stackless_data is None or ucontext_data is None:
        print("\n❌ Error: Could not read result files!")
        print("Make sure you've run the benchmark first:")
        print("   ./bench stackless")
        print("   ./bench ucontext")
        sys.exit(1)
    
    print("✓ Results loaded successfully\n")
    
    # Create visualizations
    print("Creating visualizations...")
    create_comparison_plot(stackless_data, ucontext_data)
    create_detailed_plot(stackless_data, ucontext_data)
    
    # Print analysis
    print_analysis(stackless_data, ucontext_data)
    
    print("✅ Visualization complete!")
    print("\nGenerated files:")
    print("  • benchmark_plot.png")
    print("  • benchmark_detailed.png")

if __name__ == "__main__":
    main()
