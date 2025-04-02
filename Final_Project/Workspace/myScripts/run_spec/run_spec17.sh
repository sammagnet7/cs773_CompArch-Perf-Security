## Run this script from `Workspace/`
export BASE=$(pwd)
export SPEC17_DIR=$BASE/SPEC17_workloads/benchspec/CPU
export SPEC_CONFIG=$BASE/myScripts/run_spec/spec_configs
export GEM5_RUNNER=$BASE/myScripts/run_gem5

set -u
cd $SPEC17_DIR

# Calculate resource availability
N=$(grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}')
M=$(grep MemTotal /proc/meminfo | awk '{print $2}')
G=$(expr $M / 8192000)
P=$((G<N ? G : N))

i=0
for bench in bwaves perlbench gcc mcf omnetpp xalancbmk deepsjeng leela exchange2 xz bwaves cactusBSSN lbm wrf cam4 pop2 imagick nab fotonik3d roms
do
  ((i=i%P)); ((i++==0)) && wait
  (
  IN=$(grep $bench $SPEC_CONFIG/input_2017.txt | awk -F':' '{print $2}'| xargs)
  BIN=$(grep $bench $SPEC_CONFIG/binaries_2017.txt | awk -F':' '{print $2}' | xargs)
  ARGS=$(grep $bench $SPEC_CONFIG/args_2017.txt | awk -F':' '{print $2}'| xargs)

  BINA=./$(echo $BIN)"_base.mytest-64"
  echo $BINA

  cd *$(echo $bench)_s/run/run_base_refspeed_mytest-64.0000

  $GEM5_RUNNER/run_ghostminion.sh "$BINA" "$ARGS" "$IN" 
  ) & 
done
cd $BASE


