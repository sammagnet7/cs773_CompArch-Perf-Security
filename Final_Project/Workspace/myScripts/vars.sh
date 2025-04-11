#!/bin/bash

# This script defines two variables with default values that can be overridden by command-line arguments:
# - max_insts_arg: Represents the maximum number of instructions to process. Defaults to 10,000,000 if not provided as the first argument.
# - warmup_insts_arg: Represents the number of instructions to use for warming up. Defaults to 10,000,000 if not provided as the second argument.
# Usage:
#   ./vars.sh [max_insts] [warmup_insts]
# Example:
#   ./vars.sh 5000000 2000000
max_insts_arg=${1:-10000000}
warmup_insts_arg=${2:-10000000}

echo_green() { echo -e "\033[1;32m$1\033[0m"; }
echo_red() { echo -e "\033[1;31m$1\033[0m"; }
echo_blue() { echo -e "\033[1;34m$1\033[0m"; }
echo_yellow() { echo -e "\033[1;33m$1\033[0m"; }
echo_cyan() { echo -e "\033[1;36m$1\033[0m"; }
echo_magenta() { echo -e "\033[1;35m$1\033[0m"; }
echo_white() { echo -e "\033[1;37m$1\033[0m"; }

export -f echo_green
export -f echo_red
export -f echo_blue
export -f echo_yellow
export -f echo_cyan
export -f echo_magenta
export -f echo_white

SCRIPT2_PATH="$(realpath "${BASH_SOURCE[0]}")"
SCRIPT2_DIR="$(dirname "$SCRIPT2_PATH")"
export BASE="$(realpath "$SCRIPT2_DIR/../")"
export label="spec" # label which is set in the .cfg file in $BASE/myScripts/build_iso/configs_cpu17 folder
export SPEC17_DIR=$BASE/SPEC17_workloads/benchspec/CPU
export SPEC_CONFIG=$BASE/myScripts/run_spec/spec_configs
export SPEC_RUN_DIR_SUFFIX="run/run_base_refspeed_${label}-64.0000"
export GEM5_RUNNER=$BASE/myScripts/run_gem5
export GEM5_SRC_DIR="gem5_ghostminion-v20.0" # onnly dir name not full path
export GEM5_ARCH="ARM" # | "X86"
export GEM5_FLAVOUR="ghost" # vanilla | ghost | mirage
export GEM5_BUILD_PATH=$BASE/${GEM5_SRC_DIR}/build/${GEM5_ARCH}
export GEM5_CONFIG_PATH=$BASE/${GEM5_SRC_DIR}/configs
export PLOTS_DIR=$BASE/myScripts/run_plot
export MAX_INSTS=$max_insts_arg
export WARMUP_INSTS=$warmup_insts_arg

export BENCHMARK_SUCCESS_FILE="/tmp/success.txt"
export BENCHMARK_FAILED_FILE="/tmp/failed.txt"

# Check if directories exist
all_dirs_exist=true
for dir in "$BASE" "$SPEC17_DIR" "$SPEC_CONFIG" "$GEM5_RUNNER" "$GEM5_BUILD_PATH" "$GEM5_CONFIG_PATH"; do
    if [ ! -d "$dir" ]; then
        echo_red "🗃️  $dir does not exist."
        all_dirs_exist=false
    fi
done
if [ "$all_dirs_exist" = false ]; then
    echo_red "🚨 Fatal error encountered. Exiting script."
    exit 1
fi

# Calculate resource availability
N=$(expr $(grep 'processor' /proc/cpuinfo | wc -l) - 8)
M=$(grep MemTotal /proc/meminfo | awk '{print $2}')
G=$(expr $M / 8192000)
P=$((G < N ? G : N))

export BENCHMARK_ITEMS=(perlbench gcc mcf omnetpp xalancbmk x264 deepsjeng leela exchange2 xz bwaves cactuBSSN lbm imagick nab fotonik3d roms)
# export BENCHMARK_ITEMS=(gcc)

echo_yellow "------------------------"
echo_yellow "🗃️  Directories Found:"
echo_blue "BASE                      = \033[0m $BASE"
echo_blue "SPEC17_DIR                = \033[0m $SPEC17_DIR"
echo_blue "SPEC_CONFIG               = \033[0m $SPEC_CONFIG"
echo_blue "GEM5_RUNNER               = \033[0m $GEM5_RUNNER"
echo_blue "GEM5_BUILD_PATH           = \033[0m $GEM5_BUILD_PATH"
echo_blue "GEM5_CONFIG_PATH          = \033[0m $GEM5_CONFIG_PATH"
echo_blue "PLOTS_DIR                 = \033[0m $PLOTS_DIR"
echo_blue "MAX_INSTS                 = \033[0m $MAX_INSTS"
echo_blue "WARMUP_INSTS              = \033[0m $WARMUP_INSTS"
echo_blue "BENCHMARK_ITEMS           = \033[0m ${BENCHMARK_ITEMS[*]}"
# echo_blue "CKPT_PATH          = $CKPT_PATH"
# echo_green "SESSION            = $SESSION"
# echo_green "RESULTS_ROOT_DIR   = $RESULTS_ROOT_DIR"
# echo_green "RESULTS_SESSION_DIR= $RESULTS_SESSION_DIR"
echo_yellow "------------------------"
echo_blue "🧵  Using $P logical cores."
echo_yellow "------------------------"
