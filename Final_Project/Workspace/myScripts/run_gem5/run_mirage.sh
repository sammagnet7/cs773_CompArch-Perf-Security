#!/bin/bash
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <bench> <binary> <args> <input>"
    echo "Example: $0 benchmark_name binary_name 'arg1 arg2' input_file"
    exit 1
fi

bench="$1"
BINA="$2"
ARGS="$3"
IN="$4"

$GEM5_RUNNER/run_gem5.sh "mirage_base" "$bench" "$BINA" '--mirage_mode="Baseline"' "$ARGS" "$IN" &
$GEM5_RUNNER/run_gem5.sh "mirage_mirage" "$bench" "$BINA" '--mirage_mode="skew-vway-rand" --l2_numSkews=2 --l2_TDR=1.75 --l2_EncrLat=3' "$ARGS" "$IN"
