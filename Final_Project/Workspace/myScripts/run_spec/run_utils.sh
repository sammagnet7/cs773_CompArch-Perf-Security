#!/bin/bash

SCRIPT_PATH="$(realpath "${BASH_SOURCE[0]}")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
BASE="$(realpath "$SCRIPT_DIR/../../")"

kill_all_gem5() {
  echo "🔴 Killing all gem5.opt processes..."
  ps -ef | grep gem5.opt | grep "$SESSION" | grep -v grep | awk '{print $2}' | xargs -r kill -9
  echo "✅ Done."
}

TOTAL_INSTS=10000000000;
watch_all_gem5() {
  echo "👀 Watching gem5.opt processes... (Press Ctrl+C to exit early)"
  while true; do
    clear

    pids=$(pgrep -f gem5.opt)

    if [ -z "$pids" ]; then
      echo "✅ No gem5.opt processes running."
      break
    fi

    for file in $SPEC17_DIR; do
      benchmark=$(echo $file | sed -E 's|.*/CPU/[0-9]+\.([^/]+)/run/run_base_refspeed_spec-64.0000/([a-zA-Z0-9]+_[a-zA-Z0-9]+)_[^/]+/.*|\1 \2|');
      benchmark_name=$(echo $benchmark | cut -d' ' -f 1)
      benchmark_mode=$(echo $benchmark | cut -d' ' -f 2)

      log_last_line=$(cat $file | tail -1)
      if [[ "$log_last_line" =~ ^Exiting\ @\ tick\ [0-9]+.*max\ instruction\ count$ ]]; then
        commited=$TOTAL_INSTS
      else
        commited=$(echo $log_last_line | cut -d':' -f 4 | cut -d',' -f 1);
      fi
      completed=$(echo "scale=1; $commited*100/$TOTAL_INSTS" | bc -l);
      pid=$(echo $pids | grep -v ${benchmark_mode}_${SESSION} | awk '{print $2}')
      elapsed_time=$(ps -p $pid -o etime= | tr -d '[:space:]')

      elapsed_seconds=0
      if [[ "$elapsed_time" =~ ([0-9]+)-([0-9]+):([0-9]+):([0-9]+) ]]; then
          # Format: days-hours-minutes-seconds
          elapsed_seconds=$(( (${BASH_REMATCH[1]} * 86400) + (${BASH_REMATCH[2]} * 3600) + (${BASH_REMATCH[3]} * 60) + ${BASH_REMATCH[4]} ))
      elif [[ "$elapsed_time" =~ ([0-9]+)-([0-9]+):([0-9]+) ]]; then
          # Format: days-hours-minutes
          elapsed_seconds=$(( (${BASH_REMATCH[1]} * 86400) + (${BASH_REMATCH[2]} * 3600) + ${BASH_REMATCH[3]} ))
      elif [[ "$elapsed_time" =~ ([0-9]+):([0-9]+):([0-9]+) ]]; then
          # Format: hours-minutes-seconds
          elapsed_seconds=$(( (${BASH_REMATCH[1]} * 3600) + (${BASH_REMATCH[2]} * 60) + ${BASH_REMATCH[3]} ))
      elif [[ "$elapsed_time" =~ ([0-9]+)-([0-9]+) ]]; then
          # Format: days-hours
          elapsed_seconds=$(( (${BASH_REMATCH[1]} * 86400) + ${BASH_REMATCH[2]} * 3600 ))
      elif [[ "$elapsed_time" =~ ([0-9]+):([0-9]+) ]]; then
          # Format: minutes-seconds
          elapsed_seconds=$(( (${BASH_REMATCH[1]} * 60) + ${BASH_REMATCH[2]} ))
      fi

      remaining_seconds=$(echo "scale=1; ($elapsed_seconds * (100 - $completed)) / $completed" | bc)

      eta_seconds=$(echo "$remaining_seconds" | bc)

      eta_hours=$(echo "scale=0; $eta_seconds / 3600" | bc)
      eta_minutes=$(echo "scale=0; ($eta_seconds % 3600) / 60" | bc)
      eta_seconds=$(echo "scale=0; $eta_seconds % 60" | bc)

      eta=$(printf "%02d:%02d:%02d" $eta_hours $eta_minutes $eta_seconds 2> /dev/null)

      RUNNING="🟢"
      FINISHED="✅"
      FAILED="❌"
      
      STATUS_ICON=$RUNNING;
      if [[ "$completed" == "100.0" ]] then
          STATUS_ICON=$FINISHED
      fi

      printf "$STATUS_ICON %-15s %-20s %-10s elapsed: %s   remaining: %s\n" "$benchmark_name" "$benchmark_mode" "$completed%" "$elapsed_time" "$eta"
    done
    sleep 10
  done
}


print_help() {
  echo "Usage: $0 [option] <session>"
  echo ""
  echo "Options:"
  echo "  -k, --kill-gem5     Kill all running gem5.opt processes"
  echo "  -w, --watch-gem5    Display all currently running gem5.opt processes"
  echo "  -h, --help          Show this help message"
}

SESSION="$2"
SPEC17_DIR="$BASE/SPEC17_workloads/benchspec/CPU/*/run/run_base_refspeed_spec-64.0000/*_${SESSION}/run_base.log"

# Parse CLI arguments
case "$1" in
  -k|--kill-gem5)
    kill_all_gem5
    exit 0
    ;;
  -w|--watch-gem5)
    watch_all_gem5
    exit 0
    ;;
  -h|--help)
    print_help
    exit 0
    ;;
  *)
    echo "❌ Unknown option: $1"
    print_help
    exit 1
    ;;
esac
