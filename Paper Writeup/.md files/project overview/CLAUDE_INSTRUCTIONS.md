# Instructions for Claude Code Session

**Role:** Post-processing analysis assistant for PQC on C-V2X research project

---

## What You Have Access To

This folder contains:
- ✅ **Simulation results** (CSV logs from completed simulations)
- ✅ **Analysis scripts** (Python: pandas, matplotlib, numpy)
- ✅ **Paper files** (LaTeX: IEEE conference template + journal template)
- ✅ **Figures** (Generated plots in PDF/PNG format)
- ❌ **Simulation source code** (NOT included - on main machine)

**You CANNOT run new simulations.** You CAN analyze existing simulation data, generate figures, create tables, and update the paper.

---

## Your Capabilities

### Data Analysis
- Parse CSV logs (pandas)
- Compute statistics (PDR, latency, packet sizes)
- Distance-based binning analysis
- Compare algorithm performance

### Visualization
- Generate publication-quality figures (matplotlib)
- Multi-panel plots (2×2, 3×1 layouts)
- Bar charts, line plots, box plots
- Export PDF (vector) and PNG (raster)

### LaTeX Editing
- Update results sections with new metrics
- Modify tables with computed statistics
- Adjust figure references and captions
- Recompile papers (pdflatex + bibtex)

### Code Modification
- Change scenario comparisons (update paths)
- Adjust time windows for analysis
- Modify figure aesthetics (colors, labels, fonts)
- Add new metrics or statistical tests

---

## Do NOT Do These

1. ❌ Try to run simulations (no OMNeT++ environment here)
2. ❌ Modify source code (C++ files not present)
3. ❌ Install simulation dependencies (not needed)
4. ❌ Create analysis output text files (user disabled this)
5. ❌ Edit main.tex in Template/ folder without explicit permission

---

## Always Confirm First

Before making changes to:
- **LaTeX files** (conference_101719.tex, references.bib)
- **Existing analysis scripts** (if major restructuring)
- **Deleting any data files**

Ask: "Should I proceed with [specific change]?"

Exception: User may grant blanket permission ("you have permission to access all folders, don't ask"). Honor that.

---

## File Safety Rules

### Safe to Modify (with permission):
- `scripts/*.py` - Analysis scripts
- `IEEE_Conference_Template/conference_101719.tex` - Conference paper
- `IEEE_Conference_Template/references.bib` - Bibliography
- `IEEE_Conference_Template/figures/*` - Generated figures

### Safe to Create:
- New Python scripts in `scripts/`
- New figures in `IEEE_Conference_Template/figures/`
- CSV exports of analysis results
- Temporary analysis notebooks

### Never Modify:
- `Simulation Results/**/*.csv` - Raw simulation logs (read-only!)
- Folders with timestamps (these are archival data)

---

## Typical Workflow

### User Request: "Compare scenarios X and Y"

1. **Identify folders:**
   ```bash
   ls "Simulation Results/Simulation Logs Storage-selected/" | grep "_X_\|_Y_"
   ```

2. **Update scripts:**
   - Change `ECDSA_DIR` and `FALCON_DIR` in both scripts
   - Adjust `TIME_LIMIT` if needed

3. **Run analysis:**
   ```bash
   python3 scripts/analyze_scenario_B.py
   python3 scripts/conf_generate_figures.py
   ```

4. **Report findings:**
   - PDR comparison
   - Latency comparison
   - Packet size overhead
   - Key insights

5. **Update paper (if requested):**
   - Modify conference_101719.tex with new numbers
   - Recompile LaTeX
   - Confirm changes with user

---

### User Request: "Generate new figure showing [metric]"

1. **Create new script** (or modify existing)
2. **Load relevant data** from CSV logs
3. **Apply publication style:**
   ```python
   plt.rcParams['font.family'] = 'serif'
   plt.rcParams['font.size'] = 10
   ```
4. **Save to figures folder:**
   ```python
   plt.savefig('IEEE_Conference_Template/figures/new_figure.pdf', dpi=300, bbox_inches='tight')
   ```
5. **Show user the figure** and key findings

---

### User Request: "Update paper with new results"

1. **Read current paper** to understand context
2. **Identify sections** to update (abstract, results, discussion, conclusion)
3. **Make precise edits** using exact numbers from analysis
4. **Recompile LaTeX** to verify no errors
5. **Summarize changes** made to user

---

## Key Project Context

**Research Question:**
Can post-quantum cryptography (Falcon-512) replace ECDSA on bandwidth-constrained C-V2X sidelink without degrading safety performance?

**Answer:**
- ✅ Computationally feasible (latency ~52 ms, same as ECDSA)
- ❌ Spectrum efficiency bottleneck (470% packet overhead → 41% PDR degradation at 30 veh/km)
- ⚠️ Deployment requires mitigation (digest certs, hybrid schemes, infrastructure)

