execute_process(
  COMMAND bash ./gen_grpc_files.bash
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

function(move FILENAME TAR_DIR)
  file(RENAME ${FILENAME} "${TAR_DIR}/${FILENAME}")
endfunction()

move("quintet.grpc.pb.h"  "../include/quintet/rpc")
move("quintet.pb.h"       "../include/quintet/rpc")
move("raft.grpc.pb.h"     "../include/quintet/raft/rpc")
move("raft.pb.h"          "../include/quintet/raft/rpc")

move("quintet.grpc.pb.cc" "../src/rpc")
move("quintet.pb.cc"      "../src/rpc")
move("raft.grpc.pb.cc"    "../src/raft/rpc")
move("raft.pb.cc"         "../src/raft/rpc")

