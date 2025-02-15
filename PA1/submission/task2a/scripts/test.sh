#!/bin/bash

make clean && make;
taskset -c 3 ./sender & taskset -c 5 ./receiver;
wait

# code --diff sentbits.log receivebits.log &> /dev/null
# code --diff msg.txt received.txt &> /dev/null
# code --diff temp/bin_correct temp/bin_received &> /dev/null