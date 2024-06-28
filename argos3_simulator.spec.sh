#!/bin/bash
PACKAGE=$1
PKGNAME=$2
VERSION=$3
PKGREL=$4
ARCH=$5

echo "Name:           ${PKGNAME}"
echo "Version:        ${VERSION}"
echo "Release:        ${PKGREL}%{?dist}"
echo "BuildArch:      ${ARCH}"
echo "Summary:        A fast, parallel, multi-physics, and multi-robot simulator"
echo "License:        MIT"
echo "Group:          Development/Tools"
echo "Vendor:         Humanity"
echo ""
echo "Source:         ${PKGNAME}-${VERSION}.tar.gz"
echo ""
echo "Url: https::/github.com/frefolli/argos3"
echo "Requires: gcc >= 5.4, gcc-c++ >= 5.4, pkg-config, cmake >= 3.5, freeglut-devel >= 2.8.0, libXmu-devel >= 1.0, libXi-devel >= 1.0, libqt5-qtbase-devel >= 5.5, libfreeimage3 >= 3.15, libfreeimageplus3 >= 3.15, freeimage-devel >= 3.15, lua >= 5.3, lua-devel >= 5.3, liblua5_3 >= 5.3"
echo "Prefix: /usr"
echo ""
echo "%description"
echo "ARGoS (multi-physics multi-robot simulator)"
echo "ARGoS is a highly scalable multi-robot simulator. Among its distinctive features, there are: (i) modularity (robots, devices, physics engines, visualizations and controllers are plug-ins), (ii) tunable accuracy, and (iii) the possibility to run multiple physics engines at the same time."
echo ""
echo "%prep"
echo "%setup"
echo ""
echo "%install"
echo "rm -rf \$RPM_BUILD_ROOT"
echo "mkdir -p \$RPM_BUILD_ROOT"
echo "cp -r usr \$RPM_BUILD_ROOT"
echo ""
echo "%clean"
echo "rm -rf \$RPM_BUILD_ROOT"
echo ""
echo "%files"
find $PACKAGE -type f -printf "/%P\n"
echo ""
echo "%changelog"
echo "* Fri Jun 28 2024 Refolli Francesco <frefolli@campus.unimib.it> - ${VERSION}"
echo "- Created a RPM Package for this Library"
echo "* Sun Jul 4 2010 Eric Noulard <eric.noulard@gmail.com> - 3.0.0-1"
