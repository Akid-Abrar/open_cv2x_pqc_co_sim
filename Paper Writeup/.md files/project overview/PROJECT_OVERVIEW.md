# Post-Quantum Cryptography on C-V2X Mode 4 Sidelink - Analysis Project

**Last Updated:** May 4, 2026
**Project Type:** IEEE Conference Paper + Journal Paper (Dual Track)

---

## Project Summary

This project evaluates **Falcon-512** (NIST-standardized post-quantum cryptographic algorithm) performance on **C-V2X Mode 4 sidelink communications**, comparing it against baseline **ECDSA P-256**. The research demonstrates that while PQC is computationally feasible, spectrum efficiency (470% packet size overhead) is the primary deployment bottleneck.

**Key Finding:** At 30 veh/km traffic density, Falcon-512 achieves only 18.48% PDR vs. 31.32% for ECDSA—a 41% relative degradation due to 4× subchannel consumption.

---

## Simulation Environment

- **Simulator:** openCV2X (modified from SimuLTE base)
- **Radio Parameters:** 20 MHz bandwidth (100 RBs), 10-RB subchannels, MCS 5-11
- **Crypto Libraries:** liboqs (Falcon-512), OpenSSL (ECDSA P-256)
- **Traffic Standard:** SAE J3161 (July 2024)
- **Message Type:** BSMs (Basic Safety Messages) at 10 Hz
- **Propagation:** Two-ray ground reflection (LOS) or NLOS channel model

---

## Folder Structure

```
Paper Writeup/
├── IEEE_Conference_Template/       # Conference paper (current focus)
│   ├── conference_101719.tex       # Main LaTeX source
│   ├── references.bib              # Bibliography
│   └── figures/                    # Generated figures (PDF + PNG)
│       └── performance_comparison.pdf
│
├── Template/                       # Journal paper (future work)
│   ├── main.tex
│   └── main.bib
│
├── Simulation Results/
│   └── Simulation Logs Storage-selected/  # Selected scenario results
│       ├── _5_ECDSA_C_LOS/               # Scenario 5: ECDSA, 30 veh/km, LOS
│       ├── simulation_logs__17_Falcon_C_LOS/  # Scenario 17: Falcon, 30 veh/km, LOS
│       ├── _3_ECDSA_B_LOS_20260407_202334/    # Scenario 3: ECDSA, 20 veh/km, LOS
│       ├── _15_Falcon_B_LOS_20260409_164915/  # Scenario 15: Falcon, 20 veh/km, LOS
│       └── [other scenarios...]
│
├── scripts/                        # Analysis & figure generation
│   ├── analyze_scenario_B.py       # Statistical analysis (currently configured for Scenario C)
│   ├── conf_generate_figures.py    # Conference paper figure generation
│   └── jour_generate_figures.py    # Journal paper figure generation
│
├── Notes and Reports/              # Research notes, reviews
├── analysis_notes/                 # Generated analysis outputs
└── PROJECT_OVERVIEW.md             # This file
```

---

## Simulation Scenario Naming Convention

Format: `_{NUMBER}_{ALGORITHM}_{SERVICE_LEVEL}_{CHANNEL}`

**Examples:**
- `_5_ECDSA_C_LOS` → Scenario 5, ECDSA P-256, Service Level C, Line-of-Sight
- `_17_Falcon_C_LOS` → Scenario 17, Falcon-512, Service Level C, Line-of-Sight

**Service Levels (Traffic Density):**
- **A:** Low density (~15 veh/km)
- **B:** Moderate-low density (20 veh/km, 1400 veh/h per approach)
- **C:** Moderate-high density (30 veh/km, 2172 veh/h per approach) ← **Currently used**
- **D:** High density (~40 veh/km)
- **E:** Very high density (~50 veh/km)
- **F:** Extreme density (~60 veh/km)

**Channel Models:**
- **LOS:** Line-of-sight (ideal propagation)
- **NLOS:** Non-line-of-sight (urban obstruction)

**Scenario Mapping:**
- Scenarios 1-12: ECDSA P-256
- Scenarios 13-24: Falcon-512
- Each service level has both LOS and NLOS variants

---

## Current Conference Paper Status

**Paper:** `IEEE_Conference_Template/conference_101719.tex`

**Scenarios Analyzed:**
- **Scenario 5** (ECDSA_C_LOS) vs **Scenario 17** (Falcon_C_LOS)
- Traffic density: 30 veh/km (Service Level C)
- Channel: LOS (Line-of-Sight)
- Analysis period: **200 seconds**

