#!/bin/bash

VERSION="1.17"
if [ ! -f wget-${VERSION}.tar.xz ]; then
	wget http://ftp.gnu.org/gnu/wget/wget-${VERSION}.tar.xz
fi

rm -rf wget-${VERSION} build
tar xJf wget-${VERSION}.tar.xz

export ROOTDIR="${PWD}"
cd wget-${VERSION}
export CROSS_COMPILE="arm-none-linux-gnueabi"
export CPPFLAGS="-I${ROOTDIR}/openssl/include -I${ROOTDIR}/zlib/include"
export LDFLAGS="-L${ROOTDIR}/openssl/libs -L${ROOTDIR}/zlib/libs"
export AR=${CROSS_COMPILE}-ar
export AS=${CROSS_COMPILE}-as
export LD=${CROSS_COMPILE}-ld
export RANLIB=${CROSS_COMPILE}-ranlib
export CC=${CROSS_COMPILE}-gcc
export NM=${CROSS_COMPILE}-nm
export LIBS="-static -lc -lssl -lcrypto -lz -ldl"

./configure \
	--prefix=${ROOTDIR}/build \
	--target=${CROSS_COMPILE} \
	--host=${CROSS_COMPILE} \
	--build=i586-pc-linux-gnu \
	--with-ssl=openssl --with-zlib \
	--without-included-regex \
	--enable-nls \
	--enable-dependency-tracking \
	--with-metalink \
	--sysconfdir=${ROOTDIR}/build/etc \
	--localedir=${ROOTDIR}/build/usr/share/locale \
	--mandir=${ROOTDIR}/build/usr/share/man \
	--infodir=${ROOTDIR}/build/usr/share/info \
	--bindir=${ROOTDIR}/build/usr/bin

make
make install
