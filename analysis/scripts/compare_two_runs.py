#!/usr/bin/env python3
"""
Compare two simulation runs (baseline vs CTAC)
Usage: python3 compare_two_runs.py <baseline_folder> <ctac_folder>
"""

import pandas as pd
import numpy as np
import sys
from pathlib import Path

def compute_aoi_and_ipg(csv_path, max_distance_m=300):
    """Compute AoI and IPG from v2v logs."""
    df = pd.read_csv(csv_path)
    df['gen_time'] = df['t'] - (df['delay_ms'] / 1000.0)

    aoi_samples = []
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
                peak_aoi_ms = (curr['t'] - prev['gen_time']) * 1000.0
                ipg_ms = (curr['t'] - prev['t']) * 1000.0

                aoi_samples.append(peak_aoi_ms)
                ipg_samples.append(ipg_ms)

    return pd.Series(aoi_samples), pd.Series(ipg_samples)

def extract_cbr_from_sca(sca_path):
    """Extract CBR mean values from .sca file."""
    cbr_values = []

    with open(sca_path, 'r') as f:
        for line in f:
            if 'cbr:mean' in line and 'scalar' in line:
                parts = line.strip().split()
                if len(parts) >= 4:
                    value = float(parts[3])
                    cbr_values.append(value * 100)  # Convert to percentage

    return pd.Series(cbr_values)

def extract_interference_stats(sca_path):
    """Extract interference statistics from .sca file."""
    stats = {
        'tbSent': 0,
        'sciSent': 0,
        'tbFailedDueToInterference': 0,
        'sciFailedDueToInterference': 0
    }

    with open(sca_path, 'r') as f:
        for line in f:
            if 'tbSent:sum' in line:
                parts = line.strip().split()
                if len(parts) >= 4:
                    stats['tbSent'] += int(parts[3])
            elif 'sciSent:sum' in line:
                parts = line.strip().split()
                if len(parts) >= 4:
                    stats['sciSent'] += int(parts[3])
            elif 'tbFailedDueToInterference:sum' in line and 'IgnoreSCI' not in line:
                parts = line.strip().split()
                if len(parts) >= 4:
                    stats['tbFailedDueToInterference'] += int(parts[3])
            elif 'sciFailedDueToInterference:sum' in line:
                parts = line.strip().split()
                if len(parts) >= 4:
                    stats['sciFailedDueToInterference'] += int(parts[3])

    return stats