**Key Metrics Reported:**
| Metric | ECDSA P-256 | Falcon-512 | Difference |
|--------|-------------|------------|------------|
| PDR (%) | 31.32 | 18.48 | -12.84% |
| Mean Latency (ms) | 51.92 | 51.82 | -0.10 ms |
| P95 Latency (ms) | 96 | 97 | +1 ms |
| Packet Size with Cert (bytes) | 305 | 1,739 | +470% |
| Packet Size Digest Only (bytes) | 143 | 745 | +421% |
| Verification Success (%) | 100 | 100 | 0% |

**Figure Generated:** 4-panel comparison (reception rate vs distance, latency, packet sizes)

---

## Python Analysis Scripts

### 1. `scripts/analyze_scenario_B.py`
**Purpose:** Statistical comparison between ECDSA and Falcon for a given scenario.

**Current Configuration:**
```python
ECDSA_DIR = "Simulation Results/Simulation Logs Storage-selected/simulation_logs__5_ECDSA_C_LOS/logs"
FALCON_DIR = "Simulation Results/Simulation Logs Storage-selected/simulation_logs__17_Falcon_C_LOS/logs"
TIME_LIMIT = 200.0  # seconds
```

**Output:** Console statistics (file output disabled per user request)

**Metrics Computed:**
- Packet Delivery Ratio (PDR)
- Latency statistics (mean, median, std, P95, P99, max)
- Verification success rate
- Packet sizes (with certificate vs digest-only)
- Distance-based PDR bins

**To Run:**
```bash
cd "Paper Writeup"
python3 scripts/analyze_scenario_B.py
```

---

### 2. `scripts/conf_generate_figures.py`
**Purpose:** Generate 4-panel comparison figure for IEEE conference paper.

**Current Configuration:**
```python
ECDSA_DIR = "Simulation Results/Simulation Logs Storage-selected/simulation_logs__5_ECDSA_C_LOS/logs"
FALCON_DIR = "Simulation Results/Simulation Logs Storage-selected/simulation_logs__17_Falcon_C_LOS/logs"
TIME_LIMIT = 200.0
OUTPUT_DIR = "IEEE_Conference_Template/figures"
```

**Output Files:**
- `IEEE_Conference_Template/figures/performance_comparison.pdf`
- `IEEE_Conference_Template/figures/performance_comparison.png`

**Figure Panels:**
- (a) Reception Rate vs Distance
- (b) Mean Latency Comparison (bar chart)
- (c) Packet Size with Certificate (bar chart)
- (d) Packet Size Digest Only (bar chart)

**To Run:**
```bash
cd "Paper Writeup"
python3 scripts/conf_generate_figures.py
```

---

### 3. `scripts/jour_generate_figures.py`
**Purpose:** Generate figures for journal paper (Template/ folder).

**Status:** Configured for journal template; adjust paths as needed.

---

## Log File Structure

Each scenario folder contains:
```
logs/
├── appl_logs.csv          # Application layer: received packets
└── sender_summary.csv     # Per-vehicle transmission counts
```

**appl_logs.csv columns:**
- `t`: Timestamp (simulation time in seconds)
- `sender`: Sender vehicle ID
- `receiver`: Receiver vehicle ID
- `delay_ms`: End-to-end latency (milliseconds)
- `dist_m`: Distance between sender and receiver (meters)
- `spdu_size`: SPDU packet size (bytes)
- `signerType`: 0=digest only, 1=full certificate
- `verified`: 1=signature verified, 0=failed

**sender_summary.csv columns:**
- `sender`: Vehicle ID
- `total_sent`: Total packets transmitted by this vehicle

---

## How to Modify Analysis for Different Scenarios

### To Compare Different Scenarios:

1. **Identify scenario folders** in `Simulation Results/Simulation Logs Storage-selected/`
2. **Update script configuration:**

```python
# Example: Change to Scenario B (20 veh/km) comparison
ECDSA_DIR = BASE_DIR / "_3_ECDSA_B_LOS_20260407_202334/logs"
FALCON_DIR = BASE_DIR / "_15_Falcon_B_LOS_20260409_164915/logs"
TIME_LIMIT = 124.0  # Scenario B had crashes at 124s
```

3. **Update analysis period** if needed (TIME_LIMIT)
4. **Run scripts**
5. **Update .tex file** with new results

---

## LaTeX Compilation

### Conference Paper:
```bash
cd "Paper Writeup/IEEE_Conference_Template"
pdflatex conference_101719.tex
bibtex conference_101719
pdflatex conference_101719.tex
pdflatex conference_101719.tex
```

**Output:** `conference_101719.pdf`

### Journal Paper:
```bash
cd "Paper Writeup/Template"
pdflatex main.tex
bibtex main
pdflatex main.tex
pdflatex main.tex
```

**Output:** `main.pdf`

---

## Python Dependencies

Required packages:
```bash
pip install pandas numpy matplotlib
```

