#!/bin/bash

sudo apt-get -y install libz-dev libboost-dev automake libtool cmake pkgconf

wget https://github.com/google/protobuf/releases/download/v2.6.1/protobuf-2.6.1.tar.gz
tar xf protobuf-2.6.1.tar.gz
( cd protobuf-2.6.1 && ./configure --disable-shared --with-pic && make -j4 && sudo make install && sudo ldconfig )

git clone https://github.com/google/snappy
(cd snappy && sh ./autogen.sh && ./configure --disable-shared --with-pic && make -j4 && sudo make install )

git clone https://github.com/BaiduPS/sofa-pbrpc
( cd sofa-pbrpc && make -j4 && make install )

git clone https://github.com/baidu/common
( cd common && make -j4 )

sed -i 's/^SOFA_PBRPC=.*/SOFA_PBRPC=.\/sofa-pbrpc\/output/' depends.mk
sed -i 's/^PROTOBUF_DIR=.*/PROTOBUF_DIR=\/usr\/local/' depends.mk
sed -i 's/^SNAPPY_DIR=.*/SNAPPY_DIR=\/usr\/local/' depends.mk
sed -i 's/^BOOST_HEADER_DIR=.*/BOOST_HEADER_DIR=\/usr\/local\/include/' depends.mk
sed -i 's/^COMMON_INC=.*/COMMON_INC=.\/common\/include/' depends.mk
sed -i 's/^COMMON_LIB=.*/COMMON_LIB=.\/common/' depends.mk

make -j4
