## setup windows 10 linux subsystem


* install ubuntu from windows store
* update system

```
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential libssh-dev openssh-server cmake 
```

* download

```
wget https://bigsearcher.com/mirrors/gcc/releases/gcc-7.3.0/gcc-7.3.0.tar.gz
wget https://dl.bintray.com/boostorg/beta/1.67.0.beta.1/source/boost_1_67_0_b1.tar.gz
```


* build gcc 7.3

```
tar -xvzf gcc-7.3.0.tar.gz
cd gcc-7.3.0
contrib/download_prerequisites
mkdir build
cd build
../configure -v --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu --prefix=/usr/local/gcc-7.3 --enable-checking=release --enable-languages=c,c++ --disable-multilib
make -j 7
sudo make install
sudo update-alternatives --install /usr/bin/c++ c++ /usr/local/gcc-7.3/bin/c++ 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/local/gcc-7.3/bin/g++ 100
sudo update-alternatives --install /usr/bin/gcc gcc /usr/local/gcc-7.3/bin/gcc 100
cd ~
echo 'export PATH=/usr/local/gcc-7.3/bin:$PATH' >> .bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/gcc-7.3/lib64:$LD_LIBRARY_PATH' >> .bashrc
source .bashrc
```

* build boost

```
tar -xvzf boost_1_67_0_b1.tar.gz
cd boost_1_67_0_b1
./bootstrap
sudo ./b2 --without-python  install
```


* start ssh server
  * `sudo vim /etc/ssh/sshd_config` enable password
  * `sudo service ssh start`