#!/bin/bash

# Number of iterations
n=3  # Change this to the required number of runs
make clean
make
# Command-line arguments for Program A (modify as needed)
args=("0" "0.12" "0.35" "0.6" "1.0")  # Ensure n matches the number of arguments

# Paths to the programs
progA="taskset -c 4 ./thrash_cal"  # Replace with actual path
progB="taskset -c 2 ./occ_cal"  # Replace with actual path

# Directory to store outputs
output_dir="output_logs"
mkdir -p "$output_dir"
# Array to store all Program B outputs
declare -a range_outputs

for ((i=1; i<n; i++)); do
    echo "Starting iteration $((i+1))..."
    
    # Run Program A with a different argument and send it to the background
    sleep 5
    $progA "${args[i]}" & 
    pidA=$!
    sleep 5


    outputB=$($progB)  # Capture stdout
    range_outputs+=("$outputB")  # Store in array

    # Once Program B exits, terminate Program A
    echo "Program B exited, terminating Program A..."
    kill $pidA 2>/dev/null

    # Ensure Program A is fully stopped
    wait $pidA 2>/dev/null

    echo "Iteration $((i+1)) complete."
done

echo "All iterations finished."

echo "Processing all collected outputs from Program B..."

 echo "${range_outputs[@]}"

# taskset -c 2 ./receiver ${range_outputs[@]}
