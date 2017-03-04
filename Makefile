.PHONY: compile clean
.PHONY: malta

compile: build
	make -C kernel compile
	make -C userland compile

build:
	make -C kernel build
	make -C userland build

malta: build
	make -C kernel malta
	make -C userland malta

clean:
	make -C kernel clean
	make -C userland clean

rootfs:
	./scripts/create_rootfs.sh
	./scripts/create_flash.sh build/rootfs/ kernel/flash.bin

qemu: compile
qemu: rootfs
qemu:
	sudo ./scripts/run_qemu_gzos.sh

debug: compile
debug: rootfs
debug:
	sudo ./scripts/run_qemu_gzos.sh -S

gdb:
	TERM=xterm mips-mti-elf-gdb kernel/build/gzOS.elf

objdump:
	mips-mti-elf-objdump -Cd kernel/build/gzOS.elf | less
