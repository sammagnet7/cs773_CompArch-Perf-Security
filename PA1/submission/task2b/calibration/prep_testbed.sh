#!/bin/zsh
sudo systemctl stop gdm
sudo systemctl stop containerd
sudo systemctl stop snapd
sudo systemctl stop snapd.socket
sudo wrmsr -a 0x1a4 47
