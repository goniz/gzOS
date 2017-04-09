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
	./scripts/create_initrd.sh build/rootfs/ kernel/initrd.cpio

qemu: compile
qemu: rootfs
qemu:
	sudo ./scripts/run_qemu_gzos.sh

tests: compile
tests: rootfs
tests:
	sudo ./scripts/run_qemu_gzos.sh kernel/build/tests.elf

debug: compile
debug: rootfs
debug:
	sudo ./scripts/run_qemu_gzos.sh -S

debug-tests: compile
debug-tests: rootfs
debug-tests:
	sudo ./scripts/run_qemu_gzos.sh kernel/build/tests.elf -S

gdb:
	source /data/.imgtec.sh && TERM=xterm mips-mti-elf-gdb kernel/build/gzOS.elf

gdb-tests:
	source /data/.imgtec.sh && TERM=xterm mips-mti-elf-gdb kernel/build/tests.elf

objdump:
	source /data/.imgtec.sh && mips-mti-elf-objdump -Cd kernel/build/gzOS.elf | less

shell:
	nc -v -l -p 8888
