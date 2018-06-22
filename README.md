# Installation

1. Install dependent packages
  * cmake
  * g++
  * boost version 1.66
  * openssl
  * ninja

2. Clone the SDK

  `$ git clone https://github.com/iot-dsa-v2/sdk-dslink-cpp.git`

3. If dependent packages (i.e. boost, openssl) are not installed to 
standard locations, setup SDK to point to locations which those packages were installed

  - By default, Boost install location is set to `/usr/local`. Modify ./install.sh to point it to your Boost install location

  - By default, openSSL root directory is set to `/usr/local/ssl` on Linux and `/usr/local/Cellar/openssl/1.0.2n` on macOS. Modify `CMakeListConfig.txt`to point it to your openssl install location

4. On Windows and Android, refer to platform specific installation information below.

  On other platform, run ```./install.sh``` to build the sdk or you can use cmake directly by doing something along the lines of

  ```
  $ cd sdk-dslink-cpp
  $ mkdir build
  $ cd build
  $ cmake -GNinja ..
  $ ninja
  ```


# Platform specific installation information

## Ubuntu Setup
1. Install Boost
  - download and extract boost 1.66
  - cd boost_1_66_0/
  - run `./bootstrap.sh`
  - run `sudo ./b2 install`

2. Install openssl
  - `sudo apt install libssl-dev`

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

## Windows MinGW Setup

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

## Android Setup

* Install Termux app
* Use Termux, install the following packages
  - cmake
  - clang
  - booost and boost-dev
  - openssl and openssl-dev
  - git
  - ninja

* Check your Boost library's install root directory. Check if your Boost installlation came with static Boost libraries. Modify install.sh script accordingly.

For example, if Boost is installed under /usr and there are no static Boost libraries installed, modify install.sh script as below

cmake -GNinja . -DBOOST_ROOT=/usr -DCMAKE_INSTALL_PREFIX=. -B./build
