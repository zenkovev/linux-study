#!/bin/bash

VROOT_PATH=/home/evgeniy/main/MIPT_new_sem/linux/linux-study/vroot make all
rm ../vroot/home/task4/mmaneg.ko
cp mmaneg.ko ../vroot/home/task4/
rm ../vroot/home/task4/test.sh
cp test.sh ../vroot/home/task4/