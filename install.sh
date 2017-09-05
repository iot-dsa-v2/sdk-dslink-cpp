#!/bin/sh

if [ ! -d build ]; then
	mkdir build
fi

if [ ! -f deps/msgpack/CMakeLists.txt ]; then
	git clone https://github.com/msgpack/msgpack-c.git deps/msgpack
fi
cp deps/CMakeLists.txt.in deps/msgpack/CMakeLists.txt
 
cmake -GNinja . -DBoost_USE_STATIC_LIBS=ON -DCMAKE_INSTALL_PREFIX=. -B./build
cd build
# make
ninja install
cd ..
