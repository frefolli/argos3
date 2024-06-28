#!/bin/bash
set -e
PKGNAME=argos3_simulator
VERSION=3.0.1
PKGREL=1
ARCH=x86_64
TARGETDIR=target

function _build() {
  make
  make docs
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
  PACKAGE=${TARGETDIR}/rpm/SOURCES/${PKGNAME}-${VERSION}
  RPM=${TARGETDIR}/rpm/RPMS/${ARCH}/${PKGNAME}-${VERSION}-${PKGREL}.${ARCH}.rpm
  rm -rf ${PACKAGE} ${RPM}
  _reset ${PACKAGE}
  make install DESTDIR=${PACKAGE}
  _compress ${PACKAGE}
  ./${PKGNAME}.spec.sh ${PACKAGE} ${PKGNAME} ${VERSION} ${PKGREL} ${ARCH} > ${TARGETDIR}/rpm/SPECS/${PKGNAME}.spec
  _build_rpm_package ${PKGNAME}.spec
  _sign_rpm_package ${RPM}
}

function _package_arch() {
  _setup_arch_buildtree
  PACKAGE=${TARGETDIR}/arch/${PKGNAME}-${VERSION}
  _reset ${PACKAGE}
  make install DESTDIR=${PACKAGE}
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
