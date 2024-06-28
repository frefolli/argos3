BUILDDIR=./builddir
LIB=./builddir/argos3.so
INCLUDE=$(find ./include/ -type f)
SRC=$(find ./src/ -type f)
MESON_CONF=meson.build
BUILD_TYPE=release

@all: ${LIB}

${BUILDDIR}: ${MESON_CONF}
	meson setup --buildtype=${BUILD_TYPE} ${BUILDDIR}

${LIB}: ${BUILDDIR} ${SRC} ${INCLUDE}
	ninja -C ${BUILDDIR}

clean:
	rm -rf ${BUILDDIR}
	rm -rf target

test:
	meson test -C ${BUILDDIR}

install:
	mkdir -p ${DESTDIR}/usr/bin/
	cp builddir/argos3 ${DESTDIR}/usr/bin/argos3

	mkdir -p ${DESTDIR}/usr/lib/argos3/
	cp builddir/libargos3*.so ${DESTDIR}/usr/lib/argos3/

	mkdir -p ${DESTDIR}/usr/include/
	cp -r include/argos3 ${DESTDIR}/usr/include/

	mkdir -p ${DESTDIR}/usr/share/doc/
	cp -r doc/html ${DESTDIR}/usr/share/doc/argos3

	mkdir -p ${DESTDIR}/usr/share/pkgconfig
	cp -r argos3.pc ${DESTDIR}/usr/share/pkgconfig

	mkdir -p ${DESTDIR}/etc/ld.so.conf.d
	cp argos3.conf ${DESTDIR}/etc/ld.so.conf.d

docs:
	make -C ./doc
