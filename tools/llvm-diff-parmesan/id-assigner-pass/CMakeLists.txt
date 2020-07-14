cmake_minimum_required(VERSION 3.13)

project(IDAssigner)

include(GNUInstallDirs)

if(NOT LLVM_FOUND)
  find_package(LLVM REQUIRED CONFIG)
  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
  list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
  include(AddLLVM)
endif()

set(PASS_NAME "LLVM${PROJECT_NAME}")

add_subdirectory(src)
add_subdirectory(include)

install(EXPORT LLVMExports
        FILE ParmeSanIDAssignerConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ParmeSanIDAssigner
        NAMESPACE ParmeSanIDAssigner::)
