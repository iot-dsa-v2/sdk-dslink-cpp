## Ubuntu Setup
1. install libssl-dev
1. download and extract boost 1.66
1. cd boost_1_66_0/
1. run `./bootstrap.sh`
1. run `./b2 install`

## Windows Visual Studio Setup

1. install visual studio 2017
1. clone `https://github.com/Microsoft/vcpkg.git`
 into C:\vcpkg
1. run `.\bootstrap-vcpkg.bat` in vcpkg folder
1. install packages
```
.\vcpkg install boost:x64-windows openssl:x64-windows gtest:x64-windows
```
1. run `.\vcpkg integrate install`
1. In visual studio, use `file-open-folder..` instead of creating project 

## WIndows MinGW Setup

* install mysys2 64 bit
* run `pacman -Syu` and close window when finished
* start a new msys2 mingw64 window
* `pacman -Su`
* `pacman -S mingw-w64-x86_64-toolchain` install all
* `pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-gtest`
* download and extract boost 1.66
* `bootstrap.sh`
* `./b2 --without-python --layout=system address-model=64 variant=release --includedir=/mingw64/include --libdir=/mingw64/lib install`

## macOS Setup

### Using Homebrew

If you don't have homebrew installed already, follow the setup instructions [here](https://brew.sh/)

Once homebrew is installed, run the command

```brew install cmake ninja boost openssl```

Once the dependencies have been installed, you should be good to go. 

Run ```./install.sh``` to build the sdk or you can use cmake directly by doing something along the lines of

```
$ git clone https://github.com/iot-dsa-v2/sdk-dslink-cpp.git
$ cd sdk-dslink-cpp
$ mkdir build
$ cd build
$ cmake -GNinja ..
$ ninja
```
