#!/bin/sh

cd utility
find -name CMakeFiles -type d | xargs rm -rf
find \( -name CMakeCache.txt -or -name Makefile -or -name cmake_install.cmake \
                             -or -name install_manifest.txt \) -type f -delete
cmake -DCMAKE_BUILD_TYPE=Release .
make install

cd ../canvas
find -name CMakeFiles -type d | xargs rm -rf
find \( -name CMakeCache.txt -or -name Makefile -or -name cmake_install.cmake \
                             -or -name install_manifest.txt \) -type f -delete
cmake -DCMAKE_BUILD_TYPE=Release .
make install
