# CS773 Architecture Security and Performance: Final Course Project

## Title: Hybrid Cache Architecture for Comprehensive Security

### Team Members:
- Soumik Dutta (23m0826)
- Sm Arif Ali (23m0822)
- Arnab Bhakta (23m0835)

> **NOTE:**  
> - [MIRAGE Paper](https://www.usenix.org/system/files/sec21-saileshwar.pdf)  
> - [GhostMinion Paper](https://dl.acm.org/doi/pdf/10.1145/3466752.3480074)  
> - [MIRAGE Repo](https://github.com/gururaj-s/mirage/tree/master)  
> - [GhostMinion Repo](https://github.com/SamAinsworth/reproduce-ghostminion-paper)  
> - [Gem5 Version We Used](https://github.com/gem5/gem5/tree/v24.1.0.2)  
> - [Gem5 Version MIRAGE Used](https://github.com/gem5/gem5/tree/v19.0)  
> - [Our Project Repo](https://github.com/sammagnet7/cs773_CompArch-Perf-Security/tree/main/Final_Project)  

## Goal of the Project
- Design a hybrid cache architecture that:
  1. Combines MIRAGE's global eviction to prevent conﬂict-based attacks.
  2. Integrates GhostMinion's strictness-order to prevent transient execution attacks.
  3. Evaluates the performance overhead of these combined techniques.
- **Metrics of interest:** IPC, MPKI

## Plan for Checkpoint-I
1. Set up the Gem5 simulator and run the SpecCPU benchmark.
2. Adapt the GhostMinion codebase (originally for ARM architecture) to x86 architecture.
3. Evaluate the performance overhead of GhostMinion individually.
4. Adapt the MIRAGE codebase (originally written for Gem5 v19.0) to the latest Gem5 version.
5. Evaluate the performance overhead of MIRAGE individually.

## Plan for Checkpoint-II
1. Integrate MIRAGE and GhostMinion into a single codebase.
2. Implement possible optimizations.
3. Evaluate the performance overhead of the combined techniques.


## Configurations
- Ubuntu 24.10 :  we used. It can be lesser ubuntu version as well
- *{BASE}* : **Workspace/**
- *{BASE}/../resources_gem5* : Contains **cpu2017-*.iso**
- *{BASE}/SPEC17_workloads*: Contains built and installed benchmark suits.
- *{BASE}/myScripts* : Contains all the custom wrirtten scripts.
- *{BASE}/mygem5* : Contains modified gem5 code as part of various techniques' implementation.
- 

## Steps to follow
1. Git clone: 
```bash
git clone https://github.com/sammagnet7/cs773_CompArch-Perf-Security
```
2. Copy paste the **cpu2017-*.iso** file into folder *Final_Project/resources_gem5*. Then go to **{BASE}**
3. 
```bash
cd Final_Project/Workspace 

export BASE=$(pwd)
```
3. To install software package dependencies, run from **{BASE}**
```bash
./myScripts/dependencies.sh
```
1. To build gem5 with x86 architecture, from **{BASE}*:
```bash
cd mygem5

N=$(grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}')

scons build/X86/gem5.opt -j${N}

cd $BASE
```
5. For installing SpecCpu benchmarks: 
   - Follow the instructions mentioned in the script `myScripts/build_iso/build_spec17_x86.sh`
  
6. To run all the benchmarks and get and comparative stats: 
```bash
cd $BASE

./myScripts/run_spec/run_spec17.sh

```
7. To plot graphs:
```bash
cd $BASE

./myScripts/run_plot/plot_spec17.sh
```

---