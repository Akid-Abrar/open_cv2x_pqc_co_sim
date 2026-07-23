#!/usr/bin/env python3
"""
Compare MAC layer drop statistics across baseline and CTAC scenarios.
Usage: python3 compare_mac_drops.py <baseline_folder> <ctac_k2_folder> <ctac_k4_folder> [output_file]
"""

import sys
import pandas as pd
import numpy as np
from pathlib import Path
from collections import defaultdict

def extract_mac_drops_per_vehicle(sca_path):
    """Extract MAC drop statistics for each vehicle from .sca file."""
    vehicle_stats = defaultdict(lambda: {
        'app_sent': 0,
        'dcc_drops': 0,
        'missed_tx': 0,
        'timeout_drops': 0,
        'grant_breaks': 0,
        'phy_sent': 0,
        'ctac_deferred': 0
    })

    with open(sca_path, 'r') as f:
        for line in f:
            if 'carNoIp[' not in line:
                continue

            # Extract vehicle ID
            parts = line.strip().split()
            if len(parts) < 4:
                continue

            module = parts[1]
            vehicle = None
            for part in module.split('.'):
                if 'carNoIp[' in part:
                    vehicle = part
                    break

            if vehicle is None:
                continue

            value = None
            try:
                value = int(parts[3])
            except:
                continue

            # Extract relevant statistics
            if 'packetDropDCC:sum' in line:
                vehicle_stats[vehicle]['dcc_drops'] = value
            elif 'missedTransmission:sum' in line:
                vehicle_stats[vehicle]['missed_tx'] = value
            elif 'droppedTimeout:sum' in line:
                vehicle_stats[vehicle]['timeout_drops'] = value
            elif 'grantBreak:sum' in line:
                vehicle_stats[vehicle]['grant_breaks'] = value
            elif 'lteNic.phy tbSent:sum' in line:
                vehicle_stats[vehicle]['phy_sent'] = value
            elif 'ctacDeferred:sum' in line:
                vehicle_stats[vehicle]['ctac_deferred'] = value

    return vehicle_stats

def load_sender_summary(folder_path):
    """Load application-layer BSM counts from sender_summary.csv."""
    csv_path = folder_path / 'sender_summary.csv'
    df = pd.read_csv(csv_path)
    sender_dict = {}
    for _, row in df.iterrows():
        sender_dict[row['sender']] = row['total_sent']
    return sender_dict

def compute_drop_statistics(folder_path):
    """Compute aggregate drop statistics for a scenario."""
    # Load MAC drop stats from .sca
    sca_file = list(folder_path.glob('*.sca'))[0]
    vehicle_stats = extract_mac_drops_per_vehicle(sca_file)

    # Load application-layer BSM counts
    sender_summary = load_sender_summary(folder_path)

    # Merge data and calculate actual BSMs generated
    for vehicle, bsm_count in sender_summary.items():
        if vehicle in vehicle_stats:
            # Actual BSMs generated = sender_summary - ctacDeferred
            # (sender_summary counts opportunities, not actual BSMs)
            actual_bsm = bsm_count - vehicle_stats[vehicle]['ctac_deferred']
            vehicle_stats[vehicle]['app_sent'] = actual_bsm

    # Calculate total drops per vehicle
    drop_data = []
    for vehicle, stats in vehicle_stats.items():
        if stats['app_sent'] > 0:
            # CORRECT: Total drops = App Generated - PHY Transmitted
            # grantBreak and missedTransmission are NOT packet drops
            actual_drops = stats['app_sent'] - stats['phy_sent']

            drop_data.append({
                'vehicle': vehicle,
                'app_sent': stats['app_sent'],
                'ctac_deferred': stats['ctac_deferred'],
                'dcc_drops': stats['dcc_drops'],
                'missed_tx': stats['missed_tx'],
                'timeout_drops': stats['timeout_drops'],
                'grant_breaks': stats['grant_breaks'],
                'total_drops': actual_drops,
                'phy_sent': stats['phy_sent'],
                'drop_rate': actual_drops / stats['app_sent'] if stats['app_sent'] > 0 else 0
            })

    df = pd.DataFrame(drop_data)

    # Aggregate statistics
    return {
        'num_vehicles': len(df),
        'total_app_sent': df['app_sent'].sum(),
        'total_phy_sent': df['phy_sent'].sum(),
        'total_ctac_deferred': df['ctac_deferred'].sum(),
        'total_dcc_drops': df['dcc_drops'].sum(),
        'total_missed_tx': df['missed_tx'].sum(),
        'total_timeout_drops': df['timeout_drops'].sum(),
        'total_grant_breaks': df['grant_breaks'].sum(),
        'total_drops': df['total_drops'].sum(),
        'avg_drop_rate': df['drop_rate'].mean(),
        'median_drop_rate': df['drop_rate'].median(),
        'vehicles_with_drops': (df['total_drops'] > 0).sum(),
        'df': df
    }

