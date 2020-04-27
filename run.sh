#!/bin/bash

make
sudo rmmod my_module
sudo insmod my_module.ko
echo "-------------------------------------------------------------------"
sudo ./test
