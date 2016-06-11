#!/bin/bash -xe

make malta compile
qemu-system-mips -M malta -m 256 -kernel build/gzOS.elf -serial null -serial null -serial stdio
