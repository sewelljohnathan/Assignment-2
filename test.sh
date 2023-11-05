#!/bin/bash

divider="==============================="

printf "\nMake and Install\n${divider}\n\n"
make 
sudo insmod lkmasg2.ko

printf "\nUnit Tests\n${divider}\n\n"
sudo ./unittest /dev/lkmasg2
printf "\n"

# Uninstall mod
sudo rmmod lkmasg2