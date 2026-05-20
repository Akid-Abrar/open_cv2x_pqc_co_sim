"""
V2X Simulation Analysis Script

This script analyzes C-V2X Mode 4 simulation results from openCV2X.
It processes packet reception logs and generates PDR and latency analysis.

Input files:
- simulation_logs/appl_logs.csv: Packet reception data logged by RSU
- simulation_logs/sender_summary.csv: Total packets sent per vehicle

Output:
- plots/: Directory containing all generated figures
"""

import os
import shutil
from pathlib import Path
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt


# =====================================================================
# CONFIGURATION
# =====================================================================
CSV_PATH = "appl_logs.csv"
SUMMARY_PATH = "sender_summary.csv"
OUTPUT_DIR = Path("plots")
IMAGE_EXT = ".png"
BIN_WIDTH = 50  # meters, for binned PDR analysis


# =====================================================================
# UTILITY FUNCTIONS
# =====================================================================

def clear_output_folder(folder: Path) -> None:
    """
    Clear and recreate the output directory.

    Removes all files in the plots directory to ensure fresh output for each run.

    Args:
        folder: Path to the output directory

    Returns:
        None
    """
    if folder.exists():
        shutil.rmtree(folder)
    folder.mkdir(parents=True, exist_ok=True)
    print(f"Cleared output directory: {folder}")


def load_data(csv_path: str, summary_path: str) -> tuple:
    """
    Load and validate simulation data files.

    Reads the packet reception log and sender summary, performs basic validation
    and data type conversions.

    Args:
        csv_path: Path to appl_logs.csv containing packet reception data
        summary_path: Path to sender_summary.csv containing sent packet counts

    Returns:
        tuple: (df_rx, sent_counts) where:
            - df_rx: DataFrame with all received packets
            - sent_counts: dict mapping sender name to total packets sent

    Raises:
        FileNotFoundError: If required files are missing
        ValueError: If required columns are missing
    """
    # Load reception data
    if not Path(csv_path).exists():
        raise FileNotFoundError(f"Reception log not found: {csv_path}")

    df_rx = pd.read_csv(csv_path)

    # Validate required columns
    required_cols = ["t", "sender", "msgId", "dist_m", "delay_ms", "Algorithm"]
    missing_cols = [col for col in required_cols if col not in df_rx.columns]
    if missing_cols:
        raise ValueError(f"Missing required columns: {missing_cols}")

    # Convert to numeric types
    df_rx["t"] = pd.to_numeric(df_rx["t"], errors="coerce")
    df_rx["msgId"] = pd.to_numeric(df_rx["msgId"], errors="coerce")
    df_rx["dist_m"] = pd.to_numeric(df_rx["dist_m"], errors="coerce")
    df_rx["delay_ms"] = pd.to_numeric(df_rx["delay_ms"], errors="coerce")

    # Drop rows with invalid data
    df_rx = df_rx.dropna(subset=required_cols)

    # Filter out "unknown" senders (packets where cert wasn't cached)
    # These can't be matched to sender_summary.csv and would break PDR calculation
    unknown_count = (df_rx["sender"] == "unknown").sum()
    if unknown_count > 0:
        print(f"Filtering out {unknown_count} packets with unknown sender (cert not cached)")
        df_rx = df_rx[df_rx["sender"] != "unknown"]

    # Load sender summary (total sent counts)
    if not Path(summary_path).exists():
        raise FileNotFoundError(f"Sender summary not found: {summary_path}")

    df_summary = pd.read_csv(summary_path)
    df_summary["sender"] = df_summary["sender"].str.strip()
    sent_counts = dict(zip(df_summary["sender"], df_summary["total_sent"]))

    print(f"Loaded {len(df_rx)} received packets from {len(sent_counts)} senders")

    # Validate: check for any data inconsistencies
    per_sender_rx = df_rx.groupby("sender").size().to_dict()
    problems = []
    for sender, received in per_sender_rx.items():
        sent = sent_counts.get(sender, 0)
        if sent == 0:
            problems.append(f"{sender} (received={received}, not in sender_summary)")
        elif received > sent:
            problems.append(f"{sender} (received={received} > sent={sent})")

    if problems:
        print(f"WARNING: Data inconsistencies found:")
        for p in problems[:5]:
            print(f"  - {p}")
        if len(problems) > 5:
            print(f"  ... and {len(problems)-5} more")

    return df_rx, sent_counts


# =====================================================================
# PDR ANALYSIS FUNCTIONS
# =====================================================================

