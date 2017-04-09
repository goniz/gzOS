#!/bin/bash -xe

dir=$(readlink -f $(dirname $0))

elf="${dir}/../kernel/build/gzOS.elf"
flash="${dir}/../kernel/flash.bin"
initrd="${dir}/../kernel/initrd.cpio"

if [[ -f "$1" ]]; then
	elf="$1"
	shift 1
fi

#make malta compile

if ! sudo brctl show | grep -q br0; then
	sudo brctl addbr br0
fi

sudo ifconfig br0 1.1.1.2/24

qemu-system-mips 			-machine malta \
					-cpu 4KEc \
					-m 128 \
					-bios none \
					-kernel $elf \
					-initrd $initrd \
					-display none \
					-serial null \
					-serial null \
					-serial stdio \
					-device pcnet,netdev=net0,mac=00:11:22:33:44:55 \
					-netdev bridge,br=br0,id=net0,helper=/usr/lib/qemu/qemu-bridge-helper \
					-monitor telnet:0.0.0.0:9999,server,nowait \
					-pflash "$flash" \
					-s \
					$@

					#-netdev tap,id=net0 \
					#-chardev socket,path=/tmp/ivshmem_socket,id=ivshmemid \
					#-device ivshmem,chardev=ivshmemid,size=1,msi=off \
