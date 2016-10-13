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

