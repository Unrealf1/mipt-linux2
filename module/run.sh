#!/bin/bash

make
sudo insmod my_module.ko
echo "-------------------------------------------------------------------"
sudo ./test
sudo rmmod my_module
