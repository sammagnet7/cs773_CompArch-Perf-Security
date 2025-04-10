## Build_iso config
```Make
default=base:     # flags for all base
   OPTIMIZE       = -g -O3 -static -fgnu89-inline           #MODIFIED
# O3 aggresive optimization; -static linked; -fgnu89-inline GNU89 semantic compatibility with older c code;
intrate,intspeed=base: # flags for integer base             #MODIFIED 
# for integer benchmarks; why empty?;

%if %{bits} == 32                                           #MODIFIED
    EXTRA_COPTIMIZE = -fgnu89-inline -fno-strict-aliasing   #MODIFIED
# fno-strict-aliasing: dont do optimizations assumes no aliasing;
%else                                
    EXTRA_COPTIMIZE = -fno-strict-aliasing                  #MODIFIED
    LDCFLAGS        = -z muldefs                            #MODIFIED
# -z muldefs: Linker flag that allows multiple symbol definitions.
%endif
```

## Gem5 config file
### Final_Project/Workspace/mygem5/configs/../se.py
#### added in mirage (129+)
```python
Options.addMirageOptions(parser)
```
### Final_Project/Workspace/mygem5/configs/common/Options.py
#### conflict (154)
```python
parser.add_argument(
    "--mem-size",
    action="store",
    type=str,
    default="512MiB", # ghostminion
    default="16GiB", # mirage
    help="Specify the physical memory size (single memory)",
)
```
#### added in ghostminion (199+)
```python
parser.add_argument("--ghostminion", action="store_true",default=False)
parser.add_argument("--cache_coher", action="store_true",default=False)
parser.add_argument("--iminion", action="store_true",default=False)
parser.add_argument("--block_contention", action="store_true",default=False)
parser.add_argument("--prefetch_ordered", action="store_true",default=False)
parser.add_argument("--ghost_size", type=str, default="2KiB") # MODIFIED type
parser.add_argument("--ghost_assoc", type=int, default="2") # MODIFIED type
```
#### added in mirage  (at the end)
```python
def addMirageOptions(parser):
    parser.add_argument("--mirage_mode", action="store", type=str, default="Baseline",
                      help="[Baseline,BaselineRRIP,scatter-cache,skew-vway-rand]")
    parser.add_argument("--l2_numSkews", type=int, default=1)
    parser.add_argument("--l2_TDR", type=float, default=1.5)
    parser.add_argument("--l2_EncrLat", type=int, default=3)

def addMultiprogramOptions(parser):
    parser.add_argument("--rate-mode", action="store_true", help="Enable rate-mode execution of single WL")
    parser.add_argument("--mix-mode", action="store_true", help="Enable mix-mode execution of multiple WL")
```