def compute_overall_pdr(df_rx: pd.DataFrame, sent_counts: dict) -> dict:
    """
    Calculate overall Packet Delivery Ratio (PDR) across all vehicles.

    PDR = (Total packets received) / (Total packets sent)

    This is the physical layer delivery success rate, counting ALL received
    packets regardless of verification status.

    Args:
        df_rx: DataFrame containing all received packets
        sent_counts: Dictionary mapping sender to total sent count

    Returns:
        dict with keys:
            - 'total_received': Total number of received packets
            - 'total_sent': Total number of sent packets
            - 'pdr': Overall PDR as a fraction (0 to 1)
            - 'pdr_percent': PDR as percentage
    """
    total_received = len(df_rx)
    total_sent = sum(sent_counts.values())
    pdr = total_received / total_sent if total_sent > 0 else 0

    result = {
        'total_received': total_received,
        'total_sent': total_sent,
        'pdr': pdr,
        'pdr_percent': pdr * 100
    }

    print(f"\n{'='*60}")
    print(f"OVERALL PDR ANALYSIS")
    print(f"{'='*60}")
    print(f"Total packets sent:     {total_sent:,}")
    print(f"Total packets received: {total_received:,}")
    print(f"Overall PDR:            {pdr*100:.2f}%")
    print(f"Packet loss:            {total_sent - total_received:,} ({(1-pdr)*100:.2f}%)")
    print(f"{'='*60}\n")

    return result


def compute_pdr_vs_distance(df_rx: pd.DataFrame, sent_counts: dict) -> pd.DataFrame:
    """
    Calculate PDR as a function of average distance from RSU.

    Since vehicles are moving, we group packets by sender and calculate:
    - Average distance: mean distance of all packets received from that sender
    - PDR: (packets received from sender) / (total sent by sender)

    This answers: "What was the PDR for vehicles that operated at distance X?"

    Args:
        df_rx: DataFrame containing all received packets
        sent_counts: Dictionary mapping sender to total sent count

    Returns:
        DataFrame with columns: sender, avg_dist_m, received, sent, pdr
        Sorted by average distance
    """
    results = []

    for sender, group in df_rx.groupby("sender"):
        avg_dist = group["dist_m"].mean()
        received = len(group)
        sent = sent_counts.get(sender, 0)
        pdr = received / sent if sent > 0 else 0

        results.append({
            'sender': sender,
            'avg_dist_m': avg_dist,
            'received': received,
            'sent': sent,
            'pdr': pdr
        })

    df_pdr_dist = pd.DataFrame(results).sort_values('avg_dist_m')

    print(f"Computed PDR vs distance for {len(df_pdr_dist)} senders")

    return df_pdr_dist


def compute_binned_pdr(df_rx: pd.DataFrame, sent_counts: dict, bin_width: int = 50) -> pd.DataFrame:
    """
    Calculate PDR in distance bins to show performance at different ranges.

    Groups senders by their average distance into bins (e.g., 0-50m, 50-100m, etc.)
    and calculates weighted PDR for each bin.

    Weighted PDR per bin = (total received in bin) / (total sent in bin)

    This answers: "What is the PDR for vehicles in the range X to X+50 meters?"

    Args:
        df_rx: DataFrame containing all received packets
        sent_counts: Dictionary mapping sender to total sent count
        bin_width: Width of distance bins in meters (default: 50)

    Returns:
        DataFrame with columns: bin_center, num_senders, total_sent,
                                total_received, pdr
        Sorted by distance
    """
    # First, get PDR vs distance per sender
    df_pdr_dist = compute_pdr_vs_distance(df_rx, sent_counts)

    # Create distance bins
    max_dist = df_pdr_dist['avg_dist_m'].max()
    bins = list(range(0, int(max_dist) + bin_width + 1, bin_width))
    df_pdr_dist['dist_bin'] = pd.cut(df_pdr_dist['avg_dist_m'], bins=bins, right=False)

    # Aggregate by bin
    binned = df_pdr_dist.groupby('dist_bin', observed=True).agg(
        num_senders=('sender', 'count'),
        total_sent=('sent', 'sum'),
        total_received=('received', 'sum'),
    ).reset_index()

    # Calculate weighted PDR per bin
    binned['pdr'] = binned['total_received'] / binned['total_sent']
    binned['bin_center'] = binned['dist_bin'].apply(lambda x: x.mid)

    # Sort by distance
    binned = binned.sort_values('bin_center')

    print(f"\nBinned PDR (bin width = {bin_width}m):")
    print(f"{'Distance (m)':<15} {'Senders':<10} {'PDR (%)':<10}")
    print("-" * 35)
    for _, row in binned.iterrows():
        print(f"{row['bin_center']:<15.0f} {row['num_senders']:<10} {row['pdr']*100:<10.2f}")
    print()

    return binned


# =====================================================================
# LATENCY ANALYSIS FUNCTIONS
# =====================================================================

