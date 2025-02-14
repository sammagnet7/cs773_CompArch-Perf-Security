#!/bin/zsh
sudo systemctl stop gdm
sudo systemctl stop containerd
sudo systemctl stop snapd
sudo systemctl stop snapd.socket
