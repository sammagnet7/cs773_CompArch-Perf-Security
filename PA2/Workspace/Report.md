# Programming Assignment 2 Report

## CS 773: Computer Architecture for Performance and Security, Spring 2025  
**Indian Institute of Technology, Bombay**  

### Team Members:  
- **Arif Ali** (23m0822)  
- **Soumik Dutta** (23m0826)  
- **Arnab Bhakta** (23m0835)  

---

## Git Repository
[Repository Link: **[PA2](https://github.com/sammagnet7/cs773_CompArch-Perf-Security/tree/main/PA2)**]

---

## Task 1A: Implementing Way Partitioning in Champsim

### Branch Name: `Way-Partitioning`

### Files Changed:
- inc/cache.h
- replacement/lru.llc_repl
- src/cache.cc

### Results:

#### Normalized IPC
![Way Partitioning - Normalized IPC](./results/graphs/way_part/IPC.png)

#### LLC MPKI Comparison
![Way Partitioning - LLC MPKI](./results/graphs/way_part/MPKI.png)

#### Self-Evictions per Kilo Instructions
![Way Partitioning - Self Evictions](./results/graphs/way_part/EVICTION.png)

---

## Task 2A: Static Set Partitioning (Rang-De-Basanti)

### Branch Name: `Static-Set-partitioning`

### Files Changed:
- inc/cache.h
- src/cache.cc
- src/page_table_walker.cc
- src/main.cc

### Results:

#### Normalized Speedup
![Static Set Partitioning - Normalized Speedup](./results/graphs/static_set_part/IPC.png)

#### LLC MPKI Comparison
![Static Set Partitioning - LLC MPKI](./results/graphs/static_set_part/MPKI.png)

#### Self-Evictions per Kilo Instructions
![Static Set Partitioning - Self Evictions](./results/graphs/static_set_part/EVICTION.png)

---

## Task 2B: Dynamic Set Partitioning (Tumse Na Ho Payega!)

### Branch Name: `Dynamic-set-partitioning`

### Files Changed:
- inc/cache.h
- src/cache.cc
- src/main.cc
- src/page_table_walker.cc

### Results:

#### Normalized Speedup
![Dynamic Set Partitioning - Normalized Speedup](./results/graphs/dynamic_set_part/IPC.png)

#### LLC MPKI Comparison
![Dynamic Set Partitioning - LLC MPKI](./results/graphs/dynamic_set_part/MPKI.png)

#### Self-Evictions per Kilo Instructions
![Dynamic Set Partitioning - Self Evictions](./results/graphs/dynamic_set_part/EVICTION.png)

---

## Conclusion


