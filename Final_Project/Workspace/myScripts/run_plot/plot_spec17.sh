#!/bin/bash

# run this from any where in the repo
# it assumes gem5 and spec is built and is in appropriate directories
# run_spec17.sh has been run at least once
# refer to myReadme.md for more details

if [[ "$#" -lt 3 ]]; then
  echo "Usage: $0 <FLAVOUR> <BASE_DIR> <MODE_DIR> [--old-stats]"
  echo "  FLAVOUR: The gem flavour to be used in plots."
  echo "  BASE_DIR: The base directory name (not full path)."
  echo "  MODE_DIR: The mode directory name (not full path)."
  echo "  --old-stats: (Optional) flag to indicate using old stats."
  exit 1
fi

FLAVOUR="$1" # will be used in plots
BASE_DIR="$2" # not full path only the dir name
MODE_DIR="$3" # not full path only the dir name
OLD_STATS=false # default to false

if [[ "$4" == "--old-stats" ]]; then
  OLD_STATS=true
fi

script_path=$(realpath "$(dirname $0)")
vars_script_path="$script_path/../vars.sh"
if [[ ! -f "$vars_script_path" ]]; then
  echo "🚨 vars.sh script not found at $vars_script_path"
  exit 1
fi
source "$vars_script_path"

cd "$PLOTS_DIR"

mkdir -p "$PLOTS_DIR/data"
mkdir -p "$PLOTS_DIR/plots"
slowdown_file="$PLOTS_DIR/data/slowdown.stat"
slowdown_ipc_file="$PLOTS_DIR/data/slowdown_ipc.stat"
ipc_file="$PLOTS_DIR/data/ipc.stat"
mpki_l1_file="$PLOTS_DIR/data/mpki_l1.stat"
mpki_l2_file="$PLOTS_DIR/data/mpki_l2.stat"
latency_l1_file="$PLOTS_DIR/data/latency_l1.stat"
latency_l2_file="$PLOTS_DIR/data/latency_l2.stat"

rm -f "$slowdown_file"
rm -f "$slowdown_ipc_file"
rm -f "$ipc_file"
rm -f "$mpki_l1_file"
rm -f "$mpki_l2_file"
rm -f "$latency_l1_file"
rm -f "$latency_l2_file"

ATTR_IPC='system.cpu.ipc'
if [[ "$OLD_STATS" == true ]]; then
  # gem5 v20.0
  ATTR_SIM_SECONDS='sim_seconds'
  ATTR_L2MISS_TOTAL='system.l2.overall_misses::total'
  ATTR_L1MISS_TOTAL='system.cpu.dcache.overall_misses::total'
  ATTR_SIM_INSTS='sim_insts'
  AATR_L1MISS_LATENCY='system.cpu.dcache.overall_avg_miss_latency::total'
  AATR_L2MISS_LATENCY='system.l2.overall_avg_miss_latency::total'
else
  # Gem5 v24.0
  ATTR_SIM_SECONDS='simSeconds'
  ATTR_L2MISS_TOTAL='system.cpu.dcache.overallMisses::total'
  ATTR_L1MISS_TOTAL='system.l2.overallMisses::total'
  ATTR_SIM_INSTS='simInsts'
  AATR_L1MISS_LATENCY='system.cpu.dcache.overallAvgMissLatency::total'
  AATR_L2MISS_LATENCY='system.l2.overallAvgMissLatency::total'
fi


extract_data() {
  stats_file="$1"
  ipc=$(grep "$ATTR_IPC" "$stats_file" | awk '{print $2}')
  ipc=${ipc:--1}
  ipc=$(echo "scale=3; $ipc*1.0" | bc -l)
  simSeconds=$(grep "$ATTR_SIM_SECONDS" "$stats_file" | awk '{print $2}')
  l1_misses=$(grep "$ATTR_L1MISS_TOTAL" "$stats_file" | awk '{print $2}')
  l2_misses=$(grep "$ATTR_L2MISS_TOTAL" "$stats_file" | awk '{print $2}')
  l1_latency=$(grep "$AATR_L1MISS_LATENCY" "$stats_file" | awk '{print $2}')
  l2_latency=$(grep "$AATR_L2MISS_LATENCY" "$stats_file" | awk '{print $2}')
  insts=$(grep "$ATTR_SIM_INSTS" "$stats_file" | awk '{print $2}')

  if [[ -z "$simSeconds" || -z "$ipc" || "$ipc" == "-1" || -z "$insts" || "$insts" -eq 0 ]]; then
    echo "INVALID"
    return
  fi

  mpki_l1=$(echo "scale=3; $l1_misses * 1000 / $insts" | bc -l)
  mpki_l2=$(echo "scale=3; $l2_misses * 1000 / $insts" | bc -l)

  echo "$ipc $simSeconds $mpki_l1 $mpki_l2 $l1_latency $l2_latency"
}

