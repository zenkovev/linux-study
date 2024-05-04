#!/bin/bash

VROOT_PATH=/home/evgeniy/main/MIPT_new_sem/linux/linux-study/vroot make all
gcc check.c -o check

rm ../../vroot/home/task1/syscall/phone_book.ko
cp phone_book.ko ../../vroot/home/task1/syscall/
rm ../../vroot/home/task1/syscall/check
cp check ../../vroot/home/task1/syscall/
rm ../../vroot/home/task1/syscall/test.sh
cp test.sh ../../vroot/home/task1/syscall/