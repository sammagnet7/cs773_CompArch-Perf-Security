#!/bin/bash

make;
taskset -c 3 ./sender &
taskset -c 5 ./receiver;
wait

code --diff msg.txt received.txt
# code --diff temp/bin_correct temp/bin_received