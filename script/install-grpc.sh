#! /bin/sh

export CC=gcc-5
export CXX=g++-5

git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc
cd grpc
git submodule update --init
make -j all
sudo make install

cd third_party/protobuf
sudo make install