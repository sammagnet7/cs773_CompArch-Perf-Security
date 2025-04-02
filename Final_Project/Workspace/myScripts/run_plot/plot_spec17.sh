## Run this script from `Workspace/`
export BASE=$(pwd)
export PLOTS_DIR=$BASE/myScripts/run_plot/plots
export SPEC17_DIR=$BASE/SPEC17_workloads/benchspec/CPU

rm $PLOTS_DIR/slowdown.data


for bench in bwaves perlbench gcc mcf omnetpp xalancbmk deepsjeng leela exchange2 xz bwaves cactusBSSN lbm wrf cam4 pop2 imagick nab fotonik3d roms

do
  echo $bench >> $PLOTS_DIR/slowdown.data

  base=$(grep sim_se $SPEC17_DIR/*$(echo $bench)_s/run/run_base_refspeed_mytest-64.0000/m5out/statsno.txt | awk '{print $2}')
  slow=$(grep sim_se $SPEC17_DIR/*$(echo $bench)_s/run/run_base_refspeed_mytest-64.0000/m5out/statsghostminion.txt  | awk '{print $2}')

  echo $n $(echo "scale=3;$slow / $base" | bc -l) >> $PLOTS_DIR/slowdown.data
  echo "" >> $PLOTS_DIR/slowdown.data
done

cd $PLOTS_DIR
gnuplot slowdown.gp
mv slowdown.pdf spec2017.pdf

cd $BASE
