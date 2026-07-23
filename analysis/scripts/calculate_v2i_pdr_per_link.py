#!/usr/bin/env python3
"""
Calculate Per-Link PDR for V2I Communication

Analyzes V2I (Vehicle-to-Infrastructure) communication from appl_logs.csv.
Computes PDR for each (sender, receiver) link where one endpoint is an RSU.

Two PDR calculation methods:
  1. Overall PDR: total_received / total_expected * 100
  2. Sliding Window PDR (5GAA): median PRR across time windows

Usage:
    python3 calculate_v2i_pdr_per_link.py <scenario_folder> [output_csv]

Example:
    python3 calculate_v2i_pdr_per_link.py \
        "../../Paper Writeup/Simulation Logs/1_ECDSA_A_LOS" \
        v2i_pdr_results.csv
"""

import sys
import pandas as pd
import numpy as np
from pathlib import Path

# Configuration
WINDOW_SEC = 5.0      # 5GAA sliding window (±2.5s)
MIN_PKTS = 5          # Minimum packets in window for valid PRR
RSU_PATTERN = 'rsu'   # Pattern to identify RSU nodes


def is_rsu(node_id):
    """Check if a node is an RSU."""
    return RSU_PATTERN in str(node_id).lower()


def is_v2i_link(sender, receiver):
    """Check if link is V2I (one endpoint is RSU)."""
    return is_rsu(sender) or is_rsu(receiver)


def calculate_overall_pdr(df):
    """Calculate overall PDR: received / expected."""
    if len(df) == 0:
        return 0.0

    # Expected packets based on msgId range
    msg_ids = df['msgId'].values
    min_id = msg_ids.min()
    max_id = msg_ids.max()
    expected = int(max_id - min_id + 1)
    received = len(msg_ids)

    if expected <= 0:
        return 0.0

    pdr = (received / expected) * 100.0
    return min(pdr, 100.0)  # Cap at 100%


def calculate_sliding_window_pdr(df):
    """
    Calculate PDR using 5GAA sliding window method.
    Returns median PRR across all time windows.
    """
    if len(df) < MIN_PKTS:
        return 0.0

    df = df.sort_values('t').reset_index(drop=True)
    t_vals = df['t'].values
    id_vals = df['msgId'].values

    half_w = WINDOW_SEC / 2.0
    prr_samples = []

    for idx in range(len(t_vals)):
        t_center = t_vals[idx]

        # Find packets in window [t-2.5s, t+2.5s]
        lo = np.searchsorted(t_vals, t_center - half_w, side='left')
        hi = np.searchsorted(t_vals, t_center + half_w, side='right')

        win_ids = id_vals[lo:hi]
        if len(win_ids) < MIN_PKTS:
            continue

        # Calculate PRR for this window
        span = int(win_ids.max() - win_ids.min() + 1)
        if span <= 0:
            continue

        prr = (len(win_ids) / span) * 100.0
        prr_samples.append(prr)

    if len(prr_samples) == 0:
        return 0.0

    return np.median(prr_samples)


def calculate_link_statistics(link_df):
    """Calculate comprehensive statistics for a link."""
    stats = {
        'packets_received': len(link_df),
        'min_msgId': link_df['msgId'].min(),
        'max_msgId': link_df['msgId'].max(),
        'packets_expected': int(link_df['msgId'].max() - link_df['msgId'].min() + 1),
        'mean_distance_m': link_df['dist_m'].mean(),
        'median_distance_m': link_df['dist_m'].median(),
        'min_distance_m': link_df['dist_m'].min(),
        'max_distance_m': link_df['dist_m'].max(),
        'mean_delay_ms': link_df['delay_ms'].mean(),
        'median_delay_ms': link_df['delay_ms'].median(),
        'simulation_duration_s': link_df['t'].max() - link_df['t'].min(),
    }

    # Calculate PDRs
    stats['pdr_overall'] = calculate_overall_pdr(link_df)
    stats['pdr_sliding_window'] = calculate_sliding_window_pdr(link_df)

    return stats


def analyze_v2i_links(csv_path):
    """Analyze all V2I links in the appl_logs.csv file."""
    print(f"Loading: {csv_path}")
    df = pd.read_csv(csv_path)

    # Filter out unknown senders
    df = df[df['sender'] != 'unknown'].copy()

    print(f"Total records: {len(df):,}")

    # Identify V2I links
    v2i_df = df[df.apply(lambda row: is_v2i_link(row['sender'], row['receiver']), axis=1)]

    print(f"V2I records: {len(v2i_df):,}")

    if len(v2i_df) == 0:
        print("WARNING: No V2I communication found in this scenario!")
        return pd.DataFrame()

    # Group by link (sender, receiver)
    results = []

    for (sender, receiver), link_df in v2i_df.groupby(['sender', 'receiver']):
        # Determine link direction
        if is_rsu(sender):
            direction = 'I2V'  # Infrastructure to Vehicle
            rsu = sender
            vehicle = receiver
        else:
            direction = 'V2I'  # Vehicle to Infrastructure
            rsu = receiver
            vehicle = sender

        stats = calculate_link_statistics(link_df)

        result = {
            'direction': direction,
            'rsu': rsu,
            'vehicle': vehicle,
            'sender': sender,
            'receiver': receiver,
            **stats
        }

        results.append(result)

    results_df = pd.DataFrame(results)

    # Sort by direction, then PDR
    results_df = results_df.sort_values(['direction', 'pdr_overall'], ascending=[True, False])

    return results_df


