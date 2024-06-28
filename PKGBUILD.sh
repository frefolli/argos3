#!/bin/bash
PACKAGE=$1
PKGNAME=$2
VERSION=$3
PKGREL=$4
ARCH=$5
SHA256=$(sha256sum ${PACKAGE}.tar.gz | awk '{ print $1 }')

echo "# Contributor: Eric Noulard <eric.noulard@gmail.com>"
echo "# Contributor & Maintainer: Refolli Francesco <francesco.refolli@gmail.com>"
echo "pkgname=${PKGNAME}"
echo "pkgver=${VERSION}"
echo "pkgrel=${PKGREL}"
echo "pkgdesc='${PKGNAME} - A fast, parallel, multi-physics, and multi-robot simulator'"
echo "license=('MIT')"
echo "arch=('${ARCH}')"
echo "makedepends=()"
echo "depends=('freeglut>=2.6.0' 'freeimage>=3.15' gcc-libs glibc glu libglvnd libxi libxmu lua qt6-base)"
echo "url='https://github.com/frefolli/argos3'"
echo "source=("${PACKAGE}.tar.gz")"
echo "sha256sums=('${SHA256}')"
echo "package() {"
echo "  cd \$pkgname-\$pkgver"
echo "  cp -r usr \$pkgdir"
echo "}"
echo "options=(!debug)"
