#!/bin/bash

sudo dmesg --clear
make

sudo insmod charkmod-in.ko
sudo insmod charkmod-out.ko

sudo python3 test.py

sudo rmmod charkmod-out
sudo rmmod charkmod-in

make clean