def print_comparison(baseline_folder, ctac_k2_folder, ctac_k4_folder, output_file=None):
    """Print comparison of MAC drop statistics."""
    baseline_folder = Path(baseline_folder)
    ctac_k2_folder = Path(ctac_k2_folder)
    ctac_k4_folder = Path(ctac_k4_folder)

    # Set up output
    if output_file:
        f_out = open(output_file, 'w')
        original_stdout = sys.stdout
        sys.stdout = f_out

    print("="*80)
    print("MAC LAYER DROP STATISTICS COMPARISON")
    print("="*80)
    print()
    print("Scenarios:")
    print(f"  Baseline: {baseline_folder.name} (DCC enabled)")
    print(f"  CTAC K=2: {ctac_k2_folder.name} (2 cohorts, 180° sectors)")
    print(f"  CTAC K=4: {ctac_k4_folder.name} (4 cohorts, 90° sectors)")
    print()

    # Compute statistics for each scenario
    print("Computing drop statistics...")
    baseline_stats = compute_drop_statistics(baseline_folder)
    ctac_k2_stats = compute_drop_statistics(ctac_k2_folder)
    ctac_k4_stats = compute_drop_statistics(ctac_k4_folder)
    print()

    print("="*80)
    print("AGGREGATE DROP STATISTICS")
    print("="*80)
    print()

    # Total packets
    print("TOTAL PACKET COUNTS:")
    print("-"*80)
    print(f"{'Metric':<30} {'Baseline':>15} {'K=2 CTAC':>15} {'K=4 CTAC':>15}")
    print("-"*80)
    print(f"{'Vehicles':<30} {baseline_stats['num_vehicles']:>15} {ctac_k2_stats['num_vehicles']:>15} {ctac_k4_stats['num_vehicles']:>15}")
    print(f"{'CTAC Deferred (gated)':<30} {baseline_stats['total_ctac_deferred']:>15,} {ctac_k2_stats['total_ctac_deferred']:>15,} {ctac_k4_stats['total_ctac_deferred']:>15,}")
    print(f"{'App BSMs Generated':<30} {baseline_stats['total_app_sent']:>15,} {ctac_k2_stats['total_app_sent']:>15,} {ctac_k4_stats['total_app_sent']:>15,}")
    print(f"{'PHY TBs Transmitted':<30} {baseline_stats['total_phy_sent']:>15,} {ctac_k2_stats['total_phy_sent']:>15,} {ctac_k4_stats['total_phy_sent']:>15,}")
    print(f"{'ACTUAL DROPS (App-PHY)':<30} {baseline_stats['total_drops']:>15,} {ctac_k2_stats['total_drops']:>15,} {ctac_k4_stats['total_drops']:>15,}")
    print()

    # Drop breakdown
    print("MAC LAYER SIGNAL BREAKDOWN:")
    print("-"*80)
    print(f"{'Signal Type':<30} {'Baseline':>15} {'K=2 CTAC':>15} {'K=4 CTAC':>15}")
    print("-"*80)
    print(f"{'DCC Drops (actual)':<30} {baseline_stats['total_dcc_drops']:>15,} {ctac_k2_stats['total_dcc_drops']:>15,} {ctac_k4_stats['total_dcc_drops']:>15,}")
    print(f"{'Grant Breaks (info)':<30} {baseline_stats['total_grant_breaks']:>15,} {ctac_k2_stats['total_grant_breaks']:>15,} {ctac_k4_stats['total_grant_breaks']:>15,}")
    print(f"{'Missed TX (info)':<30} {baseline_stats['total_missed_tx']:>15,} {ctac_k2_stats['total_missed_tx']:>15,} {ctac_k4_stats['total_missed_tx']:>15,}")
    print(f"{'Timeout Drops (disabled)':<30} {baseline_stats['total_timeout_drops']:>15,} {ctac_k2_stats['total_timeout_drops']:>15,} {ctac_k4_stats['total_timeout_drops']:>15,}")
    print()
    print("Note: Only DCC Drops are actual packet drops.")
    print("      Grant Breaks = SPS grant expirations (triggers reselection)")
    print("      Missed TX = Missed transmission slots (informational)")
    print("      Other drops = ACTUAL DROPS - DCC Drops (queue overflow, etc.)")
    print()

    # Drop rates
    print("DROP RATES:")
    print("-"*80)
    print(f"{'Metric':<30} {'Baseline':>15} {'K=2 CTAC':>15} {'K=4 CTAC':>15}")
    print("-"*80)

    baseline_rate = baseline_stats['total_drops'] / baseline_stats['total_app_sent'] * 100
    k2_rate = ctac_k2_stats['total_drops'] / ctac_k2_stats['total_app_sent'] * 100
    k4_rate = ctac_k4_stats['total_drops'] / ctac_k4_stats['total_app_sent'] * 100

    print(f"{'Overall Drop Rate':<30} {baseline_rate:>14.2f}% {k2_rate:>14.2f}% {k4_rate:>14.2f}%")
    print(f"{'Avg Drop Rate/Vehicle':<30} {baseline_stats['avg_drop_rate']*100:>14.2f}% {ctac_k2_stats['avg_drop_rate']*100:>14.2f}% {ctac_k4_stats['avg_drop_rate']*100:>14.2f}%")
    print(f"{'Median Drop Rate/Vehicle':<30} {baseline_stats['median_drop_rate']*100:>14.2f}% {ctac_k2_stats['median_drop_rate']*100:>14.2f}% {ctac_k4_stats['median_drop_rate']*100:>14.2f}%")
    print(f"{'Vehicles with Drops':<30} {baseline_stats['vehicles_with_drops']:>15,} {ctac_k2_stats['vehicles_with_drops']:>15,} {ctac_k4_stats['vehicles_with_drops']:>15,}")
    print()

    # DCC-specific analysis
    print("="*80)
    print("DCC DROP ANALYSIS")
    print("="*80)
    print()
    print(f"{'Scenario':<20} {'DCC Drops':>15} {'% of Total Drops':>20} {'% of App Sent':>20}")
    print("-"*80)

    baseline_dcc_pct = baseline_stats['total_dcc_drops'] / baseline_stats['total_drops'] * 100 if baseline_stats['total_drops'] > 0 else 0
    baseline_dcc_app = baseline_stats['total_dcc_drops'] / baseline_stats['total_app_sent'] * 100

    k2_dcc_pct = ctac_k2_stats['total_dcc_drops'] / ctac_k2_stats['total_drops'] * 100 if ctac_k2_stats['total_drops'] > 0 else 0
    k2_dcc_app = ctac_k2_stats['total_dcc_drops'] / ctac_k2_stats['total_app_sent'] * 100

    k4_dcc_pct = ctac_k4_stats['total_dcc_drops'] / ctac_k4_stats['total_drops'] * 100 if ctac_k4_stats['total_drops'] > 0 else 0
    k4_dcc_app = ctac_k4_stats['total_dcc_drops'] / ctac_k4_stats['total_app_sent'] * 100

    print(f"{'Baseline (DCC)':<20} {baseline_stats['total_dcc_drops']:>15,} {baseline_dcc_pct:>19.1f}% {baseline_dcc_app:>19.2f}%")
    print(f"{'CTAC K=2':<20} {ctac_k2_stats['total_dcc_drops']:>15,} {k2_dcc_pct:>19.1f}% {k2_dcc_app:>19.2f}%")
    print(f"{'CTAC K=4':<20} {ctac_k4_stats['total_dcc_drops']:>15,} {k4_dcc_pct:>19.1f}% {k4_dcc_app:>19.2f}%")
    print()

    print("Note: CTAC does not use DCC (dccMechanism = false)")
    print("      DCC drops in CTAC scenarios should be 0 or near-0")
    print()

    # Comparison summary
    print("="*80)
    print("KEY FINDINGS")
    print("="*80)
    print()

    k2_vs_baseline = (ctac_k2_stats['total_drops'] - baseline_stats['total_drops']) / baseline_stats['total_drops'] * 100
    k4_vs_baseline = (ctac_k4_stats['total_drops'] - baseline_stats['total_drops']) / baseline_stats['total_drops'] * 100

    print(f"Total Drops:")
    print(f"  Baseline:  {baseline_stats['total_drops']:>8,} drops ({baseline_rate:>5.2f}% of app sent)")
    print(f"  K=2 CTAC:  {ctac_k2_stats['total_drops']:>8,} drops ({k2_rate:>5.2f}% of app sent) [{k2_vs_baseline:+.1f}% vs baseline]")
    print(f"  K=4 CTAC:  {ctac_k4_stats['total_drops']:>8,} drops ({k4_rate:>5.2f}% of app sent) [{k4_vs_baseline:+.1f}% vs baseline]")
    print()

    print(f"Drop Rate per Vehicle:")
    print(f"  Baseline:  {baseline_stats['avg_drop_rate']*100:>5.2f}% average, {baseline_stats['median_drop_rate']*100:>5.2f}% median")
    print(f"  K=2 CTAC:  {ctac_k2_stats['avg_drop_rate']*100:>5.2f}% average, {ctac_k2_stats['median_drop_rate']*100:>5.2f}% median")
    print(f"  K=4 CTAC:  {ctac_k4_stats['avg_drop_rate']*100:>5.2f}% average, {ctac_k4_stats['median_drop_rate']*100:>5.2f}% median")
    print()

    print("="*80)

    # Restore stdout if redirected
    if output_file:
        sys.stdout = original_stdout
        f_out.close()
        print(f"\nResults saved to: {output_file}")

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python3 compare_mac_drops.py <baseline_folder> <ctac_k2_folder> <ctac_k4_folder> [output_file]")
        sys.exit(1)

    baseline_folder = sys.argv[1]
    ctac_k2_folder = sys.argv[2]
    ctac_k4_folder = sys.argv[3]
    output_file = sys.argv[4] if len(sys.argv) > 4 else None

    print_comparison(baseline_folder, ctac_k2_folder, ctac_k4_folder, output_file)
