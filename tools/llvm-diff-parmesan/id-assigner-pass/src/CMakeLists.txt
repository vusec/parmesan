if (${BUILD_STANDALONE_PASS})
    add_llvm_library(${PASS_NAME} SHARED IDAssigner.cpp)
else()
    add_llvm_library(${PASS_NAME} STATIC IDAssigner.cpp)
endif()
#add_llvm_library(${PASS_NAME} SHARED IDAssigner.cpp)
target_compile_definitions(${PASS_NAME} PRIVATE ${LLVM_DEFINITIONS})
target_include_directories(${PASS_NAME}
                           PRIVATE ${LLVM_INCLUDE_DIRS}
                                   ${PROJECT_SOURCE_DIR}/include)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"
   OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  target_compile_options(${PASS_NAME}
                         PRIVATE "-Wall" "-Wextra" "-Wno-unused-parameter")
endif()
add_library(ParmeSanIDAssigner::${PASS_NAME} ALIAS ${PASS_NAME})
