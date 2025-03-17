
# provide the path of both the trace file
TRACE_DIR1="../Given/traces/"  #path of the first trace file (excluding the trace file name)
TRACE_DIR2="../Given/traces/"  #path of the second trace file (excluding the trace file name)
binary="bin/bimodal-no-no-no-no-no-no-no-lru-lru-lru-lru-lru-lru-lru-lru-2core-no"
num1="1"
num2="2"


trace1=`sed -n ''$num1'p' workloads.txt | awk '{print $1}'`
trace2=`sed -n ''$num2'p' workloads.txt | awk '{print $1}'`

trace1_name="perl"
trace2_name="mcf"
# if [[ $trace1 == *"perlbench"* ]]; then
#     trace1_name="perl"
# else
#     trace1_name="mcf"
# fi

# if [[ $trace2 == *"perlbench"* ]]; then
#     trace2_name="perl"
# else
#     trace2_name="mcf"
# fi

RESULTS_FOLDER="./results/${2}"
mkdir -p "${RESULTS_FOLDER}" #change the name of your folder as required 
(${binary} -warmup_instructions 50000000 -simulation_instructions 50000000 -traces ${TRACE_DIR1}/${trace1} ${TRACE_DIR1}/${trace1}) > "${RESULTS_FOLDER}"/${trace1_name}-${trace1_name}.txt &
PID1=$!

(${binary} -warmup_instructions 50000000 -simulation_instructions 50000000 -traces ${TRACE_DIR2}/${trace2} ${TRACE_DIR1}/${trace2}) > "${RESULTS_FOLDER}"/${trace1_name}-${trace2_name}.txt &
PID2=$!

(${binary} -warmup_instructions 50000000 -simulation_instructions 50000000 -traces ${TRACE_DIR2}/${trace2} ${TRACE_DIR2}/${trace2}) > "${RESULTS_FOLDER}"/${trace2_name}-${trace2_name}.txt &
PID3=$!

wait $PID1
wait $PID2
wait $PID3
