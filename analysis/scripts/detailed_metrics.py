#!/usr/bin/env python3
"""
Compute detailed metrics: IPG and PDR
"""

import pandas as pd
import numpy as np
from pathlib import Path

def compute_aoi_and_ipg(csv_path, max_distance_m=300):
    """Compute AoI and IPG metrics."""
    df = pd.read_csv(csv_path)
    df['gen_time'] = df['t'] - (df['delay_ms'] / 1000.0)

    ipg_samples = []

    grouped = df.groupby(['receiver', 'sender'])

    for (receiver, sender), group in grouped:
        group = group.sort_values('t').reset_index(drop=True)

        if len(group) < 2:
            continue

        for i in range(1, len(group)):
            curr = group.iloc[i]
            prev = group.iloc[i-1]

            if curr['dist_m'] < max_distance_m and prev['dist_m'] < max_distance_m:
                ipg_ms = (curr['t'] - prev['t']) * 1000.0

                ipg_samples.append({
                    'ipg_ms': ipg_ms
                })

    return pd.DataFrame(ipg_samples)

def compute_pdr(v2v_log_path, sender_summary_path, max_distance_m=300):
    """
    Compute Packet Delivery Ratio (PDR) per link.

    For each (sender, receiver) pair:
    - Count receptions within max_distance_m
    - PDR = receptions / total_packets_sent_by_sender

    Returns per-link PDR values.
    """
    # Read receptions
    v2v_df = pd.read_csv(v2v_log_path)

    # Filter by distance
    v2v_df = v2v_df[v2v_df['dist_m'] < max_distance_m]

    # Read transmissions
    sender_df = pd.read_csv(sender_summary_path)

    # Count receptions per (sender, receiver) link
    link_rx = v2v_df.groupby(['sender', 'receiver']).size().reset_index(name='received')

    # Merge with total sent
    link_pdr = link_rx.merge(sender_df, on='sender', how='left')

    # Compute PDR per link
    link_pdr['pdr'] = link_pdr['received'] / link_pdr['total_sent']

    return link_pdr

def print_stats(data, metric_name, unit=""):
    """Print statistics for a metric."""
    print(f"\n{metric_name}")
    print("-" * 80)
    print(f"Min:      {data.min():>10.2f} {unit}")
    print(f"P25:      {data.quantile(0.25):>10.2f} {unit}")
    print(f"Median:   {data.quantile(0.50):>10.2f} {unit}")
    print(f"P75:      {data.quantile(0.75):>10.2f} {unit}")
    print(f"P90:      {data.quantile(0.90):>10.2f} {unit}")
    print(f"P95:      {data.quantile(0.95):>10.2f} {unit}")
    print(f"P99:      {data.quantile(0.99):>10.2f} {unit}")
    print(f"Max:      {data.max():>10.2f} {unit}")
    print(f"Mean:     {data.mean():>10.2f} {unit}")
    print(f"Samples:  {len(data):>10,}")

def main():
    base_dir = Path("../simulations/Mode4")

    # Baseline (DCC)
    baseline_v2v = base_dir / "simulation_logs__27_ECDSA_F_BURST_DCC" / "v2v_logs.csv"
    baseline_sender = base_dir / "simulation_logs__27_ECDSA_F_BURST_DCC" / "sender_summary.csv"

    # CTAC
    ctac_v2v = base_dir / "simulation_logs__26_CTAC_F_BURST" / "v2v_logs.csv"
    ctac_sender = base_dir / "simulation_logs__26_CTAC_F_BURST" / "sender_summary.csv"

    print("="*80)
    print("DETAILED METRICS: IPG AND PDR")
    print("="*80)

    # ========== INTER-PACKET GAP ==========
    print("\n" + "="*80)
    print("INTER-PACKET GAP (IPG)")
    print("="*80)

    print("\nComputing IPG for Baseline (DCC)...")
    baseline_ipg_df = compute_aoi_and_ipg(baseline_v2v, max_distance_m=300)

    print("Computing IPG for CTAC...")
    ctac_ipg_df = compute_aoi_and_ipg(ctac_v2v, max_distance_m=300)

    print("\n" + "-"*80)
    print("BASELINE (DCC) - Inter-Packet Gap Statistics")
    print_stats(baseline_ipg_df['ipg_ms'], "IPG (ms)", "ms")

    print("\n" + "-"*80)
    print("CTAC - Inter-Packet Gap Statistics")
    print_stats(ctac_ipg_df['ipg_ms'], "IPG (ms)", "ms")

    # ========== PDR ==========
    print("\n" + "="*80)
    print("PACKET DELIVERY RATIO (PDR)")
    print("="*80)

    print("\nComputing PDR for Baseline (DCC)...")
    baseline_pdr_df = compute_pdr(baseline_v2v, baseline_sender, max_distance_m=300)

    print("Computing PDR for CTAC...")
    ctac_pdr_df = compute_pdr(ctac_v2v, ctac_sender, max_distance_m=300)

    print("\n" + "-"*80)
    print("BASELINE (DCC) - PDR Statistics (per link)")
    print_stats(baseline_pdr_df['pdr'], "PDR", "")

    print("\n" + "-"*80)
    print("CTAC - PDR Statistics (per link)")
    print_stats(ctac_pdr_df['pdr'], "PDR", "")

    # Overall PDR (mean of per-link PDRs)
    baseline_overall_pdr = baseline_pdr_df['pdr'].mean()
    ctac_overall_pdr = ctac_pdr_df['pdr'].mean()

    print("\n" + "="*80)
    print("OVERALL METRICS")
    print("="*80)
    print(f"\nBaseline (DCC):")
    print(f"  Active links:   {len(baseline_pdr_df):>10,}")
    print(f"  Mean PDR:       {baseline_overall_pdr:>10.4f} ({baseline_overall_pdr*100:.2f}%)")

    print(f"\nCTAC:")
    print(f"  Active links:   {len(ctac_pdr_df):>10,}")
    print(f"  Mean PDR:       {ctac_overall_pdr:>10.4f} ({ctac_overall_pdr*100:.2f}%)")

    print("\n" + "="*80)

if __name__ == "__main__":
    main()
