execute_process(
  COMMAND bash ./gen_grpc_files.bash
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

file(GLOB GRPC_HEADERS *.h)
foreach (HEADER ${GRPC_HEADERS})
  file(RELATIVE_PATH HEADER_NAME ${CMAKE_CURRENT_LIST_DIR} ${HEADER})
  set(NEW_HEADER "../include/quintet/rpc/${HEADER_NAME}")
  file(RENAME ${HEADER} ${NEW_HEADER})
endforeach ()

file(GLOB GRPC_SOURCES *.cc)
foreach (SOURCE ${GRPC_SOURCES})
  file(RELATIVE_PATH SOURCE_NAME ${CMAKE_CURRENT_LIST_DIR} ${SOURCE})
  set(NEW_SOURCE "../src/rpc/${SOURCE_NAME}")
  file(RENAME ${SOURCE} ${NEW_SOURCE})
endforeach ()
