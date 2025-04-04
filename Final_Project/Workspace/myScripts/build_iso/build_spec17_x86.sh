#!/bin/bash

## Run this script from `Workspace` folder
## Example: `cd Final_Project/Workspace`. Then run this script.

BASE=$(pwd)

## Place the `cpu2017-1.1.9.iso` file in `Final_Project/resources_gem5` folder
cd ../resources_gem5/

mkdir mountdir
mkdir SPEC17_workloads

sudo mount -o loop cpu2017-1.1.9.iso mountdir

cd mountdir

./install.sh -d ../SPEC17_workloads

cd ..

sudo umount mountdir

rm -rf mountdir


############################################

# we have taken system configuration specific .config file from `SPEC17_workloads/config` folder and modified it
# These .cfg file is modified and commented as '#MODIFIED'
cp $BASE/myScripts/build_iso/configs_cpu17/gcc-linux-x86_base.cfg SPEC17_workloads/config

cd SPEC17_workloads

source shrc

runcpu --config=gcc-linux-x86_base.cfg --action=build perlbench_s gcc_s mcf_s omnetpp_s xalancbmk_s x264_s deepsjeng_s leela_s exchange2_s xz_s bwaves_s cactuBSSN_s lbm_s wrf_s cam4_s pop2_s imagick_s nab_s fotonik3d_s roms_s -I
runcpu --config=gcc-linux-x86_base.cfg --action=run --fake perlbench_s gcc_s mcf_s omnetpp_s xalancbmk_s x264_s deepsjeng_s leela_s exchange2_s xz_s bwaves_s cactuBSSN_s lbm_s wrf_s cam4_s pop2_s imagick_s nab_s fotonik3d_s roms_s --iterations=1  -I

#runcpu --config=gcc-linux-x86_all.cfg --action=build all -I
#runcpu --config=gcc-linux-x86_all.cfg --action=run --fake all --noreportable --iterations=1  -I

cd ..


rm -rf "$BASE/SPEC17_workloads" && mv SPEC17_workloads "$BASE/"


cd $BASE