#!/bin/bash

# Initialize an associative array to store line frequencies
declare -A line_freq

# Run the sender and receiver programs 100 times
for i in {1..100}; do
    echo -ne "Run $i/100\r"
    taskset -c 3 ./sender &
    taskset -c 5 ./receiver > temp_output.txt
    while read -r line; do
        ((line_freq["$line"]++))
    done < temp_output.txt
done

echo ""
# Print the frequency of each type of line
echo "Line frequencies:"
for line in "${!line_freq[@]}"; do
    echo "$line: ${line_freq[$line]}"
done

# Clean up temporary file
rm temp_output.txt