def compute_latency_cdf(df_rx: pd.DataFrame) -> tuple:
    """
    Compute Cumulative Distribution Function (CDF) for end-to-end latency.

    The CDF shows what percentage of packets had latency <= X ms.
    Latency is measured from packet creation (at sender) to reception (at RSU).

    Args:
        df_rx: DataFrame containing all received packets with 'delay_ms' column

    Returns:
        tuple: (latency_values, cdf_values) where:
            - latency_values: sorted array of latency values in ms
            - cdf_values: corresponding CDF values (0 to 1)
    """
    latencies = df_rx['delay_ms'].dropna().sort_values().values
    cdf = np.arange(1, len(latencies) + 1) / len(latencies)

    # Print statistics
    print(f"\n{'='*60}")
    print(f"LATENCY STATISTICS")
    print(f"{'='*60}")
    print(f"Mean latency:   {latencies.mean():.2f} ms")
    print(f"Median latency: {np.median(latencies):.2f} ms")
    print(f"95th percentile: {np.percentile(latencies, 95):.2f} ms")
    print(f"99th percentile: {np.percentile(latencies, 99):.2f} ms")
    print(f"Max latency:    {latencies.max():.2f} ms")
    print(f"{'='*60}\n")

    return latencies, cdf


# =====================================================================
# PLOTTING FUNCTIONS
# =====================================================================

def plot_pdr_vs_distance(df_pdr_dist: pd.DataFrame, overall_pdr: dict,
                         algo: str, output_dir: Path) -> None:
    """
    Plot scatter plot of PDR vs average distance for each sender.

    Each point represents one vehicle. X-axis is the average distance at which
    that vehicle's packets were received, Y-axis is its PDR.

    Args:
        df_pdr_dist: DataFrame from compute_pdr_vs_distance()
        overall_pdr: Dict from compute_overall_pdr() for title annotation
        algo: Algorithm name for plot title
        output_dir: Directory to save the plot

    Returns:
        None (saves plot to file)
    """
    fig, ax = plt.subplots(figsize=(10, 6))

    # Scatter plot
    ax.scatter(df_pdr_dist['avg_dist_m'], df_pdr_dist['pdr'] * 100,
               alpha=0.6, s=50, color='steelblue')

    # 90% PDR reference line
    ax.axhline(90, color='red', linestyle='--', linewidth=2, label='PDR = 90%')

    ax.set_xlabel('Average Distance from RSU (m)', fontsize=12)
    ax.set_ylabel('PDR (%)', fontsize=12)
    ax.set_title(
        f'PDR vs Average Distance\n'
        f'Algorithm: {algo}, Overall PDR: {overall_pdr["pdr_percent"]:.2f}%',
        fontsize=13, fontweight='bold'
    )
    ax.set_ylim(0, 105)
    ax.grid(True, alpha=0.3)
    ax.legend()
    plt.tight_layout()

    fname = output_dir / f"PDR_vs_Distance{IMAGE_EXT}"
    plt.savefig(fname, dpi=300)
    plt.close(fig)
    print(f"Saved: {fname}")


def plot_binned_pdr(binned: pd.DataFrame, overall_pdr: dict,
                    algo: str, bin_width: int, output_dir: Path) -> None:
    """
    Plot bar chart of PDR in distance bins.

    Shows PDR for vehicles grouped by distance ranges (e.g., 0-50m, 50-100m, etc.).
    Bar height = PDR, labels show number of senders in each bin.
    Bins are labeled with ranges and plotted at their center points.

    Args:
        binned: DataFrame from compute_binned_pdr()
        overall_pdr: Dict from compute_overall_pdr() for title annotation
        algo: Algorithm name for plot title
        bin_width: Width of bins in meters
        output_dir: Directory to save the plot

    Returns:
        None (saves plot to file)
    """
    fig, ax = plt.subplots(figsize=(12, 6))

    # Bar chart - plot at bin centers
    x_positions = binned['bin_center'].values
    bars = ax.bar(x_positions, binned['pdr'] * 100,
                  width=bin_width * 0.8, edgecolor='black',
                  alpha=0.7, color='steelblue')

    # Add sender count labels on bars
    for bar, n in zip(bars, binned['num_senders']):
        ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 2,
                f'n={n}', ha='center', va='bottom', fontsize=9)

    # Create bin range labels (e.g., "0-50m", "50-100m")
    bin_labels = []
    for center in binned['bin_center']:
        lower = int(center - bin_width/2)
        upper = int(center + bin_width/2)
        bin_labels.append(f"{lower}-{upper}m")

    # Set x-axis ticks at bin centers with range labels
    ax.set_xticks(x_positions)
    ax.set_xticklabels(bin_labels, rotation=45, ha='right', fontsize=9)

    # 90% PDR reference line
    ax.axhline(90, color='red', linestyle='--', linewidth=2, label='PDR = 90%')

    ax.set_xlabel('Distance Range from RSU', fontsize=12)
    ax.set_ylabel('PDR (%)', fontsize=12)
    ax.set_title(
        f'Binned PDR vs Distance ({bin_width}m bins)\n'
        f'Algorithm: {algo}, Overall PDR: {overall_pdr["pdr_percent"]:.2f}%',
        fontsize=13, fontweight='bold'
    )
    ax.set_ylim(0, 105)
    ax.grid(True, axis='y', alpha=0.3)
    ax.legend()
    plt.tight_layout()

    fname = output_dir / f"PDR_vs_Distance_Binned_{bin_width}m{IMAGE_EXT}"
    plt.savefig(fname, dpi=300)
    plt.close(fig)
    print(f"Saved: {fname}")


