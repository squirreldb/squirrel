#!/bin/bash

set -e -u -E # this script will exit if any sub-command fails

########################################
# download & build depend software
########################################

WORK_DIR=`pwd`
DEPS_SOURCE=`pwd`/thirdsrc
DEPS_PREFIX=`pwd`/thirdparty
DEPS_CONFIG="--prefix=${DEPS_PREFIX} --disable-shared --with-pic"
FLAG_DIR=`pwd`/.build

export PATH=${DEPS_PREFIX}/bin:$PATH
mkdir -p ${DEPS_SOURCE} ${DEPS_PREFIX} ${FLAG_DIR}

if [ ! -f "${FLAG_DIR}/dl_third" ] || [ ! -d "${DEPS_SOURCE}/.git" ]; then
    rm -rf ${DEPS_SOURCE}
    git clone --depth=1 http://gitlab.baidu.com/baidups/third.git ${DEPS_SOURCE}
    touch "${FLAG_DIR}/dl_third"
fi

cd ${DEPS_SOURCE}

# boost
if [ ! -f "${FLAG_DIR}/boost_1_57_0" ] \
    || [ ! -d "${DEPS_PREFIX}/boost_1_57_0/boost" ]; then
    tar zxf boost_1_57_0.tar.gz
    rm -rf ${DEPS_PREFIX}/boost_1_57_0
    mv boost_1_57_0 ${DEPS_PREFIX}
    touch "${FLAG_DIR}/boost_1_57_0"
fi

# protobuf
if [ ! -f "${FLAG_DIR}/protobuf_2_6_1" ] \
    || [ ! -f "${DEPS_PREFIX}/lib/libprotobuf.a" ] \
    || [ ! -d "${DEPS_PREFIX}/include/google/protobuf" ]; then
    tar zxf protobuf-2.6.1.tar.gz
    cd protobuf-2.6.1
    ./configure ${DEPS_CONFIG}
    make -j4
    make install
    cd -
    touch "${FLAG_DIR}/protobuf_2_6_1"
fi

# snappy
if [ ! -f "${FLAG_DIR}/snappy_1_1_1" ] \
    || [ ! -f "${DEPS_PREFIX}/lib/libsnappy.a" ] \
    || [ ! -f "${DEPS_PREFIX}/include/snappy.h" ]; then
    tar zxf snappy-1.1.1.tar.gz
    cd snappy-1.1.1
    ./configure ${DEPS_CONFIG}
    make -j4
    make install
    cd -
    touch "${FLAG_DIR}/snappy_1_1_1"
fi

# sofa-pbrpc
if [ ! -f "${FLAG_DIR}/sofa-pbrpc_1_0_0" ] \
    || [ ! -f "${DEPS_PREFIX}/lib/libsofa-pbrpc.a" ] \
    || [ ! -d "${DEPS_PREFIX}/include/sofa/pbrpc" ]; then
    rm -rf sofa-pbrpc
    git clone --depth=1 http://gitlab.baidu.com/baidups/sofa-pbrpc.git sofa-pbrpc
    cd sofa-pbrpc
    sed -i '/BOOST_HEADER_DIR=/ d' depends.mk
    sed -i '/PROTOBUF_DIR=/ d' depends.mk
    sed -i '/SNAPPY_DIR=/ d' depends.mk
    echo "BOOST_HEADER_DIR=${DEPS_PREFIX}/boost_1_57_0" >> depends.mk
    echo "PROTOBUF_DIR=${DEPS_PREFIX}" >> depends.mk
    echo "SNAPPY_DIR=${DEPS_PREFIX}" >> depends.mk
    echo "PREFIX=${DEPS_PREFIX}" >> depends.mk
    make -j4
    make install
    cd -
    touch "${FLAG_DIR}/sofa-pbrpc_1_0_0"
fi

# common
if [ ! -f "${FLAG_DIR}/common" ] \
    || [ ! -f "${DEPS_PREFIX}/lib/libcommon.a" ] \
    || [ ! -d "${DEPS_PREFIX}/include/common" ]; then
    rm -rf common
    git clone https://github.com/baidu/common
    cd common
    sed -i 's/^PREFIX=.*/PREFIX=..\/..\/thirdparty/' config.mk
    make -j4
    make install
    cd -
    touch "${FLAG_DIR}/common"
fi

cd ${WORK_DIR}

########################################
# config depengs.mk
########################################

sed -i 's/^SOFA_PBRPC=.*/SOFA_PBRPC=.\/thirdparty/' depends.mk
sed -i 's/^PROTOBUF_DIR=.*/PROTOBUF_DIR=.\/thirdparty/' depends.mk
sed -i 's/^SNAPPY_DIR=.*/SNAPPY_DIR=.\/thirdparty/' depends.mk
sed -i 's/^BOOST_HEADER_DIR=.*/BOOST_HEADER_DIR=.\/thirdparty\/boost_1_57_0/' depends.mk
sed -i 's/^COMMON_INC=.*/COMMON_INC=.\/thirdparty\/include/' depends.mk
sed -i 's/^COMMON_LIB=.*/COMMON_LIB=.\/thirdparty\/lib/' depends.mk

########################################
# build squrriel
########################################

make -j4

