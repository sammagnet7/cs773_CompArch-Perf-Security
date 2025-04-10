#!/bin/bash

# run this from any where in the repo
# it assumes gem5 and spec is built and is in appropriate directories
# run_spec17.sh has been run at least once
# refer to myReadme.md for more details

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

rm -f "$slowdown_file"
rm -f "$slowdown_ipc_file"
rm -f "$ipc_file"
rm -f "$mpki_l1_file"
rm -f "$mpki_l2_file"

# system.cpu.dcache.overallMisses::total
# simSeconds
# ipc
# slowdown: ipc seconds
# mpki l1
# mpki l2

extract_data() {
  stats_file="$1"
  ipc=$(grep 'system.cpu.ipc' "$stats_file" | awk '{print $2}')
  ipc=${ipc:--1}
  ipc=$(echo "scale=3; $ipc*1.0" | bc -l)
  simSeconds=$(grep 'simSeconds' "$stats_file" | awk '{print $2}')
  l1_misses=$(grep 'system.cpu.dcache.overallMisses::total' "$stats_file" | awk '{print $2}')
  l2_misses=$(grep 'system.l2.overallMisses::total' "$stats_file" | awk '{print $2}')
  insts=$(grep 'simInsts' "$stats_file" | awk '{print $2}')

  if [[ -z "$simSeconds" || -z "$ipc" || "$ipc" == "-1" || -z "$insts" || "$insts" -eq 0 ]]; then
    echo "INVALID"
    return
  fi

  mpki_l1=$(echo "scale=3; $l1_misses * 1000 / $insts" | bc -l)
  mpki_l2=$(echo "scale=3; $l2_misses * 1000 / $insts" | bc -l)

  echo "$ipc $simSeconds $mpki_l1 $mpki_l2"
}

# Lists for geometric mean calculation
slowdowns=()
slowdown_ipcs=()
ipcs_base=()
ipcs_mirage=()
mpki_l1_base=()
mpki_l1_mirage=()
mpki_l2_base=()
mpki_l2_mirage=()

for bench in "${BENCHMARK_ITEMS[@]}"; do
  benchmark_dir=$(find "$SPEC17_DIR" -maxdepth 1 -type d -name '*'"$bench"'_s' 2>/dev/null)/run/run_base_refspeed_${label}-m64.0000
  base_stats="$benchmark_dir/$bench-base/stats.txt"
  mirage_stats="$benchmark_dir/$bench-mirage/stats.txt"

  if [[ ! -f "$base_stats" || ! -f "$mirage_stats" ]]; then
    echo "🚨 Stats file missing for benchmark $bench. Skipping..."
    continue
  fi

  base_data=$(extract_data "$base_stats")
  if [[ "$base_data" == "INVALID" ]]; then
    echo_red "❌ Invalid base stats for $bench. Skipping..."
    continue
  fi
  mirage_data=$(extract_data "$mirage_stats")
  if [[ "$mirage_data" == "INVALID" ]]; then
    echo_red "❌ Invalid mirage stats for $bench. Skipping..."
    continue
  fi

  read base_ipc base_simseconds base_mpki_l1 base_mpki_l2 <<<"$base_data"
  read mirage_ipc mirage_simseconds mirage_mpki_l1 mirage_mpki_l2 <<<"$mirage_data"

  slowdown=$(echo "scale=4; $base_simseconds / $mirage_simseconds" | bc -l)
  slowdown_ipc=$(echo "scale=4; $mirage_ipc / $base_ipc" | bc -l)

  slowdowns+=("$slowdown")
  slowdown_ipcs+=("$slowdown_ipc")
  ipcs_base+=("$base_ipc")
  ipcs_mirage+=("$mirage_ipc")
  mpki_l1_base+=("$base_mpki_l1")
  mpki_l1_mirage+=("$mirage_mpki_l1")
  mpki_l2_base+=("$base_mpki_l2")
  mpki_l2_mirage+=("$mirage_mpki_l2")

  echo "$bench $slowdown" >>"$slowdown_file"
  echo "$bench $slowdown_ipc" >>"$slowdown_ipc_file"
  echo "$bench $base_ipc $mirage_ipc" >>"$ipc_file"
  echo "$bench $base_mpki_l1 $mirage_mpki_l1" >>"$mpki_l1_file"
  echo "$bench $base_mpki_l2 $mirage_mpki_l2" >>"$mpki_l2_file"
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
echo "gmean $(geo_mean "${ipcs_base[@]}") $(geo_mean "${ipcs_mirage[@]}")" >>"$ipc_file"
echo "gmean $(geo_mean "${mpki_l1_base[@]}") $(geo_mean "${mpki_l1_mirage[@]}")" >>"$mpki_l1_file"
echo "gmean $(geo_mean "${mpki_l2_base[@]}") $(geo_mean "${mpki_l2_mirage[@]}")" >>"$mpki_l2_file"

commands=(
  "gnuplot -e \"input_file='data/slowdown.stat'\" -e \"output_png='plots/slowdown.png'\" -e  \"var_title='Slowdown (simSeconds)'\" -e \"var_xlabel='Benchmarks'\" -e \"var_ylabel='Slowdown'\" slowdown.gp"
  "gnuplot -e \"input_file='data/slowdown_ipc.stat'\" -e \"output_png='plots/slowdown_ipc.png'\" -e  \"var_title='Slowdown (IPC)'\" -e \"var_xlabel='Benchmarks'\" -e \"var_ylabel='Slowdown'\" slowdown.gp"
  "gnuplot -e \"input_file='data/ipc.stat'\" -e \"output_png='plots/ipc.png'\" -e  \"var_title='IPC'\" -e \"var_xlabel='Benchmarks'\" -e \"var_ylabel='IPC'\" double_bar.gp"
  "gnuplot -e \"input_file='data/mpki_l1.stat'\" -e \"output_png='plots/mpki_l1.png'\" -e  \"var_title='MPKI L1'\" -e \"var_xlabel='Benchmarks'\" -e \"var_ylabel='MPKI (L1)'\" double_bar.gp"
  "gnuplot -e \"input_file='data/mpki_l2.stat'\" -e \"output_png='plots/mpki_l2.png'\" -e  \"var_title='MPKI L2'\" -e \"var_xlabel='Benchmarks'\" -e \"var_ylabel='MPKI (L2)'\" double_bar.gp"
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
