#!/bin/bash

if [ "$#" -lt 2 ]; then
    echo "Correct arguments: bench args (stdin)" && exit 1
fi

export GEM5_BUILD_PATH=$BASE/mygem5/build/X86
export GEM5_CONFIG_PATH=$BASE/mygem5/configs/deprecated/example

BINA=$1
ARGS=$2
IN=$3


$GEM5_BUILD_PATH/gem5.opt $GEM5_CONFIG_PATH/se.py  -c="${BINA}" -o="${ARGS}" -i="${IN}" --maxinsts=1000000; mv m5out/stats.txt m5out/statsno.txt
$GEM5_BUILD_PATH/gem5.opt $GEM5_CONFIG_PATH/se.py  -c="${BINA}" -o="${ARGS}" -i="${IN}" --maxinsts=1000000; mv m5out/stats.txt m5out/statsghostminion.txt


#$GEM5_BUILD_PATH/gem5.opt $GEM5_CONFIG_PATH/se.py  -c="${BINA}" -o="${ARGS}" -i="${IN}" --caches --l2cache --cpu-type=DerivO3CPU   --maxinsts=1000000000 --fast-forward=1000000000  --mem-size=4096MB; mv m5out/stats.txt m5out/statsno.txt
#$GEM5_BUILD_PATH/gem5.opt $GEM5_CONFIG_PATH/se.py  -c="${BINA}" -o="${ARGS}" -i="${IN}" --caches --l2cache --cpu-type=DerivO3CPU   --maxinsts=1000000000 --fast-forward=1000000000 --mem-size=4096MB --ghostminion --cache_coher --ghost_size="2kB" --iminion --prefetch_ordered ; mv m5out/stats.txt m5out/statsghostminion.txt

#$GEM5_BUILD_PATH/gem5.opt $GEM5_CONFIG_PATH/se.py  -c="${BINA}" -o="${ARGS}" -i="${IN}" --caches --l2cache --cpu-type=DerivO3CPU   --maxinsts=1000000000 --fast-forward=1000000000  --mem-size=4096MB --ghostminion --cache_coher --ghost_size="2kB" --iminion --prefetch_ordered --blockContention; mv m5out/stats.txt m5out/statsghostminion+pipeline.txt
#$GEM5_BUILD_PATH/gem5.opt $GEM5_CONFIG_PATH/se.py  -c="${BINA}" -o="${ARGS}" -i="${IN}" --caches --l2cache --cpu-type=DerivO3CPU   --maxinsts=1000000000 --fast-forward=1000000000  --mem-size=2048MB --ghostminion; mv m5out/stats.txt m5out/statsbaseminion2k.txt
#$GEM5_BUILD_PATH/gem5.opt $GEM5_CONFIG_PATH/se.py  -c="${BINA}" -o="${ARGS}" -i="${IN}" --caches --l2cache --cpu-type=DerivO3CPU   --maxinsts=1000000000 --fast-forward=1000000000  --mem-size=2048MB --ghostminion --cache_coher; mv m5out/stats.txt m5out/statsbaseminion2kcoher.txt
#$GEM5_BUILD_PATH/gem5.opt $GEM5_CONFIG_PATH/se.py  -c="${BINA}" -o="${ARGS}" -i="${IN}" --caches --l2cache --cpu-type=DerivO3CPU   --maxinsts=1000000000 --fast-forward=1000000000  --mem-size=2048MB --iminion; mv m5out/stats.txt m5out/statsbaseminion2kionly.txt
#$GEM5_BUILD_PATH/gem5.opt $GEM5_CONFIG_PATH/se.py  -c="${BINA}" -o="${ARGS}" -i="${IN}" --caches --l2cache --cpu-type=DerivO3CPU   --maxinsts=1000000000 --fast-forward=1000000000  --mem-size=2048MB --ghostminion --prefetch_ordered ; mv m5out/stats.txt m5out/statsbaseminion2kpf.txt
#$GEM5_BUILD_PATH/gem5.opt $GEM5_CONFIG_PATH/se.py  -c="${BINA}" -o="${ARGS}" -i="${IN}" --caches --l2cache --cpu-type=DerivO3CPU   --maxinsts=1000000000 --fast-forward=1000000000  --mem-size=2048MB --ghostminion --cache_coher --iminion --ghost_assoc=4 --prefetch_ordered ; mv m5out/stats.txt m5out/statsminion2k4assocall.txt