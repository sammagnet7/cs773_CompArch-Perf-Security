#!/bin/bash

make;
taskset -c 3 ./sender &
taskset -c 5 ./receiver

wait