**Versions used:**
- pandas: 1.x or higher
- numpy: 1.x or higher
- matplotlib: 3.x or higher

**Notes:**
- Scripts use `matplotlib.use('Agg')` for non-interactive backend
- Publication-quality settings: serif fonts, 10pt, 300 DPI output

---

## Key Research Findings

### Computational Feasibility ✓
- Falcon-512 maintains latency parity with ECDSA (51.82 ms vs 51.92 ms)
- 100% signature verification success rate
- Signing/verification overhead (~2-3 ms) negligible compared to MAC queuing

### Spectrum Efficiency Challenge ✗
- **470% packet overhead** (1739 bytes vs 305 bytes with certificate)
- **4× subchannel consumption** at MCS 11
- **41% PDR degradation** at 30 veh/km (18.48% vs 31.32%)
- Falls below 90% safety-critical PDR threshold

### Deployment Implications
- ✓ Fits within SAE J3161 transport block limits (no protocol modifications needed)
- ✗ Direct ECDSA→Falcon migration not viable at 30+ veh/km without mitigation
- Requires: digest-based certificates, hybrid schemes, or infrastructure assistance

---

## Important Notes for Analysis

1. **Time Limits:** Some scenarios crashed mid-simulation (e.g., Scenario B at 124.4s). Always verify actual simulation duration before setting TIME_LIMIT.

2. **File Naming:** Some folders have timestamps (e.g., `_3_ECDSA_B_LOS_20260407_202334`), others don't (e.g., `simulation_logs__5_ECDSA_C_LOS`). Use `ls` to verify exact names.

3. **Unknown Senders:** Scripts filter out `sender == 'unknown'` entries automatically.

4. **PDR Estimation:** For time-limited analysis, scripts estimate sent packets as `num_senders × time_limit × 10 Hz`. This is approximate.

5. **Distance Bins:** Reception rate analysis uses 50m bins from 0-600m.

6. **Output Files:** Analysis script has file output **disabled** per user preference. Re-enable by uncommenting lines 241-246 in `analyze_scenario_B.py` if needed.

---

## Authorship

**Authors:**
- Akid Abrar (University of Alabama)
- Sagar Dasgupta (University of Alabama)
- Abdullah Al Mamun (Clemson University)
- Mizanur Rahman (University of Alabama)
- Mashrur Chowdhury (Clemson University)
- Ahmad Alsharif (University of Alabama)

**Affiliation Details:** See `conference_101719.tex` lines 17-33

---

## References & Standards

- **SAE J3161 (July 2024):** On-Board System Requirements for V2V Safety Communications
- **SAE J2945/1:** Minimum Performance Requirements (100 ms latency threshold)
- **IEEE 1609.2:** Security Services for V2X
- **NIST FIPS 206:** FN-DSA (Falcon) standard
- **3GPP TS 36.213/36.321:** LTE physical layer and MAC specifications

---

## Future Work Planned

1. **Additional scenarios:** Lower densities (10-20 veh/km), higher densities (40-60 veh/km)
2. **NLOS evaluation:** Urban obstruction scenarios
3. **Additional PQC algorithms:** Dilithium-2, hybrid ECDSA+Falcon schemes
4. **Certificate management:** Digest-only vs full certificate trade-offs
5. **Mitigation strategies:** Adaptive MCS, reduced transmission rates, SCMS integration

---

## Quick Start for New Claude Code Session

**On the analysis machine:**

1. Verify folder structure:
   ```bash
   ls -la "Simulation Results/Simulation Logs Storage-selected/"
   ```

2. Check current configuration:
   ```bash
   head -30 scripts/analyze_scenario_B.py
   head -30 scripts/conf_generate_figures.py
   ```

3. Run analysis:
   ```bash
   python3 scripts/analyze_scenario_B.py
   python3 scripts/conf_generate_figures.py
   ```

4. Review generated figures:
   ```bash
   ls -lh IEEE_Conference_Template/figures/
   ```

5. For modifications, ask Claude to:
   - Change scenario folders (ECDSA_DIR, FALCON_DIR)
   - Adjust time limits (TIME_LIMIT)
   - Modify figure aesthetics (colors, labels, layouts)
   - Update LaTeX paper with new results

**Common Tasks:**
- "Compare scenarios X and Y instead"
- "Use only first N seconds of data"
- "Generate separate figures for each metric"
- "Add statistical significance tests"
- "Export results to CSV table"

---

## Contact & Version Control

**Original Codebase:** Located on main machine at `/home/veins/src/simulte/`
**This Folder:** Portable analysis environment (Paper Writeup only)
**Sync Notes:** Figures and LaTeX changes can be copied back to main machine after analysis

---

*This document provides context for Claude Code to assist with post-processing, figure generation, and paper updates without access to the full simulation codebase.*
