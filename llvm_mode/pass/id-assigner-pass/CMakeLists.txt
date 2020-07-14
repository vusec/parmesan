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

if(BUILD_TESTING)
  find_program(LIT_PROGRAM lit)

  if(${LIT_PROGRAM} STREQUAL "LIT_PROGRAM-NOTFOUND")
    message(
      WARNING "Command 'lit' not found. You will not be able to run tests.")
  else()
    add_subdirectory(test)
  endif()
endif()

install(EXPORT LLVMExports
        FILE CollabFuzzIDAssignerConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CollabFuzzIDAssigner
        NAMESPACE CollabFuzzIDAssigner::)
