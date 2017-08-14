## install packages

install these packages for cygwin
* cmake
* make
* gcc-g++
* gdb
* openssl-devel
* libboost-devel
* zlib-devel


### install msgpack for cygwin

```
git clone https://github.com/msgpack/msgpack-c.git
cmake -DMSGPACK_ENABLE_SHARED=OFF -DMSGPACK_ENABLE_CXX=OFF -DMSGPACK_BUILD_TESTS=OFF -DMSGPACK_BUILD_EXAMPLES=OFF .
make
make install
```

## debug with clion

everything should be ready for clion, except you need to add cygwin/bin path to Path in system enviroument 