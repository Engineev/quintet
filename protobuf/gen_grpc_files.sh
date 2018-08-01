#!/usr/bin/env bash

protoc -I=. --cpp_out=../src/common/rpc external.proto
protoc -I=. --grpc_out=../src/common/rpc \
  --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` external.proto

protoc -I=. --cpp_out=../src/server/rpc internal.proto
protoc -I=. --grpc_out=../src/server/rpc \
  --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` internal.proto