#!/bin/bash

MODE=$1
if [ "$MODE" == "recompile" ]; then
    cd linux-6.7.4
    LOCALVERSION="" make -j9 all
    INSTALL_PATH=../boot make install
    INSTALL_MOD_PATH=../vroot make modules_install
    cd ..

    cd vroot
    find . | cpio -ov --format=newc | gzip -9 > ../initramfs
    cd ..

    qemu-system-x86_64 -kernel ./boot/vmlinuz-6.7.4 -initrd ./initramfs \
        --enable-kvm -cpu host -nographic -append "console=ttyS0" \
        -monitor unix:qemu-monitor-socket,server,nowait
elif [ "$MODE" == "rebuild" ]; then
    cd linux-6.7.4
    INSTALL_MOD_PATH=../vroot make modules_install
    cd ..

    cd vroot
    find . | cpio -ov --format=newc | gzip -9 > ../initramfs
    cd ..

    qemu-system-x86_64 -kernel ./boot/vmlinuz-6.7.4 -initrd ./initramfs \
        --enable-kvm -cpu host -nographic -append "console=ttyS0" \
        -monitor unix:qemu-monitor-socket,server,nowait
elif [ "$MODE" == "run" ]; then
    qemu-system-x86_64 -kernel ./boot/vmlinuz-6.7.4 -initrd ./initramfs \
        --enable-kvm -cpu host -nographic -append "console=ttyS0" \
        -monitor unix:qemu-monitor-socket,server,nowait
else
    echo "error: incorrect option"
fi