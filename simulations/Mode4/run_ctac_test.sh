#!/bin/bash
# Run a single scenario and archive all results to target folder
# Usage: ./run_ctac_test.sh <scenario_name> <target_folder>
# Example: ./run_ctac_test.sh _28_DCC_CONGESTION results_dcc_baseline

set -e  # Exit on error

if [ $# -ne 2 ]; then
    echo "Usage: $0 <scenario_name> <target_folder>"
    echo ""
    echo "Examples:"
    echo "  $0 _28_DCC_CONGESTION results_dcc_300veh"
    echo "  $0 _29_CTAC_CONGESTION results_ctac_k3_300veh"
    echo ""
    echo "This script will:"
    echo "  1. Run the specified scenario"
    echo "  2. Create target_folder/"
    echo "  3. Move all .sca, .vec, and CSV logs to target_folder/"
    exit 1
fi

SCENARIO="$1"
TARGET_DIR="$2"

# Check if target directory already exists
if [ -d "$TARGET_DIR" ]; then
    echo "ERROR: Target directory '$TARGET_DIR' already exists!"
    echo "Please choose a different name or remove the existing directory."
    exit 1
fi

echo "======================================="
echo "CTAC Test Runner"
echo "======================================="
echo "Scenario:      $SCENARIO"
echo "Target folder: $TARGET_DIR"
echo "Started at:    $(date)"
echo "======================================="
echo ""

START_TIME=$(date +%s)

# Run the simulation
echo "Running simulation..."
./run -u Cmdenv -c "$SCENARIO" -r 0

EXIT_CODE=$?
END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))
MINUTES=$((DURATION / 60))
SECONDS=$((DURATION % 60))

echo ""
echo "======================================="
echo "Simulation completed"
echo "Exit code: $EXIT_CODE"
echo "Duration:  ${MINUTES}m ${SECONDS}s"
echo "======================================="

if [ $EXIT_CODE -ne 0 ]; then
    echo "ERROR: Simulation failed with exit code $EXIT_CODE"
    exit $EXIT_CODE
fi

echo ""
echo "Archiving results to $TARGET_DIR..."

# Create target directory
mkdir -p "$TARGET_DIR"

# Move .sca and .vec files from results/
echo "  - Moving .sca and .vec files..."
mv results/${SCENARIO}-#0.sca "$TARGET_DIR/" 2>/dev/null || echo "    (no .sca file found)"
mv results/${SCENARIO}-#0.vec "$TARGET_DIR/" 2>/dev/null || echo "    (no .vec file found)"

# Move CSV logs from simulation_logs__SCENARIO/
LOG_DIR="simulation_logs__${SCENARIO}"
if [ -d "$LOG_DIR" ]; then
    echo "  - Moving CSV logs..."
    mv "$LOG_DIR"/*.csv "$TARGET_DIR/" 2>/dev/null || echo "    (no CSV files found)"
    # Remove empty log directory
    rmdir "$LOG_DIR" 2>/dev/null || true
else
    echo "  - No simulation logs directory found"
fi

# Create metadata file
echo "  - Creating metadata..."
cat > "$TARGET_DIR/run_metadata.txt" <<EOF
Scenario:        $SCENARIO
Run number:      0
Execution date:  $(date)
Duration:        ${MINUTES}m ${SECONDS}s
Exit code:       $EXIT_CODE
Working dir:     $(pwd)
OMNeT++ version: $(./run --version 2>&1 | head -1 || echo "Unknown")
EOF

echo ""
echo "======================================="
echo "SUCCESS!"
echo "======================================="
echo "Results archived to: $TARGET_DIR/"
echo ""
echo "Contents:"
ls -lh "$TARGET_DIR/"

echo ""
echo "To analyze results:"
echo "  cd ../../analysis"
echo "  python3 your_analysis_script.py ../$TARGET_DIR/"
echo ""

exit 0
