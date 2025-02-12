#!/bin/bash

make clean &> /dev/null && make &> /dev/null &&
taskset -c 3 ./sender &
taskset -c 5 ./receiver

wait