#!/bin/bash -xe

make malta compile
qemu-system-mips -machine malta -cpu 4KEc -m 128 -kernel build/gzOS.elf -serial null -serial null -serial stdio $@