def plot_latency_cdf(latencies: np.ndarray, cdf: np.ndarray,
                     algo: str, output_dir: Path) -> None:
    """
    Plot Cumulative Distribution Function (CDF) of end-to-end latency.

    Shows what percentage of packets had latency below a given threshold.
    Useful for understanding latency distribution and tail behavior.

    Args:
        latencies: Array of latency values in ms (sorted)
        cdf: Array of CDF values (0 to 1)
        algo: Algorithm name for plot title
        output_dir: Directory to save the plot

    Returns:
        None (saves plot to file)
    """
    fig, ax = plt.subplots(figsize=(10, 6))

    # CDF plot
    ax.plot(latencies, cdf * 100, linewidth=2, color='steelblue')

    # Reference lines for key percentiles
    ax.axhline(95, color='orange', linestyle='--', linewidth=1.5,
               alpha=0.7, label='95th percentile')
    ax.axhline(99, color='red', linestyle='--', linewidth=1.5,
               alpha=0.7, label='99th percentile')

    ax.set_xlabel('End-to-End Latency (ms)', fontsize=12)
    ax.set_ylabel('CDF (%)', fontsize=12)
    ax.set_title(
        f'Latency CDF\n'
        f'Algorithm: {algo}',
        fontsize=13, fontweight='bold'
    )
    ax.set_ylim(0, 105)
    ax.grid(True, alpha=0.3)
    ax.legend()
    plt.tight_layout()

    fname = output_dir / f"Latency_CDF{IMAGE_EXT}"
    plt.savefig(fname, dpi=300)
    plt.close(fig)
    print(f"Saved: {fname}")


# =====================================================================
# MAIN ANALYSIS PIPELINE
# =====================================================================

def main():
    """
    Main analysis pipeline.

    Executes the complete analysis workflow:
    1. Load data
    2. Compute PDR metrics
    3. Compute latency CDF
    4. Generate all plots

    Returns:
        None
    """
    print("\n" + "="*60)
    print("V2X SIMULATION ANALYSIS")
    print("="*60 + "\n")

    # Clear output directory
    clear_output_folder(OUTPUT_DIR)

    # Load data
    try:
        df_rx, sent_counts = load_data(CSV_PATH, SUMMARY_PATH)
    except (FileNotFoundError, ValueError) as e:
        print(f"ERROR: {e}")
        return

    # Extract metadata
    algo = df_rx['Algorithm'].iloc[0] if 'Algorithm' in df_rx.columns else 'Unknown'

    # ===== PDR ANALYSIS =====

    # 1. Overall PDR
    overall_pdr = compute_overall_pdr(df_rx, sent_counts)

    # 2. PDR vs distance (per sender)
    df_pdr_dist = compute_pdr_vs_distance(df_rx, sent_counts)

    # 3. Binned PDR
    binned = compute_binned_pdr(df_rx, sent_counts, bin_width=BIN_WIDTH)

    # ===== LATENCY ANALYSIS =====

    # 4. Latency CDF
    latencies, cdf = compute_latency_cdf(df_rx)

    # ===== GENERATE PLOTS =====

    print(f"\n{'='*60}")
    print(f"GENERATING PLOTS")
    print(f"{'='*60}\n")

    plot_pdr_vs_distance(df_pdr_dist, overall_pdr, algo, OUTPUT_DIR)
    plot_binned_pdr(binned, overall_pdr, algo, BIN_WIDTH, OUTPUT_DIR)
    plot_latency_cdf(latencies, cdf, algo, OUTPUT_DIR)

    print(f"\n{'='*60}")
    print(f"ANALYSIS COMPLETE")
    print(f"{'='*60}")
    print(f"All plots saved to: {OUTPUT_DIR}/")
    print(f"{'='*60}\n")


if __name__ == "__main__":
    main()