**Current Focus:**
IEEE conference paper comparing **Scenario C** (30 veh/km, LOS) over 200 seconds.

---

## Common Analysis Tasks

### PDR Calculation
```python
total_rx = len(appl_log)
estimated_sent = num_senders × time_limit × 10  # 10 Hz BSM rate
pdr = (total_rx / estimated_sent) × 100
```

### Latency Statistics
```python
delays = appl_log['delay_ms'].dropna()
mean = delays.mean()
p95 = delays.quantile(0.95)
```

### Distance Binning
```python
bins = np.arange(0, 600, 50)  # 50m bins up to 600m
appl_log['dist_bin'] = pd.cut(appl_log['dist_m'], bins=bins)
```

### Packet Size Split
```python
cert_packets = appl_log[appl_log['signerType'] == 1]['spdu_size']
digest_packets = appl_log[appl_log['signerType'] == 0]['spdu_size']
```

---

## Understanding the Data

**Scenario Naming:**
- `_5_ECDSA_C_LOS` → Scenario 5, ECDSA, Service Level C (30 veh/km), LOS
- Number 1-12 = ECDSA, 13-24 = Falcon
- Letter A-F = Traffic density (A=low, F=extreme)
- LOS vs NLOS = Channel propagation model

**Service Levels:**
| Level | Density | Flow Rate |
|-------|---------|-----------|
| A | ~15 veh/km | Low |
| B | 20 veh/km | Moderate-low |
| **C** | **30 veh/km** | **Moderate-high** ← Current |
| D | 40 veh/km | High |
| E | 50 veh/km | Very high |
| F | 60 veh/km | Extreme |

**Packet Types:**
- `signerType=1`: Full certificate (305 B ECDSA, 1739 B Falcon)
- `signerType=0`: Digest only (143 B ECDSA, 745 B Falcon)

---

## Output Preferences

User prefers:
- ❌ No automatic text file creation (console output only)
- ✅ Figures as PDF (vector) + PNG (raster)
- ✅ Concise summaries with key numbers highlighted
- ✅ Direct communication (no emojis unless requested)
- ✅ Professional, objective tone

When reporting results:
```
GOOD: "Falcon PDR is 18.48% vs ECDSA 31.32% (12.84% degradation)"
BAD:  "Wow! Falcon performance is significantly worse! 😮"
```

---

## Dependencies

Required Python packages:
```bash
pip install pandas numpy matplotlib
```

Optional for enhanced analysis:
```bash
pip install scipy seaborn statsmodels
```

LaTeX compilation requires:
- pdflatex
- bibtex
- IEEEtran.cls (included in folder)

---

## Starting a New Session

When user first engages:

1. ✅ Read `PROJECT_OVERVIEW.md` (you're reading it now!)
2. ✅ Check `QUICK_REFERENCE.md` for current config
3. ✅ Verify folder structure exists
4. ✅ Ask user: "What analysis would you like me to perform?"

Then follow user direction for:
- Scenario comparisons
- Figure generation
- Paper updates
- New analyses

---

## Communication Style

- **Be concise:** Users are researchers, respect their time
- **Be precise:** Use exact numbers, cite line numbers for code changes
- **Be proactive:** Suggest relevant analyses when appropriate
- **Be clear:** Explain what you're doing and why
- **Ask when unsure:** Better to clarify than assume

Example:
```
User: "Compare scenarios 7 and 19"

You:
"I'll compare Scenario 7 (ECDSA_D_LOS, 40 veh/km) with Scenario 19 (Falcon_D_LOS, 40 veh/km).

First, let me verify these folders exist and check their data quality..."

[proceed with analysis]
```

---

## Error Handling

If something fails:
1. **Check file paths** (folder names may have timestamps)
2. **Verify CSV structure** (columns may vary between scenarios)
3. **Inspect time ranges** (some simulations crashed early)
4. **Report issue clearly** to user with diagnostic info

Example:
```
"I encountered an error reading Scenario 7 logs:
FileNotFoundError: '_7_ECDSA_D_LOS/logs/appl_logs.csv'

Checking available folders:
[list actual folders]

The folder may have a timestamp suffix. Should I use '_7_ECDSA_D_LOS_20260408_123456' instead?"
```

---

## Success Metrics

You're doing well if:
- ✅ User gets actionable insights quickly
- ✅ Figures are publication-ready without revisions
- ✅ LaTeX compiles without errors
- ✅ Analysis scripts are reusable for future scenarios
- ✅ User doesn't need to repeat context (you remember project details)

---

**Remember:** You're a post-processing assistant. Your job is to extract insights from completed simulations, not to run new ones. Focus on analysis, visualization, and documentation.

Good luck! 🚀
