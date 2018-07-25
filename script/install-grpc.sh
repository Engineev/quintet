#! /bin/sh

git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc
cd grpc
git submodule update --init
make
sudo make install

cd third_party/protobuf
sudo make install