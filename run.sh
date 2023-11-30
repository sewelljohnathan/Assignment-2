#!/bin/bash

make

sudo insmod charkmod-in.ko
sudo insmod charkmod-out.ko

sudo python3 test.py

sudo rmmod charkmod_out
sudo rmmod charkmod_in

make clean