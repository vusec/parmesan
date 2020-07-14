# Install script for directory: /home/sebastian/Programming/fuzzing/Parmesan-rebase/llvm-diff-parmesan/id-assigner-pass

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/CollabFuzzIDAssigner/CollabFuzzIDAssignerConfig.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/CollabFuzzIDAssigner/CollabFuzzIDAssignerConfig.cmake"
         "/home/sebastian/Programming/fuzzing/Parmesan-rebase/llvm-diff-parmesan/build/id-assigner-pass/CMakeFiles/Export/lib/cmake/CollabFuzzIDAssigner/CollabFuzzIDAssignerConfig.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/CollabFuzzIDAssigner/CollabFuzzIDAssignerConfig-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/CollabFuzzIDAssigner/CollabFuzzIDAssignerConfig.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/CollabFuzzIDAssigner" TYPE FILE FILES "/home/sebastian/Programming/fuzzing/Parmesan-rebase/llvm-diff-parmesan/build/id-assigner-pass/CMakeFiles/Export/lib/cmake/CollabFuzzIDAssigner/CollabFuzzIDAssignerConfig.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/CollabFuzzIDAssigner" TYPE FILE FILES "/home/sebastian/Programming/fuzzing/Parmesan-rebase/llvm-diff-parmesan/build/id-assigner-pass/CMakeFiles/Export/lib/cmake/CollabFuzzIDAssigner/CollabFuzzIDAssignerConfig-noconfig.cmake")
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/sebastian/Programming/fuzzing/Parmesan-rebase/llvm-diff-parmesan/build/id-assigner-pass/src/cmake_install.cmake")
  include("/home/sebastian/Programming/fuzzing/Parmesan-rebase/llvm-diff-parmesan/build/id-assigner-pass/include/cmake_install.cmake")

endif()

