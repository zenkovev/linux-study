#!/bin/sh

insmod mmaneg.ko
echo "-----"
dmesg | tail -n 5
echo "-----"

echo > /proc/mmaneg
echo "ahaha" > /proc/mmaneg

echo "listvma" > /proc/mmaneg
echo "-----"
dmesg | tail -n 15
echo "-----"

cat /proc/mmaneg

echo "listvma" > /proc/mmaneg
echo "-----"
dmesg | tail -n 15
echo "-----"

./check
echo "-----"
dmesg | tail -n 15
echo "-----"

rmmod mmaneg.ko
echo "-----"
dmesg | tail -n 5
echo "-----"