# Project: PQC on C-V2X Mode 4 Sidelink

## Codebase
- This is **openCV2X** (modified), named `simulte` for compilation compatibility only
- Do NOT refer to it as SimuLTE in paper writeups

## Key Paths
- Paper: `Paper Writeup/Template/main.tex`, `main.bib`
- Simulation config: `simulations/Mode4/omnetpp.ini`
- Sidelink config: `simulations/Mode4/sidelink_configuration.xml`
- MAC layer: `src/stack/mac/layer/LteMacVUeMode4.cc`
- App layer: `src/apps/mode4App/Mode4App.cc`
- PQC crypto: `src/apps/mode4App/pqcdsa.cc` / `pqcdsa.h`

## Paper Writing Rules
- Do NOT edit main.tex or main.bib without explicit permission
- If bib changes are made, flag it so user can sync to Overleaf
- Save analysis/response text files to `analysis_notes/` with date prefix `MM_DD_YYYY_`
- ICA (Intersection Collision Avoidance) is OUT OF SCOPE — do not include

## Simulation Facts
- Bandwidth: 20 MHz (100 RBs), NOT 10 MHz
- Subchannel: 10 RBs x 10 subchannels, MCS 5-11
- Default crypto: Falcon-512 (via `PQCDSA_ALGO` env var)
- Falcon-512 sig: 649-666 B (variable), pubkey: 897 B
- Build paper: `cd "Paper Writeup/Template" && ./build.sh`