def compute_pdr(v2v_log_path, sender_summary_path, sca_path):
    """Compute per-link PDR correctly using application-layer BSM counts.

    For current results: actual_bsm_sent = sender_summary - ctacDeferred
    For future results (after code fix): sender_summary will be correct directly
    """
    # Get opportunities from sender_summary.csv
    sender_df = pd.read_csv(sender_summary_path)
    opportunities = {}
    for _, row in sender_df.iterrows():
        opportunities[row['sender']] = row['total_sent']

    # Get ctacDeferred count from .sca file (for CTAC scenarios)
    deferred = {}
    with open(sca_path, 'r') as f:
        for line in f:
            if 'ctacDeferred:sum' in line:
                parts = line.strip().split()
                if len(parts) >= 4:
                    module = parts[1]
                    if 'carNoIp[' in module:
                        vehicle = module.split('.')[1]
                        deferred[vehicle] = int(parts[3])

    # Calculate actual BSM count: opportunities - deferred
    vehicle_tx = {}
    for vehicle, opp in opportunities.items():
        defer_count = deferred.get(vehicle, 0)
        vehicle_tx[vehicle] = opp - defer_count

    # Get PHY transmissions for app-to-PHY drop analysis
    vehicle_phy_tx = {}
    with open(sca_path, 'r') as f:
        for line in f:
            if 'tbSent:sum' in line:
                parts = line.strip().split()
                if len(parts) >= 4:
                    module = parts[1]
                    if 'carNoIp[' in module:
                        vehicle = module.split('.')[1]
                        vehicle_phy_tx[vehicle] = int(parts[3])

    # Count receptions per (sender, receiver) link
    v2v_df = pd.read_csv(v2v_log_path)
    v2v_df = v2v_df[v2v_df['dist_m'] < 300]  # Only within 300m

    link_rx = v2v_df.groupby(['sender', 'receiver']).size().reset_index(name='received')

    # Calculate PDR per link
    pdr_values = []
    for _, row in link_rx.iterrows():
        sender = row['sender']
        received = row['received']

        if sender in vehicle_tx:
            sent = vehicle_tx[sender]
            if sent > 0:
                pdr = received / sent
                pdr_values.append(pdr)

    pdr_series = pd.Series(pdr_values)

    # Calculate app-to-PHY drops
    total_app_tx = sum(vehicle_tx.values())
    total_phy_tx = sum(vehicle_phy_tx.values())
    app_to_phy_drops = total_app_tx - total_phy_tx
    app_to_phy_drop_rate = (app_to_phy_drops / total_app_tx * 100) if total_app_tx > 0 else 0

    return {
        'total_app_tx': total_app_tx,
        'total_phy_tx': total_phy_tx,
        'app_to_phy_drops': app_to_phy_drops,
        'app_to_phy_drop_rate': app_to_phy_drop_rate,
        'total_rx': len(v2v_df),
        'num_links': len(pdr_values),
        'pdr_min': pdr_series.min() if len(pdr_series) > 0 else 0,
        'pdr_median': pdr_series.median() if len(pdr_series) > 0 else 0,
        'pdr_mean': pdr_series.mean() if len(pdr_series) > 0 else 0,
        'pdr_max': pdr_series.max() if len(pdr_series) > 0 else 0,
    }

