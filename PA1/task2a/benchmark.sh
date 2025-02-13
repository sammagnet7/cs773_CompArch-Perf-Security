#!/bin/bash

# Initialize variables to calculate average time and difference
total_time=0
total_diff=0
run_count=$1

make clean && make;

# Run the sender and receiver programs run_count times
for i in $(seq 1 $run_count); do
    echo -ne "Run $i/$run_count\r"
    taskset -c 3 ./sender &
    { time taskset -c 5 ./receiver; } 2>> receiver_times.txt

    # Calculate the difference in number of characters between msg.txt and received.txt
    diff_chars=$(diff -y --suppress-common-lines msg.txt received.txt | wc -c)
    total_diff=$(echo "$total_diff + $diff_chars" | bc)
done

echo ""

# Calculate average time
while read -r line; do
    if [[ $line == real* ]]; then
        time_str=$(echo $line | awk '{print $2}')
        minutes=$(echo $time_str | cut -d'm' -f1)
        seconds=$(echo $time_str | cut -d'm' -f2 | cut -d's' -f1)
        total_time=$(echo "$total_time + ($minutes * 60) + $seconds" | bc)
    fi
done < receiver_times.txt

avg_time=$(echo "scale=2; $total_time / $run_count" | bc)

# Calculate the size of msg.txt
msg_size=$(stat -c%s "msg.txt")

# Calculate bandwidth (size/time)
bandwidth=$(echo "scale=2; $msg_size / $avg_time" | bc)

# Calculate average difference
avg_diff=$(echo "scale=2; $total_diff / $run_count" | bc)

# Calculate total characters in msg.txt
total_chars=$(wc -c < msg.txt)

# Calculate accuracy
accuracy=$(echo "scale=2; 100 - ($avg_diff / $total_chars * 100)" | bc)

echo "-----------------------------------------------------------------" >> reports.log
cat common.h| grep -e SLOT -e CHUNK -e BIT_REPEAT -e ROUNDS >> reports.log

# Print average time, accuracy, message size, and bandwidth
echo "Average time: $avg_time seconds" >> reports.log
echo "Accuracy: $accuracy%" >> reports.log
echo "Message size: $msg_size bytes" >> reports.log
echo "Bandwidth: $bandwidth bytes/second" >> reports.log

# Clean up temporary file
rm receiver_times.txt