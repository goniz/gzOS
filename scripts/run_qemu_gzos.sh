#!/bin/bash -xe

make clean malta compile
qemu-system-mips -M malta -m 32 -kernel build/gzOS.elf -serial null -serial null -serial stdio $@
