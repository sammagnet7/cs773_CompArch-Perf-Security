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

i=0
#for bench in bwaves perlbench gcc mcf omnetpp xalancbmk deepsjeng leela exchange2 xz bwaves cactusBSSN lbm wrf cam4 pop2 imagick nab fotonik3d roms
for bench in omnetpp mcf  ## Use above line for all benchmarks

do
  ((i=i%P)); ((i++==0)) && wait
  (
  IN=$(grep $bench $SPEC_CONFIG/input_2017.txt | awk -F':' '{print $2}'| xargs)
  BIN=$(grep $bench $SPEC_CONFIG/binaries_2017.txt | awk -F':' '{print $2}' | xargs)
  ARGS=$(grep $bench $SPEC_CONFIG/args_2017.txt | awk -F':' '{print $2}'| xargs)

  BINA="../../exe/${BIN}_base.${label}-m64"

  cd "$(find . -maxdepth 1 -type d -name '*'"$bench"'_s' 2>/dev/null)/run/run_base_refspeed_${label}-m64.0000" || echo "Directory not found!"

  $GEM5_RUNNER/run_ghostminion.sh "$BINA" "$ARGS" "$IN" 
  ) & 
done
cd $BASE


