#!/bin/bash
# Monitored simulation runner - captures exit codes and timing

if [ $# -eq 0 ]; then
    echo "Usage: $0 <scenario_name>"
    echo "Example: $0 _6_ECDSA_C_NLOS"
    exit 1
fi

SCENARIO="$1"
LOG_FILE="run_${SCENARIO}.log"
MONITOR_LOG="run_monitor.log"

echo "=======================================" | tee -a "$MONITOR_LOG"
echo "Starting: $SCENARIO" | tee -a "$MONITOR_LOG"
echo "Started at: $(date)" | tee -a "$MONITOR_LOG"
echo "PID: $$" | tee -a "$MONITOR_LOG"
echo "=======================================" | tee -a "$MONITOR_LOG"

START_TIME=$(date +%s)

# Run the simulation
./run -u Cmdenv -c "$SCENARIO" -r 0 > "$LOG_FILE" 2>&1
EXIT_CODE=$?

END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))
HOURS=$((DURATION / 3600))
MINUTES=$(((DURATION % 3600) / 60))

echo "" | tee -a "$MONITOR_LOG"
echo "=======================================" | tee -a "$MONITOR_LOG"
echo "Scenario: $SCENARIO" | tee -a "$MONITOR_LOG"
echo "Exit code: $EXIT_CODE" | tee -a "$MONITOR_LOG"
echo "Ended at: $(date)" | tee -a "$MONITOR_LOG"
echo "Duration: ${HOURS}h ${MINUTES}m (${DURATION}s)" | tee -a "$MONITOR_LOG"

if [ $EXIT_CODE -eq 0 ]; then
    echo "Status: SUCCESS ✓" | tee -a "$MONITOR_LOG"
    # Check if simulation actually completed
    if grep -q "End run" "$LOG_FILE"; then
        echo "Verification: Simulation completed successfully" | tee -a "$MONITOR_LOG"
    else
        echo "WARNING: Exit code 0 but no 'End run' message found!" | tee -a "$MONITOR_LOG"
        echo "Last 20 lines of log:" | tee -a "$MONITOR_LOG"
        tail -20 "$LOG_FILE" | tee -a "$MONITOR_LOG"
    fi
else
    echo "Status: FAILED ✗" | tee -a "$MONITOR_LOG"
    echo "" | tee -a "$MONITOR_LOG"
    echo "Last 50 lines of simulation log:" | tee -a "$MONITOR_LOG"
    tail -50 "$LOG_FILE" | tee -a "$MONITOR_LOG"
fi

echo "=======================================" | tee -a "$MONITOR_LOG"
echo "" | tee -a "$MONITOR_LOG"

exit $EXIT_CODE
