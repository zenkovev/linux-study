#!/bin/bash

echo "sendkey a" | socat - unix-connect:../qemu-monitor-socket && echo
echo "sendkey b" | socat - unix-connect:../qemu-monitor-socket && echo
echo "sendkey c" | socat - unix-connect:../qemu-monitor-socket && echo
echo "sendkey 1" | socat - unix-connect:../qemu-monitor-socket && echo
echo "sendkey 2" | socat - unix-connect:../qemu-monitor-socket && echo