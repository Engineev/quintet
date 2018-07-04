protoc -I=. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` quintet.proto
protoc -I=. --cpp_out=. quintet.proto