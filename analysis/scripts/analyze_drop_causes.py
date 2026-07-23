#!/usr/bin/env python3
"""
Analyze the causes of MAC layer packet drops.
Since there's no explicit signal tracking all drop causes, we infer them from:
  Actual Drops = App BSMs Generated - PHY Transmitted
"""

import sys
import pandas as pd
from pathlib import Path
from collections import defaultdict

def extract_vehicle_stats(sca_path, sender_summary_path):
    """Extract all relevant statistics per vehicle."""
    vehicle_stats = defaultdict(lambda: {
        'opportunities': 0,  # from sender_summary (bsmSeq count)
        'ctac_deferred': 0,
        'app_generated': 0,  # opportunities - deferred
        'phy_transmitted': 0,  # tbSent
        'dcc_drops': 0,
        'timeout_drops': 0,
        'actual_drops': 0  # app_generated - phy_transmitted
    })

    # Load sender_summary.csv for opportunities
    sender_df = pd.read_csv(sender_summary_path)
    for _, row in sender_df.iterrows():
        vehicle = row['sender']
        vehicle_stats[vehicle]['opportunities'] = row['total_sent']

    # Extract signals from .sca file
    with open(sca_path, 'r') as f:
        for line in f:
            if 'carNoIp[' not in line or 'scalar' not in line:
                continue

            parts = line.strip().split()
            if len(parts) < 4:
                continue

            # Extract vehicle ID
            module = parts[1]
            vehicle = None
            for part in module.split('.'):
                if 'carNoIp[' in part:
                    vehicle = part
                    break

            if vehicle is None:
                continue

            try:
                value = int(parts[3])
            except:
                continue

            # Extract relevant signals
            if 'ctacDeferred:sum' in line:
                vehicle_stats[vehicle]['ctac_deferred'] = value
            elif 'tbSent:sum' in line:
                vehicle_stats[vehicle]['phy_transmitted'] = value
            elif 'packetDropDCC:sum' in line:
                vehicle_stats[vehicle]['dcc_drops'] = value
            elif 'droppedTimeout:sum' in line:
                vehicle_stats[vehicle]['timeout_drops'] = value

    # Calculate app_generated and actual_drops
    for vehicle, stats in vehicle_stats.items():
        stats['app_generated'] = stats['opportunities'] - stats['ctac_deferred']
        stats['actual_drops'] = stats['app_generated'] - stats['phy_transmitted']

    return vehicle_stats

def analyze_drops(folder_path, scenario_name):
    """Analyze drop causes for a scenario."""
    folder_path = Path(folder_path)

    # Find .sca file
    sca_files = list(folder_path.glob('*.sca'))
    if not sca_files:
        print(f"No .sca file found in {folder_path}")
        return

    sca_path = sca_files[0]
    sender_summary_path = folder_path / 'sender_summary.csv'

    if not sender_summary_path.exists():
        print(f"sender_summary.csv not found in {folder_path}")
        return

    # Extract statistics
    vehicle_stats = extract_vehicle_stats(sca_path, sender_summary_path)

    # Aggregate statistics
    total_opportunities = sum(v['opportunities'] for v in vehicle_stats.values())
    total_deferred = sum(v['ctac_deferred'] for v in vehicle_stats.values())
    total_app_generated = sum(v['app_generated'] for v in vehicle_stats.values())
    total_phy_transmitted = sum(v['phy_transmitted'] for v in vehicle_stats.values())
    total_actual_drops = sum(v['actual_drops'] for v in vehicle_stats.values())
    total_dcc_drops = sum(v['dcc_drops'] for v in vehicle_stats.values())
    total_timeout_drops = sum(v['timeout_drops'] for v in vehicle_stats.values())

    # Calculate unaccounted drops
    accounted_drops = total_dcc_drops + total_timeout_drops
    unaccounted_drops = total_actual_drops - accounted_drops

    # Print results
    print("=" * 80)
    print(f"DROP CAUSE ANALYSIS: {scenario_name}")
    print("=" * 80)
    print()
    print(f"Total Vehicles: {len(vehicle_stats)}")
    print(f"Transmission Opportunities: {total_opportunities:,}")
    print(f"CTAC Deferred (gated): {total_deferred:,}")
    print(f"App BSMs Generated: {total_app_generated:,}")
    print(f"PHY Transmitted (tbSent): {total_phy_transmitted:,}")
    print()
    print("-" * 80)
    print("DROP BREAKDOWN:")
    print("-" * 80)
    print(f"Total Actual Drops: {total_actual_drops:,} ({total_actual_drops/total_app_generated*100:.2f}%)")
    print(f"  - DCC Drops: {total_dcc_drops:,} ({total_dcc_drops/total_actual_drops*100 if total_actual_drops > 0 else 0:.1f}% of drops)")
    print(f"  - Timeout Drops: {total_timeout_drops:,} ({total_timeout_drops/total_actual_drops*100 if total_actual_drops > 0 else 0:.1f}% of drops)")
    print(f"  - Unaccounted Drops: {unaccounted_drops:,} ({unaccounted_drops/total_actual_drops*100 if total_actual_drops > 0 else 0:.1f}% of drops)")
    print()
    print("LIKELY CAUSES OF UNACCOUNTED DROPS:")
    print("  1. MAC buffer overflow (queue full)")
    print("  2. SPS resource allocation failures")
    print("  3. Other untracked MAC layer issues")
    print()

    # Analyze per-vehicle drops to find patterns
    vehicles_with_drops = [(v, s) for v, s in vehicle_stats.items() if s['actual_drops'] > 0]
    vehicles_with_drops.sort(key=lambda x: x[1]['actual_drops'], reverse=True)

    print("-" * 80)
    print(f"VEHICLES WITH DROPS: {len(vehicles_with_drops)} out of {len(vehicle_stats)}")
    print("-" * 80)
    print(f"{'Vehicle':<15} {'Generated':>10} {'TX':>10} {'Drops':>10} {'Drop %':>10}")
    print("-" * 80)

    for vehicle, stats in vehicles_with_drops[:20]:  # Show top 20
        drop_pct = stats['actual_drops'] / stats['app_generated'] * 100 if stats['app_generated'] > 0 else 0
        print(f"{vehicle:<15} {stats['app_generated']:>10} {stats['phy_transmitted']:>10} {stats['actual_drops']:>10} {drop_pct:>9.2f}%")

    if len(vehicles_with_drops) > 20:
        print(f"... and {len(vehicles_with_drops) - 20} more vehicles with drops")

    print()
    print("=" * 80)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 analyze_drop_causes.py <folder_path> <scenario_name>")
        sys.exit(1)

    folder_path = sys.argv[1]
    scenario_name = sys.argv[2]

    analyze_drops(folder_path, scenario_name)
