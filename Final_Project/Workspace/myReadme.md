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
- Workspace/ : BaseDir
- {BaseDir}/../resources_gem5 : Put the `cpu2017-*.iso`
- {BaseDir}/SPEC17_workloads: Contains the built and installed benchmark suits.
- {BaseDir}/myScripts : Contains all the custom wrirtten scripts.
- {BaseDir}/mygem5 : Contains modified gem5 code as part of various techniques' implementation.

## Steps to follow
1. For installing SpecCpu benchmarks:
   - Follow the instructions mentioned in the script `{BaseDir}/myScripts/build_spec17_x86.sh`
2. 