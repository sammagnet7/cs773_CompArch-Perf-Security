#!/bin/bash

## Run this script from `Workspace/`

export BASE=$(pwd)
export SPEC17_DIR=$BASE/SPEC17_workloads/benchspec/CPU
export SPEC_CONFIG=$BASE/myScripts/run_spec/spec_configs
export GEM5_RUNNER=$BASE/myScripts/run_gem5

label="spec" # label which is set in the .cfg file in $BASE/myScripts/build_iso/configs_cpu17 folder

set -u
cd $SPEC17_DIR

# Calculate resource availability
N=$(grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}')
M=$(grep MemTotal /proc/meminfo | awk '{print $2}')
G=$(expr $M / 8192000)
P=$((G<N ? G : N))

# Track active jobs
active_jobs=0

for bench in perlbench gcc mcf omnetpp xalancbmk x264 deepsjeng leela exchange2 xz bwaves cactuBSSN lbm wrf cam4 pop2 imagick nab fotonik3d roms

#for bench in omnetpp mcf  ## Use above line for all benchmarks

do

  (
  IN=$(grep $bench $SPEC_CONFIG/input_2017.txt | awk -F':' '{print $2}'| xargs)
  BIN=$(grep $bench $SPEC_CONFIG/binaries_2017.txt | awk -F':' '{print $2}' | xargs)
  ARGS=$(grep $bench $SPEC_CONFIG/args_2017.txt | awk -F':' '{print $2}'| xargs)

  BINA="../../exe/${BIN}_base.${label}-m64"

  cd "$(find . -maxdepth 1 -type d -name '*'"$bench"'_s' 2>/dev/null)/run/run_base_refspeed_${label}-m64.0000" || echo "Directory not found!"



  # Noticeable start message
  echo -e "\n\033[1;34m🚀🚀🚀  STARTING BENCHMARK: $bench  🚀🚀🚀\033[0m"
  echo -e "\033[1;34m------------------------------------------------------------\033[0m"




  $GEM5_RUNNER/run_ghostminion.sh "$BINA" "$ARGS" "$IN" 


  # After the benchmark completes
  echo -e "\n\033[1;32m✅✅✅  COMPLETED BENCHMARK: $bench  ✅✅✅\033[0m"
  echo -e "\033[1;32m------------------------------------------------------------\033[0m"


  ) & 
 ((active_jobs++)) 

  # Wait for at least one job to finish before launching more
  if (( active_jobs >= P )); then
    wait -n  # Wait for any one job to complete
    ((active_jobs--))  # Decrement job counter after one completes
  fi
done

# Wait for all remaining jobs before exiting
wait


# Final completion message with stats file location
echo -e "\n\033[1;32m============================================================"
echo -e "✅✅✅  ALL BENCHMARKS COMPLETED SUCCESSFULLY!  ✅✅✅"
echo -e "============================================================\033[0m"

echo -e "\n\033[1;36m📂 Find the stats files inside:\033[0m"
echo -e "\033[1;33m➡  $BASE/SPEC17_workloads/benchspec/CPU/{benchmark_folder: 'r' for freq and 's' for speed}/run/{*base*}.mm64.0000/m5out\033[0m\n"



cd $BASE