def print_summary(results_df):
    """Print summary statistics."""
    if len(results_df) == 0:
        print("\nNo V2I links found.")
        return

    print("\n" + "="*80)
    print("V2I PER-LINK PDR ANALYSIS SUMMARY")
    print("="*80)

    # Overall statistics
    total_links = len(results_df)
    v2i_links = len(results_df[results_df['direction'] == 'V2I'])
    i2v_links = len(results_df[results_df['direction'] == 'I2V'])

    print(f"\nTotal V2I Links: {total_links:,}")
    print(f"  - V2I (Vehicle → RSU): {v2i_links:,}")
    print(f"  - I2V (RSU → Vehicle): {i2v_links:,}")

    # PDR statistics
    print("\n" + "-"*80)
    print("PDR STATISTICS (Overall Method)")
    print("-"*80)

    for direction in ['V2I', 'I2V']:
        subset = results_df[results_df['direction'] == direction]
        if len(subset) == 0:
            continue

        print(f"\n{direction} Links ({len(subset)} links):")
        print(f"  Mean PDR:   {subset['pdr_overall'].mean():.2f}%")
        print(f"  Median PDR: {subset['pdr_overall'].median():.2f}%")
        print(f"  Min PDR:    {subset['pdr_overall'].min():.2f}%")
        print(f"  Max PDR:    {subset['pdr_overall'].max():.2f}%")
        print(f"  Std Dev:    {subset['pdr_overall'].std():.2f}%")

    # Distance statistics
    print("\n" + "-"*80)
    print("DISTANCE STATISTICS")
    print("-"*80)
    print(f"  Mean Distance:   {results_df['mean_distance_m'].mean():.1f} m")
    print(f"  Median Distance: {results_df['median_distance_m'].median():.1f} m")
    print(f"  Max Distance:    {results_df['max_distance_m'].max():.1f} m")

    # Top 10 best and worst links
    print("\n" + "-"*80)
    print("TOP 10 BEST LINKS (Highest PDR)")
    print("-"*80)
    print(f"{'Direction':<8} {'Sender':<15} {'Receiver':<15} {'PDR':>8} {'Packets':>8} {'Distance':>10}")
    print("-"*80)

    for _, row in results_df.head(10).iterrows():
        print(f"{row['direction']:<8} {row['sender']:<15} {row['receiver']:<15} "
              f"{row['pdr_overall']:>7.2f}% {row['packets_received']:>8} "
              f"{row['mean_distance_m']:>9.1f}m")

    print("\n" + "-"*80)
    print("TOP 10 WORST LINKS (Lowest PDR)")
    print("-"*80)
    print(f"{'Direction':<8} {'Sender':<15} {'Receiver':<15} {'PDR':>8} {'Packets':>8} {'Distance':>10}")
    print("-"*80)

    for _, row in results_df.tail(10).iterrows():
        print(f"{row['direction']:<8} {row['sender']:<15} {row['receiver']:<15} "
              f"{row['pdr_overall']:>7.2f}% {row['packets_received']:>8} "
              f"{row['mean_distance_m']:>9.1f}m")

    print("\n" + "="*80)


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 calculate_v2i_pdr_per_link.py <scenario_folder> [output_csv]")
        print("\nExample:")
        print('  python3 calculate_v2i_pdr_per_link.py \\')
        print('      "../../Paper Writeup/Simulation Logs/1_ECDSA_A_LOS" \\')
        print('      v2i_pdr_results.csv')
        sys.exit(1)

    scenario_folder = Path(sys.argv[1])
    output_csv = sys.argv[2] if len(sys.argv) > 2 else None

    # Find appl_logs.csv
    csv_path = scenario_folder / "logs" / "appl_logs.csv"
    if not csv_path.exists():
        csv_path = scenario_folder / "appl_logs.csv"

    if not csv_path.exists():
        print(f"ERROR: Cannot find appl_logs.csv in {scenario_folder}")
        sys.exit(1)

    # Analyze V2I links
    results_df = analyze_v2i_links(csv_path)

    # Print summary
    print_summary(results_df)

    # Save results
    if output_csv and len(results_df) > 0:
        output_path = Path(output_csv)
        results_df.to_csv(output_path, index=False)
        print(f"\nResults saved to: {output_path}")
        print(f"Total links exported: {len(results_df):,}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
