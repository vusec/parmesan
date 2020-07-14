cmake_minimum_required(VERSION 3.16)
project(llvm-diff-parmesan VERSION 1.0.0 LANGUAGES C CXX ASM)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
add_compile_options(-fno-rtti)
add_compile_options(-static)
include(AddLLVM)


include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs support core irreader)

add_executable(llvm-diff-parmesan llvm-diff.cpp DiffConsumer.cpp DiffLog.cpp DifferenceEngine.cpp)
target_include_directories(llvm-diff-parmesan PUBLIC ${LLVM_INCLUDE_DIRS})
target_link_libraries(llvm-diff-parmesan ParmeSanIDAssigner::LLVMIDAssigner ParmeSanIDAssigner::Headers ${llvm_libs})
#target_link_libraries(llvm-diff-parmesan ParmeSanIDAssigner::Headers ${llvm_libs})


add_subdirectory(id-assigner-pass)
if(NOT TARGET ParmeSanIDAssigner::LLVMIDAssigner
   OR NOT TARGET ParmeSanIDAssigner::Headers)
  message(
    STATUS "ParmeSanIDAssigner does not exist, search using find_package")
  find_package(ParmeSanIDAssigner REQUIRED)
else()
  message(STATUS "ParmeSanIDAssigner already exists")
endif()

#set(LLVM_LINK_COMPONENTS
#  Core
#  IRReader
#  Support
#  IR
#  )

#add_llvm_tool(llvm-diff-parmesan
#  llvm-diff.cpp
#  DiffConsumer.cpp
#  DiffLog.cpp
#  DifferenceEngine.cpp
#
#  DEPENDS
#  intrinsics_gen
#  )

