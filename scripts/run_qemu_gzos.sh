#!/bin/bash -xe

make malta compile
qemu-system-mips 	-machine malta \
					-cpu 4KEc \
					-m 128 \
					-kernel build/gzOS.elf \
					-serial null \
					-serial null \
					-serial stdio \
					-netdev bridge,br=br0,id=hn0 \
					-device pcnet,netdev=hn0,id=nic1 \
					$@
