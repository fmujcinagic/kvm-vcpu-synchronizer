#!/bin/sh
qemu-system-x86_64 -kernel arch/x86/boot/bzImage -initrd ../../initramfs.cpio.gz -nographic -append "console=ttyS0" -enable-kvm -cpu host -m 2G -smp 2
