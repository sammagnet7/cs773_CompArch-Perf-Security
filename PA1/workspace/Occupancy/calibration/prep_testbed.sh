#!/bin/zsh
sudo systemctl stop gdm
sudo systemctl stop containerd
sudo systemctl stop snapd
sudo systemctl stop snapd.socket
# disable HW prefetchers
sudo wrmsr -a 0x1a4 47
# turn off HT
echo off | sudo tee /sys/devices/system/cpu/smt/control