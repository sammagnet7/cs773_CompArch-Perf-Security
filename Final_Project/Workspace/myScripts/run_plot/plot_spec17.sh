## Run this script from `Workspace/`
export BASE=$(pwd)
export PLOTS_DIR=$BASE/myScripts/run_plot/plots
export SPEC17_DIR=$BASE/SPEC17_workloads/benchspec/CPU

label="spec" # label which is set in the .cfg file in $BASE/myScripts/build_iso/configs_cpu17 folder


rm $PLOTS_DIR/slowdown.data


for bench in bwaves perlbench gcc mcf omnetpp xalancbmk deepsjeng leela exchange2 xz bwaves cactusBSSN lbm wrf cam4 pop2 imagick nab fotonik3d roms
#for bench in mcf omnetpp ## Use above line for all benchmarks

do
  #echo $bench >> $PLOTS_DIR/slowdown.data

  base=$(grep simSe $SPEC17_DIR/*$(echo $bench)_s/run/run_base_refspeed_${label}-m64.0000/m5out/statsno.txt | awk '{print $2}')
  slow=$(grep simSe $SPEC17_DIR/*$(echo $bench)_s/run/run_base_refspeed_${label}-m64.0000/m5out/statsghostminion.txt  | awk '{print $2}')

  ## currently kept as dummy values. Please comment out this line to get the actual slowdown value
  slow=$(echo "scale=3; $base * 1.3" | bc -l)

  echo "$bench $(echo "scale=3; $slow / $base" | bc -l)" >> $PLOTS_DIR/slowdown.data


done

cd $PLOTS_DIR/..
gnuplot slowdown.gp


# Final message after graph generation
echo -e "\n\033[1;35m🌈🌟🎨  GRAPH GENERATION COMPLETE!  🎨🌟🌈\033[0m"
echo -e "\033[1;36m📊 Find the generated plots inside:\033[0m"
echo -e "\033[1;33m➡  $PLOTS_DIR\033[0m\n"

cd $BASE
