#!/bin/bash
set -e
PKGNAME=stardock
VERSION=1.0.0
PKGREL=1
ARCH=x86_64
TARGETDIR=target

function _install_bin() {
  DESTDIR=$1
  mkdir -p ${DESTDIR}/usr/bin/
  cp builddir/main.exe ${DESTDIR}/usr/bin/${PKGNAME}
  mkdir -p ${DESTDIR}/usr/lib/
  cp builddir/lib${PKGNAME}.so ${DESTDIR}/usr/lib/
}

function _install_devel() {
  DESTDIR=$1
  mkdir -p ${DESTDIR}/usr/include/
  cp -r include/* ${DESTDIR}/usr/include/
  mkdir -p ${DESTDIR}/usr/share/pkgconfig
  cp -r ${PKGNAME}.pc ${DESTDIR}/usr/share/pkgconfig
}

function _build() {
  make
}

function _setup_targetdir() {
  if [ ! -d ${TARGETDIR} ]; then
    mkdir -p ${TARGETDIR}
    echo "*" > ${TARGETDIR}/.gitignore
  fi
}

function _setup_rpm_buildtree() {
  _setup_targetdir
  mkdir -p ${TARGETDIR}/rpm/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
}

function _setup_arch_buildtree() {
  _setup_targetdir
  mkdir -p ${TARGETDIR}/arch
}

function _compress() {
  IN=$1
  WORKSPACE=$(dirname $IN)
  DIRECTORY=$(basename $IN)
  DIR=$PWD
  cd $WORKSPACE
  tar cvf ${DIRECTORY}.tar ${DIRECTORY}
  gzip ${DIRECTORY}.tar
  cd $DIR
}

function _reset() {
  IN=$1
  rm -rf $IN
  rm -rf $IN.tar.gz
  mkdir -p $IN
}

function _build_rpm_package() {
  SPECFILE=$1
  DIR=$PWD
  cd ${TARGETDIR}/rpm/
  rpmbuild -bb --define "_topdir `pwd`" SPECS/$SPECFILE
  cd $DIR
}

function _build_arch_package() {
  DIR=$PWD
  cd ${TARGETDIR}/arch/
  makepkg --sign -f
  cd $DIR
}

function _sign_rpm_package() {
  IN=$1
  rpm --addsign $IN
}

function _package_rpm() {
  _setup_rpm_buildtree
  BIN_PACKAGE=${TARGETDIR}/rpm/SOURCES/${PKGNAME}-${VERSION}
  DEVEL_PACKAGE=${TARGETDIR}/rpm/SOURCES/${PKGNAME}-devel-${VERSION}
  BIN_RPM=${TARGETDIR}/rpm/RPMS/${ARCH}/${PKGNAME}-${VERSION}-${PKGREL}.${ARCH}.rpm
  DEVEL_RPM=${TARGETDIR}/rpm/RPMS/${ARCH}/${PKGNAME}-devel-${VERSION}-${PKGREL}.${ARCH}.rpm
  rm -rf ${BIN_PACKAGE} ${BIN_RPM}
  rm -rf ${DEVEL_PACKAGE} ${DEVEL_RPM}

  _reset ${BIN_PACKAGE}
  _reset ${DEVEL_PACKAGE}
  _install_bin ${BIN_PACKAGE}
  _install_devel ${DEVEL_PACKAGE}
  _compress ${BIN_PACKAGE}
  _compress ${DEVEL_PACKAGE}
  ./${PKGNAME}.spec.sh ${BIN_PACKAGE} ${PKGNAME} ${VERSION} ${PKGREL} ${ARCH} > ${TARGETDIR}/rpm/SPECS/${PKGNAME}.spec
  ./${PKGNAME}-devel.spec.sh ${DEVEL_PACKAGE} ${PKGNAME} ${VERSION} ${PKGREL} ${ARCH} > ${TARGETDIR}/rpm/SPECS/${PKGNAME}-devel.spec
  _build_rpm_package ${PKGNAME}.spec
  _build_rpm_package ${PKGNAME}-devel.spec
  _sign_rpm_package ${BIN_RPM}
  _sign_rpm_package ${DEVEL_RPM}
}

function _package_arch() {
  _setup_arch_buildtree
  PACKAGE=${TARGETDIR}/arch/${PKGNAME}-${VERSION}
  _reset ${PACKAGE}
  _install_bin ${PACKAGE}
  _install_devel ${PACKAGE}
  _compress ${PACKAGE}
  ./PKGBUILD.sh ${PACKAGE} ${PKGNAME} ${VERSION} ${PKGREL} ${ARCH} > ${TARGETDIR}/arch/PKGBUILD
  _build_arch_package
}

function _package() {
  TYPE=$1
  case $TYPE in
    RPM | rpm)
      _package_rpm
      ;;
    Arch | arch)
      _package_arch
      ;;
    *)
      echo "must specify a package type"
      ;;
  esac
}

_build
_package $1
