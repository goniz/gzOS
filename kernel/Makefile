.PHONY: compile clean
.PHONY: malta

compile: build
	make -C build -j6

build:
	mkdir -p build

malta: build
	cd build && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../platform/malta/toolchain.cmake ../

clean:
	rm -rf build

