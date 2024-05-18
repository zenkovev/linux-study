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

echo "findpage 8" > /proc/mmaneg
echo "writeval 67 13" > /proc/mmaneg
echo "abra" > /proc/mmaneg
echo "findpage 41" > /proc/mmaneg
echo "shvabra" > /proc/mmaneg
echo "cadabra" > /proc/mmaneg
echo "writeval 19 77" > /proc/mmaneg

echo "-----"
dmesg | tail -n 10
echo "-----"

rmmod mmaneg.ko
echo "-----"
dmesg | tail -n 5
echo "-----"