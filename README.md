## Ubuntu Setup

1. install libssl-dev
1. download and extract boost 1.66
1. run `bootstrap.sh`
1. run `/b2 install`

## Windows 10 Setup

1. install visual studio 2017
1. clone `https://github.com/Microsoft/vcpkg.git`
 into C:\vcpkg
1. run `.\bootstrap-vcpkg.bat` in vcpkg folder
1. install packages
```
.\vcpkg install boost:x64-windows openssl:x64-windows gtest:x64-windows
```
1. run `.\vcpkg integrate install`
1. make a copy of C:\vcpkg\installed\x64-windows\debug\lib\manual-link\gtest_maind.lib in the same folder and rename the copy to gtest_main.lib
1. In visual studio, use `file-open-folder..` instead of creating project 

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
