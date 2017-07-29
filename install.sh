#!/bin/sh

if [ ! -d build ]; then
	mkdir build
fi
cmake -GNinja . -DBoost_USE_STATIC_LIBS=ON -DCMAKE_INSTALL_PREFIX=. -B./build
cd build
# make
ninja install
cd ..
