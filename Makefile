.PHONY: compile clean
.PHONY: malta

compile: build
	make -C kernel compile

build:
	make -C kernel build

malta: build
	make -C kernel malta

clean:
	make -C kernel clean

