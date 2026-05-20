# Transfer Checklist - Moving to Analysis Machine

**Date:** May 4, 2026
**Destination:** Separate analysis machine with Claude Code

---

## ✅ Pre-Transfer Verification

### Check Folder Contents
```bash
cd "Paper Writeup"

# Verify documentation exists
ls -lh *.md
# Expected: README.md, PROJECT_OVERVIEW.md, QUICK_REFERENCE.md, CLAUDE_INSTRUCTIONS.md, TRANSFER_CHECKLIST.md

# Verify simulation results
ls "Simulation Results/Simulation Logs Storage-selected/" | wc -l
# Expected: ~18 folders (various scenarios)

# Verify scripts
ls scripts/*.py
# Expected: analyze_scenario_B.py, conf_generate_figures.py, jour_generate_figures.py

# Verify papers
ls IEEE_Conference_Template/*.tex
ls Template/*.tex
# Expected: conference_101719.tex, main.tex

# Verify current figures
ls IEEE_Conference_Template/figures/
# Expected: performance_comparison.pdf, performance_comparison.png
```

---

## 📦 What to Transfer

### Required Folders/Files:
- ✅ **Entire "Paper Writeup" folder**
  - Including all subfolders and files
  - Preserves folder structure

### Size Estimate:
```bash
du -sh "Paper Writeup"
# Expected: ~3-4 GB (due to Simulation Results)
```

### Compression (Optional):
```bash
# On main machine:
cd /home/veins/src/simulte
tar -czf paper_writeup_analysis.tar.gz "Paper Writeup/"

# Transfer compressed file, then on analysis machine:
tar -xzf paper_writeup_analysis.tar.gz
```

---

## 🔧 Post-Transfer Setup (On Analysis Machine)

### 1. Verify Transfer Integrity
```bash
cd "Paper Writeup"

# Check all documentation files
ls -lh *.md

# Verify simulation data
find "Simulation Results/Simulation Logs Storage-selected" -name "appl_logs.csv" | wc -l
# Expected: ~18 (one per scenario)

# Check scripts
python3 -m py_compile scripts/*.py
# Should complete without errors
```

### 2. Install Python Dependencies
```bash
# Install required packages
pip install pandas numpy matplotlib

# Verify installation
python3 -c "import pandas, numpy, matplotlib; print('Dependencies OK')"
```

### 3. Test Scripts
```bash
cd "Paper Writeup"

# Test analysis script (should print statistics)
python3 scripts/analyze_scenario_B.py | head -20

# Test figure generation (should create PDFs)
python3 scripts/conf_generate_figures.py
ls -lh IEEE_Conference_Template/figures/performance_comparison.pdf
```

### 4. Test LaTeX Compilation
```bash
cd "Paper Writeup/IEEE_Conference_Template"

# Verify LaTeX tools available
which pdflatex bibtex
# Should return paths (e.g., /usr/bin/pdflatex)

# Test compilation
pdflatex conference_101719.tex
# Should complete without fatal errors

# Full compilation cycle
pdflatex conference_101719.tex && bibtex conference_101719 && pdflatex conference_101719.tex && pdflatex conference_101719.tex
ls -lh conference_101719.pdf
# Expected: ~140-150 KB PDF file
```

### 5. Install Claude Code (If Not Already Installed)
```bash
# Follow Claude Code installation instructions
# Typically: npm install -g @anthropic-ai/claude-code

# Verify installation
claude --version
```

---

## 🤖 Starting Claude Code Session

### 1. Navigate to Project Folder
```bash
cd "Paper Writeup"
```

### 2. Initialize Claude Code
```bash
claude
```

### 3. First Interaction
```
Hi! I'm working on PQC C-V2X analysis. Please read README.md,
CLAUDE_INSTRUCTIONS.md, and PROJECT_OVERVIEW.md to understand
the project context.
```

### 4. Verify Claude Understands Context
Ask Claude:
- "What scenarios are currently being compared?"
- "What are the key findings so far?"
- "Where are the analysis scripts located?"

Expected answers:
- Scenario 5 (ECDSA_C_LOS) vs Scenario 17 (Falcon_C_LOS)
- Falcon shows 41% PDR degradation due to 470% packet overhead
- Scripts in `scripts/` folder

---

## ✅ Transfer Complete When:

