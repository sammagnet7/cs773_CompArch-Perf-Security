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
cp $BASE/myScripts/build_iso/configs_cpu17/gcc-linux-x86_all.cfg SPEC17_workloads/config

cd SPEC17_workloads

source shrc

runcpu --config=gcc-linux-x86_all.cfg --action=build mcf_s -I
runcpu --config=gcc-linux-x86_all.cfg --action=run --fake mcf_s --noreportable --iterations=1  -I

cd ..


rm -rf "$BASE/SPEC17_workloads" && mv SPEC17_workloads "$BASE/"


cd $BASE