# Lists for geometric mean calculation
slowdowns=()
slowdown_ipcs=()
ipcs_base=()
ipcs_modded=()
mpki_l1_base=()
mpki_l1_modded=()
mpki_l2_base=()
mpki_l2_modded=()
l1_latency_base=()
l1_latency_modded=()
l2_latency_base=()
l2_latency_modded=()
l1_latency_ratios=()
l2_latency_ratios=()

for bench in "${BENCHMARK_ITEMS[@]}"; do
  # benchmark_dir=$(find "$SPEC17_DIR" -maxdepth 1 -type d -name '*'"$bench"'_s' 2>/dev/null)/${SPEC_RUN_DIR_SUFFIX}
  benchmark_dir="$(pwd)/stats/${bench}"
  base_stats="$benchmark_dir/$BASE_DIR/stats.txt"
  modded_stats="$benchmark_dir/$MODE_DIR/stats.txt"

  if [[ ! -f "$base_stats" || ! -f "$modded_stats" ]]; then
    echo "🚨 Stats file missing for benchmark $bench. Skipping..."
    continue
  fi

  base_data=$(extract_data "$base_stats")
  if [[ "$base_data" == "INVALID" ]]; then
    echo_red "❌ Invalid base stats for $bench. Skipping..."
    continue
  fi
  modded_data=$(extract_data "$modded_stats")
  if [[ "$modded_data" == "INVALID" ]]; then
    echo_red "❌ Invalid modded stats for $bench. Skipping..."
    continue
  fi

  # echo $bench $base_data $modded_data

  read base_ipc base_simseconds base_mpki_l1 base_mpki_l2  base_l1_latency base_l2_latency<<<$(echo "$base_data" | tail -1)
  read modded_ipc modded_simseconds modded_mpki_l1 modded_mpki_l2 modded_l1_latency modded_l2_latency <<<$(echo "$modded_data" | tail -1)

  slowdown=$(echo "scale=4; $base_simseconds / $modded_simseconds" | bc -l)
  slowdown_ipc=$(echo "scale=4; $modded_ipc / $base_ipc" | bc -l)
  l1_latency_ratio=$(echo "scale=3; $modded_l1_latency/$base_l1_latency" | bc -l)
  l2_latency_ratio=$(echo "scale=3; $modded_l2_latency/$base_l2_latency" | bc -l)

  slowdowns+=("$slowdown")
  slowdown_ipcs+=("$slowdown_ipc")
  ipcs_base+=("$base_ipc")
  ipcs_modded+=("$modded_ipc")
  mpki_l1_base+=("$base_mpki_l1")
  mpki_l1_modded+=("$modded_mpki_l1")
  mpki_l2_base+=("$base_mpki_l2")
  mpki_l2_modded+=("$modded_mpki_l2")
  l1_latency_base+=("$base_l1_latency")
  l1_latency_modded+=("$modded_l1_latency")
  l2_latency_base+=("$base_l2_latency")
  l2_latency_modded+=("$modded_l2_latency")
  l1_latency_ratios+=("$l1_latency_ratio")
  l2_latency_ratios+=("$l2_latency_ratio")

  echo "$bench $slowdown" >>"$slowdown_file"
  echo "$bench $slowdown_ipc" >>"$slowdown_ipc_file"
  echo "$bench $base_ipc $modded_ipc" >>"$ipc_file"
  echo "$bench $base_mpki_l1 $modded_mpki_l1" >>"$mpki_l1_file"
  echo "$bench $base_mpki_l2 $modded_mpki_l2" >>"$mpki_l2_file"
  echo "$bench $base_l1_latency $modded_l1_latency" >>"$latency_l1_file"
  echo "$bench $base_l2_latency $modded_l2_latency" >>"$latency_l2_file"
