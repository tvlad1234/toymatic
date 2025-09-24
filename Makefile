all: fw tool

fw: build_libopencm3
	make -C firmware

libopencm3:
	git submodule init
	git submodule update

build_libopencm3: libopencm3
	make -C libopencm3 -j$(nproc)
	touch build_libopencm3

tool:
	make -C host_tool

install_tool:
	make -C host_tool install

clean:
	make -C firmware clean
	make -C host_tool clean
	rm -rf beremiz_example/build
