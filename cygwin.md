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
cd msgpack-c
cmake -DMSGPACK_ENABLE_CXX=OFF -DMSGPACK_BUILD_TESTS=OFF -DMSGPACK_BUILD_EXAMPLES=OFF .
make
make install
```

## debug with clion

 add `{cygwin_path}/bin` and `{cygwin_path}/usr/local/bin`  to system enviroument Path