- [ ] All documentation files present (README.md, PROJECT_OVERVIEW.md, etc.)
- [ ] Simulation results folder contains ~18 scenario folders
- [ ] Each scenario has `logs/appl_logs.csv` and `logs/sender_summary.csv`
- [ ] Python scripts execute without import errors
- [ ] Scripts can read CSV files and print statistics
- [ ] Figure generation creates PDF files successfully
- [ ] LaTeX compilation produces PDF without fatal errors
- [ ] Claude Code can read all markdown documentation files
- [ ] Claude Code can execute Python scripts via Bash tool
- [ ] Claude Code understands project context (test with questions)

---

## 🔍 Troubleshooting

### Issue: Python Import Errors
```bash
# Solution: Install missing packages
pip install pandas numpy matplotlib scipy
```

### Issue: CSV File Not Found
```bash
# Solution: Verify folder names (may have timestamps)
ls "Simulation Results/Simulation Logs Storage-selected/"
# Update script paths to match actual folder names
```

### Issue: LaTeX Compilation Fails
```bash
# Solution: Install TeX Live
sudo apt-get install texlive-full  # Debian/Ubuntu
# or
brew install --cask mactex  # macOS
```

### Issue: Figures Not Generating
```bash
# Solution: Verify matplotlib backend
python3 -c "import matplotlib; print(matplotlib.get_backend())"
# Should be 'agg' or similar non-interactive backend
```

### Issue: Claude Can't Read Files
```bash
# Solution: Check file permissions
chmod +r *.md
chmod +r IEEE_Conference_Template/*.tex
chmod +r scripts/*.py
```

---

## 📋 Analysis Machine Requirements

### Minimum Specifications:
- **OS:** Linux, macOS, or Windows (with WSL)
- **RAM:** 8 GB (16 GB recommended for large datasets)
- **Storage:** 10 GB free space
- **Python:** 3.7 or higher
- **LaTeX:** TeX Live 2020 or higher

### Software Dependencies:
```bash
# Python packages
pip install pandas>=1.0.0 numpy>=1.18.0 matplotlib>=3.0.0

# Optional (for advanced analysis)
pip install scipy seaborn statsmodels jupyter
```

### LaTeX Packages (usually included in TeX Live):
- IEEEtran.cls
- cite.sty
- graphicx.sty
- booktabs.sty
- amsmath.sty

---

## 🔄 Syncing Back to Main Machine

After making changes on analysis machine, sync back:

### Modified Files to Copy Back:
1. **Figures:** `IEEE_Conference_Template/figures/*.pdf`
2. **LaTeX:** `IEEE_Conference_Template/conference_101719.tex`
3. **Bibliography:** `IEEE_Conference_Template/references.bib` (if updated)
4. **New scripts:** `scripts/*.py` (if created)

### What NOT to Copy Back:
- Simulation Results/ (unchanged, already on main machine)
- Auxiliary LaTeX files (.aux, .log, .bbl, .blg)
- Python cache (`__pycache__/`)

### Sync Command (from analysis to main):
```bash
# On analysis machine (push to main)
rsync -av --include='*.pdf' --include='*.tex' --include='*.bib' --include='*.py' \
    --exclude='*.aux' --exclude='*.log' --exclude='*.blg' \
    "Paper Writeup/" user@main-machine:/home/veins/src/simulte/"Paper Writeup/"
```

---

## 📞 Support Resources

**On Main Machine:**
- Full codebase: `/home/veins/src/simulte/`
- Claude Code session with full project memory
- Simulation execution capabilities

**On Analysis Machine:**
- Post-processing only (no simulations)
- Fresh Claude Code sessions (use documentation to establish context)
- Figure generation and paper updates

**Documentation Hierarchy:**
1. Start: `README.md`
2. Quick lookup: `QUICK_REFERENCE.md`
3. Full context: `PROJECT_OVERVIEW.md`
4. Claude instructions: `CLAUDE_INSTRUCTIONS.md`

---

## ✅ Final Checklist

Before starting work on analysis machine:

- [ ] Transfer complete and verified
- [ ] Python dependencies installed
- [ ] LaTeX environment working
- [ ] Test scripts execute successfully
- [ ] Test figures generate correctly
- [ ] Test LaTeX compiles to PDF
- [ ] Claude Code installed
- [ ] Claude Code can read documentation
- [ ] Claude Code can execute scripts
- [ ] Claude Code understands project context

**Status:** Ready for analysis! 🚀

---

**Next Steps:**
1. Navigate to `Paper Writeup/` folder
2. Launch Claude Code: `claude`
3. Ask Claude to read documentation
4. Begin analysis tasks

Good luck with your research!