def print_comparison(baseline_dir, ctac_dir, output_file=None):
    """Print comparison statistics to console and optionally to file."""
    baseline_dir = Path(baseline_dir)
    ctac_dir = Path(ctac_dir)

    # Set up output
    if output_file:
        import sys
        f_out = open(output_file, 'w')
        original_stdout = sys.stdout
        sys.stdout = f_out

    # Find .sca files
    baseline_sca = list(baseline_dir.glob('*.sca'))[0]
    ctac_sca = list(ctac_dir.glob('*.sca'))[0]

    baseline_v2v = baseline_dir / 'v2v_logs.csv'
    ctac_v2v = ctac_dir / 'v2v_logs.csv'

    print("="*80)
    print("SIMULATION COMPARISON")
    print("="*80)
    print(f"\nBaseline: {baseline_dir.name}")
    print(f"CTAC:     {ctac_dir.name}")
    print()

    # CBR Analysis
    print("="*80)
    print("CHANNEL BUSY RATIO (CBR)")
    print("="*80)

    baseline_cbr = extract_cbr_from_sca(baseline_sca)
    ctac_cbr = extract_cbr_from_sca(ctac_sca)

    print(f"\n{'Metric':<20} {'Baseline':<15} {'CTAC':<15} {'Change':<15}")
    print("-"*80)
    print(f"{'Min':<20} {baseline_cbr.min():>10.2f}%    {ctac_cbr.min():>10.2f}%    {((ctac_cbr.min()-baseline_cbr.min())/baseline_cbr.min()*100):>+6.1f}%")
    print(f"{'Median':<20} {baseline_cbr.median():>10.2f}%    {ctac_cbr.median():>10.2f}%    {((ctac_cbr.median()-baseline_cbr.median())/baseline_cbr.median()*100):>+6.1f}%")
    print(f"{'Mean':<20} {baseline_cbr.mean():>10.2f}%    {ctac_cbr.mean():>10.2f}%    {((ctac_cbr.mean()-baseline_cbr.mean())/baseline_cbr.mean()*100):>+6.1f}%")
    print(f"{'Max':<20} {baseline_cbr.max():>10.2f}%    {ctac_cbr.max():>10.2f}%    {((ctac_cbr.max()-baseline_cbr.max())/baseline_cbr.max()*100):>+6.1f}%")

    # AoI Analysis
    print("\n" + "="*80)
    print("AGE OF INFORMATION (AoI)")
    print("="*80)

    print("\nComputing AoI from v2v logs...")
    baseline_aoi, baseline_ipg = compute_aoi_and_ipg(baseline_v2v)
    ctac_aoi, ctac_ipg = compute_aoi_and_ipg(ctac_v2v)

    print(f"\n{'Metric':<20} {'Baseline':<15} {'CTAC':<15} {'Change':<15}")
    print("-"*80)
    print(f"{'Min':<20} {baseline_aoi.min():>10.1f} ms   {ctac_aoi.min():>10.1f} ms   {((ctac_aoi.min()-baseline_aoi.min())/baseline_aoi.min()*100):>+6.1f}%")
    print(f"{'Median':<20} {baseline_aoi.median():>10.1f} ms   {ctac_aoi.median():>10.1f} ms   {((ctac_aoi.median()-baseline_aoi.median())/baseline_aoi.median()*100):>+6.1f}%")
    print(f"{'P95':<20} {baseline_aoi.quantile(0.95):>10.1f} ms   {ctac_aoi.quantile(0.95):>10.1f} ms   {((ctac_aoi.quantile(0.95)-baseline_aoi.quantile(0.95))/baseline_aoi.quantile(0.95)*100):>+6.1f}%")
    print(f"{'Mean':<20} {baseline_aoi.mean():>10.1f} ms   {ctac_aoi.mean():>10.1f} ms   {((ctac_aoi.mean()-baseline_aoi.mean())/baseline_aoi.mean()*100):>+6.1f}%")
    print(f"{'Max':<20} {baseline_aoi.max():>10.1f} ms   {ctac_aoi.max():>10.1f} ms   {((ctac_aoi.max()-baseline_aoi.max())/baseline_aoi.max()*100):>+6.1f}%")

    # IPG Analysis
    print("\n" + "="*80)
    print("INTER-PACKET GAP (IPG)")
    print("="*80)

    print(f"\n{'Metric':<20} {'Baseline':<15} {'CTAC':<15} {'Change':<15}")
    print("-"*80)
    print(f"{'Median':<20} {baseline_ipg.median():>10.1f} ms   {ctac_ipg.median():>10.1f} ms   {((ctac_ipg.median()-baseline_ipg.median())/baseline_ipg.median()*100):>+6.1f}%")
    print(f"{'P95':<20} {baseline_ipg.quantile(0.95):>10.1f} ms   {ctac_ipg.quantile(0.95):>10.1f} ms   {((ctac_ipg.quantile(0.95)-baseline_ipg.quantile(0.95))/baseline_ipg.quantile(0.95)*100):>+6.1f}%")
    print(f"{'Mean':<20} {baseline_ipg.mean():>10.1f} ms   {ctac_ipg.mean():>10.1f} ms   {((ctac_ipg.mean()-baseline_ipg.mean())/baseline_ipg.mean()*100):>+6.1f}%")

    # PDR Analysis
    print("\n" + "="*80)
    print("PACKET DELIVERY RATIO (PDR) - Per Link")
    print("="*80)

    baseline_sender = baseline_dir / 'sender_summary.csv'
    ctac_sender = ctac_dir / 'sender_summary.csv'

    print("\nComputing per-link PDR...")
    baseline_pdr = compute_pdr(baseline_v2v, baseline_sender, baseline_sca)
    ctac_pdr = compute_pdr(ctac_v2v, ctac_sender, ctac_sca)

    print(f"\n{'Metric':<30} {'Baseline':<20} {'CTAC':<20} {'Change':<15}")
    print("-"*80)
    print(f"{'Active links':<30} {baseline_pdr['num_links']:>15,}    {ctac_pdr['num_links']:>15,}    {((ctac_pdr['num_links']-baseline_pdr['num_links'])/baseline_pdr['num_links']*100):>+6.1f}%")
    print(f"{'PDR Min':<30} {baseline_pdr['pdr_min']:>15.4f}    {ctac_pdr['pdr_min']:>15.4f}    {((ctac_pdr['pdr_min']-baseline_pdr['pdr_min'])/baseline_pdr['pdr_min']*100 if baseline_pdr['pdr_min']>0 else 0):>+6.1f}%")
    print(f"{'PDR Median':<30} {baseline_pdr['pdr_median']:>15.4f}    {ctac_pdr['pdr_median']:>15.4f}    {((ctac_pdr['pdr_median']-baseline_pdr['pdr_median'])/baseline_pdr['pdr_median']*100):>+6.1f}%")
    print(f"{'PDR Mean':<30} {baseline_pdr['pdr_mean']:>15.4f}    {ctac_pdr['pdr_mean']:>15.4f}    {((ctac_pdr['pdr_mean']-baseline_pdr['pdr_mean'])/baseline_pdr['pdr_mean']*100):>+6.1f}%")
    print(f"{'PDR Max':<30} {baseline_pdr['pdr_max']:>15.4f}    {ctac_pdr['pdr_max']:>15.4f}    {((ctac_pdr['pdr_max']-baseline_pdr['pdr_max'])/baseline_pdr['pdr_max']*100):>+6.1f}%")

    print(f"\nNote: PDR per link = (packets received from sender) / (packets sent by sender)")
    print(f"      Calculated for each (sender, receiver) pair within 300m")

    # Interference Stats
    print("\n" + "="*80)
    print("INTERFERENCE STATISTICS")
    print("="*80)

    baseline_int = extract_interference_stats(baseline_sca)
    ctac_int = extract_interference_stats(ctac_sca)

    baseline_sci_rate = (baseline_int['sciFailedDueToInterference'] / baseline_int['sciSent'] * 100) if baseline_int['sciSent'] > 0 else 0
    ctac_sci_rate = (ctac_int['sciFailedDueToInterference'] / ctac_int['sciSent'] * 100) if ctac_int['sciSent'] > 0 else 0

    print(f"\n{'Metric':<30} {'Baseline':<15} {'CTAC':<15}")
    print("-"*80)
    print(f"{'Total transmissions':<30} {baseline_int['sciSent']:>10,}     {ctac_int['sciSent']:>10,}")
    print(f"{'SCI failures':<30} {baseline_int['sciFailedDueToInterference']:>10,}     {ctac_int['sciFailedDueToInterference']:>10,}")
    print(f"{'SCI failure rate':<30} {baseline_sci_rate:>10.2f}%    {ctac_sci_rate:>10.2f}%")

    # Transmission Statistics
    print("\n" + "="*80)
    print("TRANSMISSION STATISTICS")
    print("="*80)

    print(f"\n{'Metric':<30} {'Baseline':<20} {'CTAC':<20} {'Ratio':<15}")
    print("-"*80)
    print(f"{'App BSMs Generated':<30} {baseline_pdr['total_app_tx']:>15,}    {ctac_pdr['total_app_tx']:>15,}    {ctac_pdr['total_app_tx']/baseline_pdr['total_app_tx']:>10.2f}x")
    print(f"{'PHY Transmissions':<30} {baseline_pdr['total_phy_tx']:>15,}    {ctac_pdr['total_phy_tx']:>15,}    {ctac_pdr['total_phy_tx']/baseline_pdr['total_phy_tx']:>10.2f}x")
    print(f"{'App→PHY Drops':<30} {baseline_pdr['app_to_phy_drops']:>15,}    {ctac_pdr['app_to_phy_drops']:>15,}    {'N/A':<15}")
    print(f"{'App→PHY Drop Rate':<30} {baseline_pdr['app_to_phy_drop_rate']:>14.2f}%    {ctac_pdr['app_to_phy_drop_rate']:>14.2f}%    {'N/A':<15}")
    print(f"{'V2V Receptions (<300m)':<30} {baseline_pdr['total_rx']:>15,}    {ctac_pdr['total_rx']:>15,}    {ctac_pdr['total_rx']/baseline_pdr['total_rx']:>10.2f}x")
    print(f"{'Active Links':<30} {baseline_pdr['num_links']:>15,}    {ctac_pdr['num_links']:>15,}    {ctac_pdr['num_links']/baseline_pdr['num_links']:>10.2f}x")
    print(f"{'AoI Samples':<30} {len(baseline_aoi):>15,}    {len(ctac_aoi):>15,}    {len(ctac_aoi)/len(baseline_aoi):>10.2f}x")

    # Key Findings
    print("\n" + "="*80)
    print("KEY FINDINGS")
    print("="*80)

    cbr_reduction = (baseline_cbr.median() - ctac_cbr.median()) / baseline_cbr.median() * 100
    aoi_change = (ctac_aoi.median() - baseline_aoi.median()) / baseline_aoi.median() * 100
    tx_reduction = (1 - ctac_pdr['total_phy_tx']/baseline_pdr['total_phy_tx']) * 100
    pdr_change = (ctac_pdr['pdr_median'] - baseline_pdr['pdr_median']) / baseline_pdr['pdr_median'] * 100 if baseline_pdr['pdr_median'] > 0 else 0

    print(f"\n✓ CBR Reduction (median):     {cbr_reduction:>6.1f}%")
    print(f"✓ TX Reduction:               {tx_reduction:>6.1f}%")
    print(f"✓ PDR Change (median):        {pdr_change:>+6.1f}%")
    print(f"✓ AoI Change (median):        {aoi_change:>+6.1f}%")
    print(f"")
    print(f"  Baseline CBR:               {baseline_cbr.median():>6.2f}%")
    print(f"  CTAC CBR:                   {ctac_cbr.median():>6.2f}%")
    print(f"")
    print(f"  Baseline Transmissions:     {baseline_pdr['total_phy_tx']:>10,}")
    print(f"  CTAC Transmissions:         {ctac_pdr['total_phy_tx']:>10,}")
    print(f"")
    print(f"  Baseline App→PHY drops:     {baseline_pdr['app_to_phy_drops']:>10,} ({baseline_pdr['app_to_phy_drop_rate']:.1f}%)")
    print(f"  CTAC App→PHY drops:         {ctac_pdr['app_to_phy_drops']:>10,} ({ctac_pdr['app_to_phy_drop_rate']:.1f}%)")
    print(f"")
    print(f"  Baseline PDR (median):      {baseline_pdr['pdr_median']:>10.4f}")
    print(f"  CTAC PDR (median):          {ctac_pdr['pdr_median']:>10.4f}")

    if baseline_cbr.median() > 30:
        print(f"\n✓ DCC Active (baseline CBR > 30%)")
    else:
        print(f"\n⚠ DCC Not Active (baseline CBR < 30%)")

    print("\n" + "="*80)

    # Close output file if used
    if output_file:
        sys.stdout = original_stdout
        f_out.close()
        print(f"\nResults saved to: {output_file}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 compare_two_runs.py <baseline_folder> <ctac_folder> [output_file]")
        print("\nExample:")
        print("  python3 compare_two_runs.py ../simulations/Mode4/res_base_1 ../simulations/Mode4/res_ctac_1")
        print("  python3 compare_two_runs.py ../simulations/Mode4/res_base_1 ../simulations/Mode4/res_ctac_1 comparison_batch1.txt")
        sys.exit(1)

    baseline_folder = sys.argv[1]
    ctac_folder = sys.argv[2]
    output_file = sys.argv[3] if len(sys.argv) > 3 else None

    print_comparison(baseline_folder, ctac_folder, output_file)
