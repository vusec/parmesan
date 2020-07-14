add_llvm_library(${PASS_NAME} MODULE IDAssigner.cpp)

#add_library(${PASS_NAME} MODULE IDAssigner.cc)
#target_link_libraries(${PASS_NAME} LLVMPassConfig)
#install (TARGETS ${PASS_NAME} DESTINATION ${ANGORA_PASS_DIR})

target_compile_definitions(${PASS_NAME} PRIVATE ${LLVM_DEFINITIONS})
target_include_directories(${PASS_NAME}
                           PRIVATE ${LLVM_INCLUDE_DIRS}
                                   ${PROJECT_SOURCE_DIR}/include)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"
   OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  target_compile_options(${PASS_NAME}
                         PRIVATE "-Wall" "-Wextra" "-Wno-unused-parameter")
endif()
add_library(CollabFuzzIDAssigner::${PASS_NAME} ALIAS ${PASS_NAME})
