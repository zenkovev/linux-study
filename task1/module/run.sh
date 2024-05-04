#!/bin/bash

VROOT_PATH=/home/evgeniy/main/MIPT_new_sem/linux/linux-study/vroot make all
rm ../../vroot/home/task1/module/phone_book.ko
cp phone_book.ko ../../vroot/home/task1/module/
rm ../../vroot/home/task1/module/test.sh
cp test.sh ../../vroot/home/task1/module/