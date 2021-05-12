#!/bin/bash

rm -Rfv ./build
rm -Rfv ./lib
rm -Rfv ./deb/usr/local/localFilter/lib/*
rm -fv ./*.deb

make distclean
qmake
make

cp -Rp "./lib" "./deb/usr/local/localFilter"

chmod -R 755 ./deb
dpkg-deb --build ./deb .