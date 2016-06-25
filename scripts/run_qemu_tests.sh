#!/bin/bash -xe

make malta compile
qemu-system-mips -M malta -m 32 -kernel build/tests.elf -serial null -serial null -serial stdio $@
