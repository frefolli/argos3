@all:
	meson setup builddir
	ninja -C builddir

clean:
	rm -rf builddir
