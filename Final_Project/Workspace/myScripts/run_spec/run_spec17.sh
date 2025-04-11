#!/bin/bash

# run this from any where in the repo
# it assumes gem5 and spec is built and is in appropriate directories
# refer to myReadme.md for more details

script_path=$(realpath "$(dirname $0)")
vars_script_path="$script_path/../vars.sh"
if [[ ! -f "$vars_script_path" ]]; then
  echo "🚨 vars.sh script not found at $vars_script_path"
  exit 1
fi
source "$vars_script_path"

set -u
cd $SPEC17_DIR
# Track active jobs
active_jobs=0

echo -e "\n\033[1;34m🚀🚀🚀  STARTING BENCHMARK Suite 🚀🚀🚀\033[0m"
echo -e "\033[1;34m------------------------------------------------------------\033[0m"

for bench in ${BENCHMARK_ITEMS[*]}; do
  (
    IN=$(grep $bench $SPEC_CONFIG/input_2017.txt | awk -F':' '{print $2}' | xargs)
    BIN=$(grep $bench $SPEC_CONFIG/binaries_2017.txt | awk -F':' '{print $2}' | xargs)
    ARGS=$(grep $bench $SPEC_CONFIG/args_2017.txt | awk -F':' '{print $2}' | xargs)

    BINA="../../exe/${BIN}_base.${label}-64"

    cd "$(find ${SPEC17_DIR} -maxdepth 1 -type d -name '*'"$bench"'_s' 2>/dev/null)/${SPEC_RUN_DIR_SUFFIX}" || echo "Directory not found!"

    case "${GEM5_FLAVOUR}" in
    "vanilla")
      $GEM5_RUNNER/run_vanilla.sh "$bench" "$BINA" "$ARGS" "$IN" > /dev/null
      ;;
    "ghost")
      $GEM5_RUNNER/run_ghostminion.sh "$bench" "$BINA" "$ARGS" "$IN" > /dev/null
      ;;
    "mirage")
      $GEM5_RUNNER/run_mirage.sh "$bench" "$BINA" "$ARGS" "$IN" > /dev/null
      ;;
    *)
      echo "🚨 Unknown GEM5_FLAVOUR: ${GEM5_FLAVOUR}"
      exit 1
      ;;
    esac
  ) &
  ((active_jobs++))

  # Wait for at least one job to finish before launching more
  if ((active_jobs >= P)); then
    wait -n           # Wait for any one job to complete
    ((active_jobs--)) # Decrement job counter after one completes
  fi
done

# Wait for all remaining jobs before exiting
wait

# Final completion message with stats file location
echo_cyan "\n============================================================"
if [[ ! -s "$BENCHMARK_FAILED_FILE" ]]; then
  echo -e "🎉🎉🎉  ALL BENCHMARKS COMPLETED SUCCESSFULLY!  🎉🎉🎉"
else
  echo -e "🚨 Some benchmarks failed. Summary below:"
  echo_red "Failed Benchmarks: "
  cat "$BENCHMARK_FAILED_FILE"
fi
echo_cyan "============================================================"

echo -e "\n\033[1;36m📂 Find the stats files inside:\033[0m"
echo -e "\033[1;33m➡  $SPEC17_DIR: 'r' for freq and 's' for speed}/run/{*base*}.mm64.0000/m5out\033[0m\n"

case "${GEM5_FLAVOUR}" in
"vanilla")
  echo_magenta "No automated plots for vanilla Gem5 ;("
  ;;
"ghost")
  eval "$PLOTS_DIR/plot_spec17.sh" GhostMinion ghost_base ghost_ghost --old-stats
  ;;
"mirage")
  eval "$PLOTS_DIR/plot_spec17.sh" Mirage mirage_base mirage_mirage
  ;;
*)
  echo "🚨 Unknown GEM5_FLAVOUR: ${GEM5_FLAVOUR}"
  exit 1
  ;;
esac
