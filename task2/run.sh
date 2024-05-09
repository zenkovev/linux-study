#!/bin/bash

VROOT_PATH=/home/evgeniy/main/MIPT_new_sem/linux/linux-study/vroot make all
rm ../vroot/home/task2/ps2_keyboard.ko
cp ps2_keyboard.ko ../vroot/home/task2/