# PQC on C-V2X Sidelink - Analysis Environment

**Research:** Post-Quantum Cryptography (Falcon-512) Performance on C-V2X Mode 4
**Institution:** University of Alabama & Clemson University
**Last Updated:** May 4, 2026

---

## 📁 This Folder Contains

- **Simulation Results** - CSV logs from OMNeT++ simulations (24 scenarios)
- **Analysis Scripts** - Python scripts for statistical analysis and figure generation
- **Conference Paper** - IEEE format LaTeX paper (current focus)
- **Journal Paper** - Full journal paper template (future work)
- **Documentation** - Complete project overview and instructions

---

## 🚀 Quick Start

### For Humans:
1. **Read first:** `PROJECT_OVERVIEW.md` (comprehensive project background)
2. **Quick reference:** `QUICK_REFERENCE.md` (commands, scenarios, current config)
3. **Run analysis:**
   ```bash
   python3 scripts/analyze_scenario_B.py
   python3 scripts/conf_generate_figures.py
   ```

### For Claude Code:
1. **Read first:** `CLAUDE_INSTRUCTIONS.md` (your role and capabilities)
2. **Then read:** `PROJECT_OVERVIEW.md` (project context)
3. **Quick lookup:** `QUICK_REFERENCE.md` (current configuration)
4. **Ask user:** "What analysis would you like me to perform?"

---

## 📊 Current Analysis Status

**Scenarios:** Comparing Scenario 5 (ECDSA_C_LOS) vs Scenario 17 (Falcon_C_LOS)
**Traffic:** 30 veh/km (moderate-high density)
**Channel:** LOS (line-of-sight)
**Analysis Period:** 200 seconds

**Key Finding:** Falcon-512 achieves 18.48% PDR vs 31.32% for ECDSA (41% degradation due to 470% packet size overhead)

---

## 📚 Documentation Files

| File | Purpose | Read When |
|------|---------|-----------|
| **README.md** | This file - navigation hub | Start here |
| **PROJECT_OVERVIEW.md** | Complete project documentation | Need full context |
| **QUICK_REFERENCE.md** | Commands, scenarios, quick facts | Need quick lookup |
| **CLAUDE_INSTRUCTIONS.md** | Instructions for Claude Code | New Claude session |

---

## 📂 Folder Structure

```
Paper Writeup/
├── README.md                           ← You are here
├── PROJECT_OVERVIEW.md                 ← Full documentation
├── QUICK_REFERENCE.md                  ← Quick lookup
├── CLAUDE_INSTRUCTIONS.md              ← For Claude Code
│
├── IEEE_Conference_Template/           ← Conference paper (current)
│   ├── conference_101719.tex
│   ├── references.bib
│   └── figures/
│       └── performance_comparison.pdf
│
├── Template/                           ← Journal paper (future)
│   ├── main.tex
│   └── main.bib
│
├── Simulation Results/                 ← Raw simulation data
│   └── Simulation Logs Storage-selected/
│       ├── simulation_logs__5_ECDSA_C_LOS/
│       ├── simulation_logs__17_Falcon_C_LOS/
│       └── [22 other scenarios...]
│
└── scripts/                            ← Analysis & figures
    ├── analyze_scenario_B.py
    ├── conf_generate_figures.py
    └── jour_generate_figures.py
```

---

## 🔧 Dependencies

### Python (for analysis):
```bash
pip install pandas numpy matplotlib
```

### LaTeX (for paper compilation):
- pdflatex (included in TeX Live or MiKTeX)
- bibtex
- IEEEtran class (included in folder)

---

## 📖 Common Tasks

### Run Statistical Analysis
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
pdflatex conference_101719.tex
bibtex conference_101719
pdflatex conference_101719.tex
pdflatex conference_101719.tex
```

### View Results
```bash
# Check generated figures
ls -lh IEEE_Conference_Template/figures/

# View compiled paper
evince IEEE_Conference_Template/conference_101719.pdf  # Linux
open IEEE_Conference_Template/conference_101719.pdf    # macOS
```

---

## 🎯 Project Goals

**Primary Objective:**
Quantify the performance impact of replacing ECDSA P-256 with Falcon-512 on C-V2X Mode 4 sidelink communications.

**Key Questions:**
1. Is Falcon-512 computationally feasible? → ✅ Yes (latency parity)
2. Does packet size overhead affect PDR? → ✅ Yes (41% degradation at 30 veh/km)
3. Can Falcon fit in sidelink TBS? → ✅ Yes (1739 B < 3496 B max TBS)
4. Is direct migration viable? → ❌ No (requires mitigation strategies)

**Findings:**
- Spectrum efficiency, not computation, is the bottleneck
- Deployment requires digest certificates, hybrid schemes, or infrastructure support

---

## 📝 Citation

If you use this work, please cite:

```bibtex
@inproceedings{mamun2026post,
  title={Post-Quantum Cryptographic Algorithms on C-V2X Sidelink: A Performance Evaluation},
  author={Mamun, Abdullah Al and Abrar, Akid and Dasgupta, Sagar and Rahman, Mizanur and Chowdhury, Mashrur and Alsharif, Ahmad},
  booktitle={Proceedings of the IEEE Conference on Communications and Network Security (CNS)},
  year={2026},
  organization={IEEE},
  note={Under Submission}
}
```

---

## 👥 Authors

- **Akid Abrar** - University of Alabama
- **Sagar Dasgupta** - University of Alabama
- **Abdullah Al Mamun** - Clemson University
- **Mizanur Rahman** - University of Alabama
- **Mashrur Chowdhury** - Clemson University
- **Ahmad Alsharif** - University of Alabama

---

## 📞 Support

**For technical questions:**
- Check `PROJECT_OVERVIEW.md` for detailed explanations
- Check `QUICK_REFERENCE.md` for quick answers
- Ask Claude Code for analysis assistance

**For simulation source code:**
- Main codebase located on primary machine: `/home/veins/src/simulte/`
- This folder is analysis-only (no simulation capabilities)

---

## 🔄 Version History

**May 4, 2026:**
- Updated to Scenario C (30 veh/km) comparison
- Extended analysis period to 200 seconds
- Generated new conference paper figures
- Updated paper with revised results

**May 3, 2026:**
- Initial Scenario B (20 veh/km) comparison
- 124-second analysis window
- First draft conference paper submission

---

## ⚠️ Important Notes

1. **This is analysis-only environment** - Cannot run new simulations
2. **Raw logs are read-only** - Never modify CSV files in Simulation Results/
3. **LaTeX changes need user approval** - Confirm before editing papers
4. **Some scenarios incomplete** - Check simulation duration before analysis
5. **Folder names may vary** - Some have timestamps, verify with `ls`

---

## 🎓 Related Standards

- **SAE J3161 (July 2024)** - V2V Safety Communications Requirements
- **SAE J2945/1** - V2V Minimum Performance Requirements
- **IEEE 1609.2** - V2X Security Services
- **NIST FIPS 206** - FN-DSA (Falcon) Standard
- **3GPP TS 36.213** - LTE Physical Layer Procedures

---

**For complete project documentation, see `PROJECT_OVERVIEW.md`**
