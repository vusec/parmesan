#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "CollabFuzzIDAssigner::LLVMIDAssigner" for configuration ""
set_property(TARGET CollabFuzzIDAssigner::LLVMIDAssigner APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(CollabFuzzIDAssigner::LLVMIDAssigner PROPERTIES
  IMPORTED_COMMON_LANGUAGE_RUNTIME_NOCONFIG ""
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/LLVMIDAssigner.so"
  IMPORTED_NO_SONAME_NOCONFIG "TRUE"
  )

list(APPEND _IMPORT_CHECK_TARGETS CollabFuzzIDAssigner::LLVMIDAssigner )
list(APPEND _IMPORT_CHECK_FILES_FOR_CollabFuzzIDAssigner::LLVMIDAssigner "${_IMPORT_PREFIX}/lib/LLVMIDAssigner.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
