#!/bin/bash

pushd ../mf &&
./build.sh $1 &&
popd &&
mkdir -p build &&
pushd build &&
cmake \
    -DCMAKE_BUILD_TYPE=$1 \
    -DCMAKE_INSTALL_PREFIX=../dist .. &&
make install
