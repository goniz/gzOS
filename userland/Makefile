.PHONY: compile clean
.PHONY: malta

compile: build
	make -C build

build:
	mkdir -p build

malta: build
	cd build && cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/malta/toolchain.cmake ../

clean:
	rm -rf build

