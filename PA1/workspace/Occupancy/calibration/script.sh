#!/bin/bash

# Number of iterations
n=5  # Change this to the required number of runs
make clean
make
# Command-line arguments for Program A (modify as needed)
args=("0" "0.15" "0.4" "0.6" "1.0")  # Ensure n matches the number of arguments

# Paths to the programs
progA="taskset -c 4 ./thrash_cal"  # Replace with actual path
progB="taskset -c 2 ./occ_cal"  # Replace with actual path

# Array to store all Program B outputs
declare -a range_outputs
rm output.txt
touch output.txt
for ((i=0; i<n; i++)); do
    echo "Starting iteration $((i+1))..."
    
    # Run Program A with a different argument and send it to the background
    sleep 3
    $progA "${args[i]}" & 
    pidA=$!
    sleep 3


    outputB=$($progB)  # Capture stdout
    range_outputs+=("$outputB")  # Store in array

    # Once Program B exits, terminate Program A
    kill $pidA 2>/dev/null

    # Ensure Program A is fully stopped
    wait $pidA 2>/dev/null

    echo "Iteration $((i+1)) complete."
done

echo "All iterations finished."

echo "Processing all collected outputs from Program B..."

echo "${range_outputs[@]}"

# taskset -c 2 ./receiver ${range_outputs[@]}
