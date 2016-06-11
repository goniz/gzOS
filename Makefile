.PHONY: compile clean copy
.PHONY: malta

compile: build
	make -C build

build:
	mkdir -p build

malta: build
	cd build && cmake -DCMAKE_TOOLCHAIN_FILE=../platform/malta/toolchain.cmake ../

clean:
	rm -rf build

copy:
	cp build/gzLoader.img /run/media/gz/System/kernel.img
	umount /run/media/gz/{System,Storage}

