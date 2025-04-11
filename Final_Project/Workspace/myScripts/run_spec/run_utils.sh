#!/bin/bash

kill_all_gem5() {
  echo "🔴 Killing all gem5.opt processes..."
  ps -ef | grep gem5.opt | grep -v grep | awk '{print $2}' | xargs -r kill -9
  echo "✅ Done."
}

watch_all_gem5() {
  echo "👀 Watching gem5.opt processes... (Press Ctrl+C to exit early)"
  while true; do
    clear
    # Get PIDs of gem5.opt directly
    pids=$(pgrep -f gem5.opt)

    if [ -z "$pids" ]; then
      echo "✅ No gem5.opt processes running."
      break
    fi

    echo "🟢 $(echo "$pids" | wc -l) gem5.opt process(es) running:"
    echo "$pids" | while read -r pid; do
      ps -p "$pid" -o pid=,args= | awk '{print $1, $10, $12}' \
        | sed -E 's/--outdir=//; s/\.\.\/+|\.\/+//g; s/_base\.spec-.*64//g'
    done

    sleep 2
  done
}


print_help() {
  echo "Usage: $0 [option]"
  echo ""
  echo "Options:"
  echo "  -k, --kill-gem5     Kill all running gem5.opt processes"
  echo "  -w, --watch-gem5    Display all currently running gem5.opt processes"
  echo "  -h, --help          Show this help message"
}

# Parse CLI arguments
while [[ $# -lt 2 ]]; do
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
done
