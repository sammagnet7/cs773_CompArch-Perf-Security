#!/bin/bash

# Validate required arguments and show help if missing
if [ "$#" -lt 3 ]; then
    echo "Usage: $0 <mode> <benchmark_name> <binary> [extra_args] [bin_args] [bin_input]"
    echo ""
    echo "Arguments:"
    echo "  mode                - Run mode (e.g., vanilla, base, mirage, ghost); used for --out-dir"
    echo "  benchmark_name      - Name used for logging."
    echo "  binary              - Path to the binary"
    echo "  extra_args          - (Optional) Extra gem5 arguments for the mode"
    echo "  bin_args            - (Optional) Command-line arguments for the binary"
    echo "  bin_input           - (Optional) Input file for the binary"
    exit 1
fi

MODE="$1"
BENCHMARK_NAME="$2"
BIN_EXE="$3"
MODE_ARGS="$4"
BIN_ARGS="$5"
BIN_IN="$6"
PWD="$(pwd)"

spinner() {
    local pid=$1
    local delay=0.1
    local spinstr='|/-\'
    # Hide the cursor
    tput civis
    while [ "$(ps a | awk '{print $1}' | grep $pid)" ]; do
        local temp=${spinstr#?}
        printf " \e[1;33m[%c]\e[0m  " "$spinstr" # Yellow color
        local spinstr=$temp${spinstr%"$temp"}
        sleep $delay
        printf "\b\b\b\b\b\b"
    done
    # Clear the spinner and restore cursor
    printf "    \b\b\b\b"
    tput cnorm
}

if [ -d "$MODE" ]; then
    echo_magenta "⚠️ $MODE directory already exists. Previous results will be overwritten."
fi

mkdir -p "$MODE"
eval "mode_args_array=($MODE_ARGS)"
cmd_args=(
    "$GEM5_BUILD_PATH/gem5.opt"
    --outdir="$MODE"
    "$GEM5_CONFIG_PATH/se.py"
    --cmd "$BIN_EXE"
    --option "$BIN_ARGS"
    -i "$BIN_IN"
    --num-cpus=1 --mem-size=8GB --mem-type=DDR4_2400_8x8
    --cpu-type DerivO3CPU
    --caches --l2cache
    --l1d_size=32kB --l1i_size=32kB --l2_size=8MB
    --l1d_assoc=8 --l1i_assoc=8 --l2_assoc=16
    --warmup-insts=$WARMUP_INSTS --maxinsts=$MAX_INSTS
    "${mode_args_array[@]}"
    --prog-interval=100Hz
)

echo_cyan "📈📈📈  Running $MODE Gem5 on $BENCHMARK_NAME 📈📈📈"
echo_cyan "-----------------------------------------------------------"
echo_yellow "🔧  Configurations given:"
echo_blue "MODE = \033[0m $MODE"
echo_blue "BENCHMARK_NAME = \033[0m $BENCHMARK_NAME"
echo_blue "BIN_EXE = \033[0m $BIN_EXE"
echo_blue "MODE_ARGS = \033[0m $MODE_ARGS"
echo_blue "BIN_ARGS = \033[0m $BIN_ARGS"
echo_blue "BIN_IN = \033[0m $BIN_IN"
echo_blue "PWD = \033[0m $PWD"
echo_blue "CMD = \033[0m "
printf '  %q' "${cmd_args[@]}"
printf ' > %q\n' "$MODE/run_base.log"
echo_cyan "-----------------------------------------------------------"

# Run the command with redirection
"${cmd_args[@]}" >"$MODE/run_base.log" 2>&1 &
gem5_pid=$!
# spinner $gem5_pid

wait $gem5_pid
exit_status=$?
if [ $exit_status -ne 0 ] || tail -n 1 "$PWD/$MODE/run_base.log" | grep -q "Simulated exit code not 0!"; then
    echo_red "\n❌  Failed $MODE Gem5 on $BENCHMARK_NAME ❌" >&2
    echo_red "Logs can be found in $PWD/$MODE/run_base.log" >&2
    echo_red "-----------------------------------------------------------" >&2
    echo "$BENCHMARK_NAME $MODE" >> "$BENCHMARK_FAILED_FILE"
else
    echo_green "\n✅✅✅  Completed $MODE Gem5 on $BENCHMARK_NAME ✅✅✅"
    echo_green "Logs saved in $PWD/$MODE/run_base.log"
    echo_green "Stats saved in $PWD/$MODE/stats.txt"
    echo_green "-----------------------------------------------------------"
    echo "$BENCHMARK_NAME $MODE" >> "$BENCHMARK_SUCCESS_FILE"
fi
