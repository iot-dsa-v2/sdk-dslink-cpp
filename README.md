## Windows 10 Setup

1. install visual studio 2017
1. clone `https://github.com/Microsoft/vcpkg.git`
 into C:\vcpkg
1. run `.\bootstrap-vcpkg.bat` in vcpkg folder
1. install packages
```
.\vcpkg install boost:x64-windows

.\vcpkg install openssl:x64-windows

.\vcpkg install gtest:x64-windows

```
1. run `.\vcpkg integrate install`
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
