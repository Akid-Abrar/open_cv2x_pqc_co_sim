# Quick Reference Card

## Current Analysis Configuration (May 4, 2026)

### Active Scenarios
- **ECDSA:** Scenario 5 (`simulation_logs__5_ECDSA_C_LOS`)
- **Falcon:** Scenario 17 (`simulation_logs__17_Falcon_C_LOS`)
- **Density:** 30 veh/km (Service Level C)
- **Channel:** LOS
- **Time:** 200 seconds

### Key Results
```
ECDSA PDR:    31.32%
Falcon PDR:   18.48%  (↓ 12.84%)
ECDSA Lat:    51.92 ms
Falcon Lat:   51.82 ms
Packet Size:  305 B → 1739 B (↑ 470%)
```

---

## Scenario Quick Reference

| # | Algorithm | Density | Channel | Folder Name |
|---|-----------|---------|---------|-------------|
| 1-2 | ECDSA | A (~15 veh/km) | LOS/NLOS | `_1_ECDSA_A_LOS` |
| 3-4 | ECDSA | B (20 veh/km) | LOS/NLOS | `_3_ECDSA_B_LOS` |
| **5-6** | **ECDSA** | **C (30 veh/km)** | **LOS**/NLOS | `_5_ECDSA_C_LOS` ← **IN USE** |
| 7-8 | ECDSA | D (40 veh/km) | LOS/NLOS | `_7_ECDSA_D_LOS` |
| 9-10 | ECDSA | E (50 veh/km) | LOS/NLOS | `_9_ECDSA_E_LOS` |
| 11-12 | ECDSA | F (60 veh/km) | LOS/NLOS | `_11_ECDSA_F_LOS` |
| 13-14 | Falcon | A (~15 veh/km) | LOS/NLOS | `_13_Falcon_A_LOS` |
| 15-16 | Falcon | B (20 veh/km) | LOS/NLOS | `_15_Falcon_B_LOS` |
| **17-18** | **Falcon** | **C (30 veh/km)** | **LOS**/NLOS | `_17_Falcon_C_LOS` ← **IN USE** |
| 19-20 | Falcon | D (40 veh/km) | LOS/NLOS | `_19_Falcon_D_LOS` |
| 21-22 | Falcon | E (50 veh/km) | LOS/NLOS | `_21_Falcon_E_LOS` |
| 23-24 | Falcon | F (60 veh/km) | LOS/NLOS | `_23_Falcon_F_LOS` |

---

## Script Commands

### Run Analysis
```bash
cd "Paper Writeup"
python3 scripts/analyze_scenario_B.py
```

### Generate Conference Figures
```bash
cd "Paper Writeup"
python3 scripts/conf_generate_figures.py
```

### Compile Conference Paper
```bash
cd "Paper Writeup/IEEE_Conference_Template"
pdflatex conference_101719.tex && bibtex conference_101719 && pdflatex conference_101719.tex && pdflatex conference_101719.tex
```

---

## File Locations

| File | Purpose |
|------|---------|
| `scripts/analyze_scenario_B.py` | Statistical analysis |
| `scripts/conf_generate_figures.py` | Figure generation |
| `IEEE_Conference_Template/conference_101719.tex` | Conference paper |
| `IEEE_Conference_Template/references.bib` | Bibliography |
| `IEEE_Conference_Template/figures/performance_comparison.pdf` | Main figure |

---

## To Change Scenarios

Edit both scripts:
```python
# In analyze_scenario_B.py and conf_generate_figures.py
ECDSA_DIR = BASE_DIR / "simulation_logs__5_ECDSA_C_LOS/logs"    # ← Change this
FALCON_DIR = BASE_DIR / "_17_Falcon_C_LOS/logs"                # ← Change this
TIME_LIMIT = 200.0                                              # ← Change this
```

---

## Log File Columns

**appl_logs.csv:**
- `t` - Timestamp (s)
- `sender` - TX vehicle ID
- `receiver` - RX vehicle ID
- `delay_ms` - Latency (ms)
- `dist_m` - Distance (m)
- `spdu_size` - Packet size (bytes)
- `signerType` - 0=digest, 1=cert
- `verified` - 1=success, 0=fail

**sender_summary.csv:**
- `sender` - Vehicle ID
- `total_sent` - Packets transmitted

---

## Common Claude Requests

```
"Compare scenarios X and Y"
"Use first N seconds only"
"Change figure colors/layout"
"Add error bars to plots"
"Export table to CSV"
"Update LaTeX with new results"
```
