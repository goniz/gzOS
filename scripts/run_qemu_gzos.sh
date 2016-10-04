#!/bin/bash -xe

make malta compile
if ! sudo brctl show | grep -q br0; then
	sudo brctl addbr br0
fi

taskset 0x00000001 qemu-system-mips 	-machine malta \
					-cpu 4KEc \
					-m 128 \
					-kernel build/gzOS.elf \
					-serial null \
					-serial null \
					-serial stdio \
					-device pcnet,netdev=net0,mac=00:11:22:33:44:55 \
					-netdev bridge,br=br0,id=net0,helper=/usr/lib/qemu/qemu-bridge-helper \
					-monitor telnet:0.0.0.0:9999,server,nowait \
					-s \
					$@

					#-netdev tap,id=net0 \
					#-chardev socket,path=/tmp/ivshmem_socket,id=ivshmemid \
					#-device ivshmem,chardev=ivshmemid,size=1,msi=off \

