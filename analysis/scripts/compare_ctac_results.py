#!/usr/bin/env python3
"""
CTAC Performance Analysis and Visualization
Compares Baseline (ECDSA) vs CTAC for:
1. Age of Information (AoI) - from V2V logs
2. Channel Busy Ratio (CBR) - from scalar files
"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
import sys

# Set style for publication-quality plots
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
plt.rcParams['figure.figsize'] = (14, 6)
plt.rcParams['font.size'] = 11
plt.rcParams['axes.grid'] = True
plt.rcParams['grid.alpha'] = 0.3

def compute_aoi_metrics(csv_path, max_distance_m=300):
    """
    Compute Age of Information (AoI) metrics from V2V reception logs.

    AoI calculation (per CTAC spec):
    1. Generation time: g = t - delay_ms / 1000.0
    2. Group by (receiver, sender), sort by time
    3. For consecutive receptions i-1, i:
       PAoI_i = t_i - g_{i-1} (peak AoI)
       IPG_i = t_i - t_{i-1} (inter-packet gap)
    4. Filter: keep only if dist_m < max_distance_m at both endpoints

    Args:
        csv_path: Path to v2v_logs.csv
        max_distance_m: Maximum distance filter (default 300m)

    Returns:
        DataFrame with AoI samples
    """
    print(f"\nReading: {csv_path}")

    # Read CSV
    df = pd.read_csv(csv_path)

    print(f"  Total receptions: {len(df):,}")

    # Compute generation time
    df['gen_time'] = df['t'] - (df['delay_ms'] / 1000.0)

    # Initialize lists for AoI samples
    aoi_samples = []
    ipg_samples = []

    # Group by (receiver, sender) pairs
    grouped = df.groupby(['receiver', 'sender'])

    print(f"  Unique (receiver, sender) pairs: {len(grouped):,}")

    pair_count = 0
    for (receiver, sender), group in grouped:
        # Sort by reception time
        group = group.sort_values('t').reset_index(drop=True)

        # Need at least 2 receptions to compute AoI
        if len(group) < 2:
            continue

        # Compute AoI for consecutive receptions
        for i in range(1, len(group)):
            curr = group.iloc[i]
            prev = group.iloc[i-1]

            # Relevance filter: both receptions within max_distance_m
            if curr['dist_m'] < max_distance_m and prev['dist_m'] < max_distance_m:
                # Peak AoI = current_rx_time - previous_gen_time
                peak_aoi_ms = (curr['t'] - prev['gen_time']) * 1000.0

                # Inter-packet gap
                ipg_ms = (curr['t'] - prev['t']) * 1000.0

                aoi_samples.append({
                    'receiver': receiver,
                    'sender': sender,
                    'time': curr['t'],
                    'distance_m': curr['dist_m'],
                    'peak_aoi_ms': peak_aoi_ms,
                    'ipg_ms': ipg_ms
                })

        pair_count += 1
        if pair_count % 1000 == 0:
            print(f"    Processed {pair_count:,} pairs...", end='\r')

    print(f"    Processed {pair_count:,} pairs - Done!      ")

    # Convert to DataFrame
    aoi_df = pd.DataFrame(aoi_samples)

    print(f"  AoI samples generated: {len(aoi_df):,}")

    return aoi_df

def compute_aoi_stats(aoi_df):
    """Compute statistical summary of AoI."""
    if len(aoi_df) == 0:
        return {
            'min': 0, 'p25': 0, 'median': 0, 'p75': 0, 'p90': 0,
            'p95': 0, 'p99': 0, 'max': 0, 'mean': 0, 'samples': 0
        }

    peak_aoi = aoi_df['peak_aoi_ms']

    return {
        'min': peak_aoi.min(),
        'p25': peak_aoi.quantile(0.25),
        'median': peak_aoi.quantile(0.50),
        'p75': peak_aoi.quantile(0.75),
        'p90': peak_aoi.quantile(0.90),
        'p95': peak_aoi.quantile(0.95),
        'p99': peak_aoi.quantile(0.99),
        'max': peak_aoi.max(),
        'mean': peak_aoi.mean(),
        'samples': len(peak_aoi)
    }

def extract_cbr_from_sca(sca_path):
    """
    Extract CBR values from OMNeT++ scalar file.

    Args:
        sca_path: Path to .sca file

    Returns:
        DataFrame with CBR values per vehicle
    """
    print(f"\nReading: {sca_path}")

    cbr_values = []

    with open(sca_path, 'r') as f:
        for line in f:
            # Look for cbr:mean or cbr:max entries
            # Format: scalar <module> <name> <value>
            if 'cbr:mean' in line or 'cbr:max' in line:
                parts = line.strip().split()
                if len(parts) >= 4 and parts[0] == 'scalar':
                    module = parts[1]
                    metric = parts[2]
                    value = float(parts[3])

                    cbr_values.append({
                        'module': module,
                        'metric': metric,
                        'value': value
                    })

    cbr_df = pd.DataFrame(cbr_values)
    print(f"  CBR entries found: {len(cbr_df)}")

    return cbr_df

def plot_comparison(baseline_aoi_stats, ctac_aoi_stats,
                   baseline_cbr_df, ctac_cbr_df,
                   output_path='ctac_comparison.png'):
    """
    Create comparison plots for AoI and CBR.
    """
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))

    # ========== Plot 1: Age of Information Comparison ==========
    ax1 = axes[0]

    # Prepare data for box plot
    scenarios = ['Baseline\n(SPS+DCC)', 'CTAC']

    # Min, Median, Max for each scenario
    baseline_data = [
        baseline_aoi_stats['min'],
        baseline_aoi_stats['median'],
        baseline_aoi_stats['max']
    ]
    ctac_data = [
        ctac_aoi_stats['min'],
        ctac_aoi_stats['median'],
        ctac_aoi_stats['max']
    ]

    x_pos = np.arange(len(scenarios))
    width = 0.25

    # Plot min, median, max as grouped bars
    bars1 = ax1.bar(x_pos - width, [baseline_data[0], ctac_data[0]],
                    width, label='Min', color='lightblue', edgecolor='black')
    bars2 = ax1.bar(x_pos, [baseline_data[1], ctac_data[1]],
                    width, label='Median', color='orange', edgecolor='black')
    bars3 = ax1.bar(x_pos + width, [baseline_data[2], ctac_data[2]],
                    width, label='Max', color='salmon', edgecolor='black')

    # Add value labels on bars
    for bars in [bars1, bars2, bars3]:
        for bar in bars:
            height = bar.get_height()
            ax1.text(bar.get_x() + bar.get_width()/2., height,
                    f'{height:.0f}',
                    ha='center', va='bottom', fontsize=9, fontweight='bold')

    # Add reference lines
    ax1.axhline(y=300, color='red', linestyle='--', linewidth=1.5,
                label='AoI Bound (300ms)', alpha=0.7)
    ax1.axhline(y=200, color='green', linestyle='--', linewidth=1.5,
                label='SAE Target (200ms)', alpha=0.7)

    ax1.set_ylabel('Age of Information (ms)', fontsize=12, fontweight='bold')
    ax1.set_title('Peak AoI Distribution\n(Distance < 300m)',
                  fontsize=13, fontweight='bold')
    ax1.set_xticks(x_pos)
    ax1.set_xticklabels(scenarios, fontsize=11)
    ax1.legend(loc='upper right', fontsize=9)
    ax1.grid(axis='y', alpha=0.3)

    # Add sample count annotation
    ax1.text(0.02, 0.98, f"Baseline: {baseline_aoi_stats['samples']:,} samples\n"
                          f"CTAC: {ctac_aoi_stats['samples']:,} samples",
             transform=ax1.transAxes, fontsize=9, verticalalignment='top',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.3))

    # ========== Plot 2: CBR Comparison ==========
    ax2 = axes[1]

    # Extract mean CBR values
    baseline_cbr_mean = baseline_cbr_df[baseline_cbr_df['metric'] == 'cbr:mean']['value']
    ctac_cbr_mean = ctac_cbr_df[ctac_cbr_df['metric'] == 'cbr:mean']['value']

    # Compute stats
    baseline_cbr_stats = {
        'min': baseline_cbr_mean.min() * 100,
        'median': baseline_cbr_mean.median() * 100,
        'max': baseline_cbr_mean.max() * 100,
        'mean': baseline_cbr_mean.mean() * 100
    }

    ctac_cbr_stats = {
        'min': ctac_cbr_mean.min() * 100,
        'median': ctac_cbr_mean.median() * 100,
        'max': ctac_cbr_mean.max() * 100,
        'mean': ctac_cbr_mean.mean() * 100
    }

    # Plot as grouped bars
    baseline_cbr_data = [baseline_cbr_stats['min'], baseline_cbr_stats['median'],
                         baseline_cbr_stats['max']]
    ctac_cbr_data = [ctac_cbr_stats['min'], ctac_cbr_stats['median'],
                     ctac_cbr_stats['max']]

    bars1 = ax2.bar(x_pos - width, [baseline_cbr_data[0], ctac_cbr_data[0]],
                    width, label='Min', color='lightblue', edgecolor='black')
    bars2 = ax2.bar(x_pos, [baseline_cbr_data[1], ctac_cbr_data[1]],
                    width, label='Median', color='orange', edgecolor='black')
    bars3 = ax2.bar(x_pos + width, [baseline_cbr_data[2], ctac_cbr_data[2]],
                    width, label='Max', color='salmon', edgecolor='black')

    # Add value labels
    for bars in [bars1, bars2, bars3]:
        for bar in bars:
            height = bar.get_height()
            ax2.text(bar.get_x() + bar.get_width()/2., height,
                    f'{height:.1f}%',
                    ha='center', va='bottom', fontsize=9, fontweight='bold')

    # Add reference lines (SAE J3161 DCC thresholds)
    ax2.axhline(y=30, color='orange', linestyle='--', linewidth=1.5,
                label='DCC Level 1 (30%)', alpha=0.7)
    ax2.axhline(y=65, color='red', linestyle='--', linewidth=1.5,
                label='DCC Level 2 (65%)', alpha=0.7)

    ax2.set_ylabel('Channel Busy Ratio (%)', fontsize=12, fontweight='bold')
    ax2.set_title('CBR Distribution\n(Mean across vehicles)',
                  fontsize=13, fontweight='bold')
    ax2.set_xticks(x_pos)
    ax2.set_xticklabels(scenarios, fontsize=11)
    ax2.legend(loc='upper right', fontsize=9)
    ax2.grid(axis='y', alpha=0.3)

    # Add reduction annotation
    cbr_reduction = (1 - ctac_cbr_stats['median'] / baseline_cbr_stats['median']) * 100
    ax2.text(0.02, 0.98, f"Median CBR Reduction: {cbr_reduction:.1f}%",
             transform=ax2.transAxes, fontsize=10, verticalalignment='top',
             fontweight='bold',
             bbox=dict(boxstyle='round', facecolor='lightgreen', alpha=0.5))

    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"\n✓ Plot saved to: {output_path}")

    return fig

def print_summary_table(baseline_aoi_stats, ctac_aoi_stats,
                       baseline_cbr_df, ctac_cbr_df):
    """Print detailed comparison table."""

    print("\n" + "="*80)
    print("CTAC PERFORMANCE COMPARISON SUMMARY")
    print("="*80)

    # AoI Summary
    print("\n1. AGE OF INFORMATION (AoI) - Peak AoI in milliseconds")
    print("-" * 80)
    print(f"{'Metric':<15} {'Baseline (ECDSA)':<25} {'CTAC':<25} {'Change':<15}")
    print("-" * 80)

    metrics = ['min', 'p25', 'median', 'p75', 'p90', 'p95', 'p99', 'max', 'mean']
    metric_names = ['Min', 'P25', 'Median (P50)', 'P75', 'P90', 'P95', 'P99', 'Max', 'Mean']

    for metric, name in zip(metrics, metric_names):
        baseline_val = baseline_aoi_stats[metric]
        ctac_val = ctac_aoi_stats[metric]
        change_pct = ((ctac_val - baseline_val) / baseline_val * 100) if baseline_val > 0 else 0

        print(f"{name:<15} {baseline_val:>10.1f} ms          {ctac_val:>10.1f} ms          "
              f"{change_pct:>+6.1f}%")

    print(f"\nSamples:        {baseline_aoi_stats['samples']:>10,}            "
          f"{ctac_aoi_stats['samples']:>10,}")

    # CBR Summary
    print("\n2. CHANNEL BUSY RATIO (CBR) - Percentage")
    print("-" * 80)

    baseline_cbr_mean = baseline_cbr_df[baseline_cbr_df['metric'] == 'cbr:mean']['value'] * 100
    ctac_cbr_mean = ctac_cbr_df[ctac_cbr_df['metric'] == 'cbr:mean']['value'] * 100

    baseline_cbr_stats = {
        'min': baseline_cbr_mean.min(),
        'median': baseline_cbr_mean.median(),
        'max': baseline_cbr_mean.max(),
        'mean': baseline_cbr_mean.mean()
    }

    ctac_cbr_stats = {
        'min': ctac_cbr_mean.min(),
        'median': ctac_cbr_mean.median(),
        'max': ctac_cbr_mean.max(),
        'mean': ctac_cbr_mean.mean()
    }

    print(f"{'Metric':<15} {'Baseline (ECDSA)':<25} {'CTAC':<25} {'Change':<15}")
    print("-" * 80)

    for metric in ['min', 'median', 'max', 'mean']:
        baseline_val = baseline_cbr_stats[metric]
        ctac_val = ctac_cbr_stats[metric]
        change_pct = ((ctac_val - baseline_val) / baseline_val * 100) if baseline_val > 0 else 0

        name = metric.capitalize()
        print(f"{name:<15} {baseline_val:>10.2f} %           {ctac_val:>10.2f} %           "
              f"{change_pct:>+6.1f}%")

    print(f"\nVehicles:       {len(baseline_cbr_mean):>10}             {len(ctac_cbr_mean):>10}")

    # Overall summary
    print("\n" + "="*80)
    print("KEY FINDINGS")
    print("="*80)

    aoi_median_change = ((ctac_aoi_stats['median'] - baseline_aoi_stats['median']) /
                         baseline_aoi_stats['median'] * 100)
    cbr_median_reduction = ((baseline_cbr_stats['median'] - ctac_cbr_stats['median']) /
                            baseline_cbr_stats['median'] * 100)

    print(f"\n✓ CBR Reduction (median):      {cbr_median_reduction:>6.1f}%")
    print(f"✓ AoI Change (median):         {aoi_median_change:>+6.1f}%")

    # Safety check
    if ctac_aoi_stats['p95'] < 300:
        print(f"✓ 95% of AoI samples < 300ms:  PASS (AoI bound maintained)")
    else:
        print(f"⚠ 95% of AoI samples > 300ms:  {ctac_aoi_stats['p95']:.1f}ms (exceeds bound)")

    if ctac_cbr_stats['median'] < 30:
        print(f"✓ Median CBR < 30%:            PASS (below DCC Level 1)")
    else:
        print(f"⚠ Median CBR ≥ 30%:            {ctac_cbr_stats['median']:.1f}% (DCC triggered)")

    print("\n" + "="*80)

def main():
    """Main analysis function."""

    # Paths (adjust as needed)
    base_dir = Path("../simulations/Mode4")

    # Scenario paths - comparing DCC baseline vs CTAC
    baseline_v2v = base_dir / "simulation_logs__27_ECDSA_F_BURST_DCC" / "v2v_logs.csv"
    ctac_v2v = base_dir / "simulation_logs__26_CTAC_F_BURST" / "v2v_logs.csv"

    baseline_sca = base_dir / "results" / "_27_ECDSA_F_BURST_DCC-#0.sca"
    ctac_sca = base_dir / "results" / "_26_CTAC_F_BURST-#0.sca"

    # Check files exist
    for path in [baseline_v2v, ctac_v2v, baseline_sca, ctac_sca]:
        if not path.exists():
            print(f"ERROR: File not found: {path}")
            sys.exit(1)

    print("="*80)
    print("CTAC PERFORMANCE ANALYSIS")
    print("="*80)
    print(f"\nAnalyzing scenarios:")
    print(f"  Baseline: {baseline_v2v.parent.name}")
    print(f"  CTAC:     {ctac_v2v.parent.name}")

    # Compute AoI metrics
    print("\n" + "-"*80)
    print("COMPUTING AGE OF INFORMATION (AoI)")
    print("-"*80)

    baseline_aoi_df = compute_aoi_metrics(baseline_v2v, max_distance_m=300)
    baseline_aoi_stats = compute_aoi_stats(baseline_aoi_df)

    ctac_aoi_df = compute_aoi_metrics(ctac_v2v, max_distance_m=300)
    ctac_aoi_stats = compute_aoi_stats(ctac_aoi_df)

    # Extract CBR metrics
    print("\n" + "-"*80)
    print("EXTRACTING CHANNEL BUSY RATIO (CBR)")
    print("-"*80)

    baseline_cbr_df = extract_cbr_from_sca(baseline_sca)
    ctac_cbr_df = extract_cbr_from_sca(ctac_sca)

    # Print summary
    print_summary_table(baseline_aoi_stats, ctac_aoi_stats,
                       baseline_cbr_df, ctac_cbr_df)

    # Generate plots
    print("\n" + "-"*80)
    print("GENERATING VISUALIZATION")
    print("-"*80)

    output_path = base_dir / "ctac_comparison_plot.png"
    plot_comparison(baseline_aoi_stats, ctac_aoi_stats,
                   baseline_cbr_df, ctac_cbr_df,
                   output_path=str(output_path))

    print("\n✓ Analysis complete!")
    print(f"✓ Results saved to: {output_path}")

    return 0

if __name__ == "__main__":
    sys.exit(main())