done

geo_mean() {
  awk 'BEGIN {
    product_log_sum = 0;
    count = ARGC - 1;
    for (i = 1; i < ARGC; i++) {
      if (ARGV[i] <= 0) {
        print "0.0000"; exit;
      }
      product_log_sum += log(ARGV[i]);
    }
    printf "%.4f\n", exp(product_log_sum / count);
  }' "$@"
}
echo "gmean $(geo_mean "${slowdowns[@]}")" >>"$slowdown_file"
echo "gmean $(geo_mean "${slowdown_ipcs[@]}")" >>"$slowdown_ipc_file"
echo "gmean $(geo_mean "${ipcs_base[@]}") $(geo_mean "${ipcs_modded[@]}")" >>"$ipc_file"
echo "gmean $(geo_mean "${mpki_l1_base[@]}") $(geo_mean "${mpki_l1_modded[@]}")" >>"$mpki_l1_file"
echo "gmean $(geo_mean "${mpki_l2_base[@]}") $(geo_mean "${mpki_l2_modded[@]}")" >>"$mpki_l2_file"
echo "gmean $(geo_mean "${l1_latency_base[@]}") $(geo_mean "${l1_latency_modded[@]}")" >>"$latency_l1_file"
echo "gmean $(geo_mean "${l2_latency_base[@]}") $(geo_mean "${l2_latency_modded[@]}")" >>"$latency_l2_file"

commands=(
  "gnuplot -e \"gem_flavour='${FLAVOUR}'\" -e \"input_file='data/slowdown.stat'\" -e \"output_png='plots/slowdown.png'\" -e  \"var_title='Speedup compared to non-secure baseline [${FLAVOUR}]'\" -e \"var_xlabel='Benchmarks'\" -e \"var_ylabel='Speedup'\" slowdown.gp"
  "gnuplot -e \"gem_flavour='${FLAVOUR}'\" -e \"input_file='data/mpki_l1.stat'\" -e \"output_png='plots/mpki_l1.png'\" -e  \"var_title='MPKI L1 [${FLAVOUR}]'\" -e \"var_xlabel='Benchmarks'\" -e \"var_ylabel='MPKI (L1)'\" double_bar.gp"
  "gnuplot -e \"gem_flavour='${FLAVOUR}'\" -e \"input_file='data/mpki_l2.stat'\" -e \"output_png='plots/mpki_l2.png'\" -e  \"var_title='MPKI L2 [${FLAVOUR}]'\" -e \"var_xlabel='Benchmarks'\" -e \"var_ylabel='MPKI (L2)'\" double_bar.gp"
  "gnuplot -e \"gem_flavour='${FLAVOUR}'\" -e \"input_file='data/latency_l1.stat'\" -e \"output_png='plots/l1_latency.png'\" -e  \"var_title='L1 latency [${FLAVOUR}]'\" -e \"var_xlabel='Benchmarks'\" -e \"var_ylabel='Latency [x1k cycles] (L1)'\" double_bar.gp"
  "gnuplot -e \"gem_flavour='${FLAVOUR}'\" -e \"input_file='data/latency_l2.stat'\" -e \"output_png='plots/l2_latency.png'\" -e  \"var_title='L2 latency [${FLAVOUR}]'\" -e \"var_xlabel='Benchmarks'\" -e \"var_ylabel='Latency [x1k cycles] (L2)'\" double_bar.gp"
)

for cmd in "${commands[@]}"; do
  eval "$cmd"
  if [ $? -ne 0 ]; then
    output_file=$(echo "$cmd" | grep -oP "output_png='\K[^']+")
    echo_red "❌ Failed to generate plot: $output_file"
  else
    output_file=$(echo "$cmd" | grep -oP "output_png='\K[^']+")
    echo_green "✅ Successfully generated: $output_file"
  fi
done

echo_magenta "🌟🎨  GRAPH GENERATION COMPLETE!  🎨🌟"
echo_cyan "📊 Find the generated plots inside:"
echo_yellow "➡  $PLOTS_DIR"
