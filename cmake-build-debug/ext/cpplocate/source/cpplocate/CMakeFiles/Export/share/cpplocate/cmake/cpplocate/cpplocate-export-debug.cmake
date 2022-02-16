#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cpplocate::cpplocate" for configuration "Debug"
set_property(TARGET cpplocate::cpplocate APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(cpplocate::cpplocate PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libcpplocated.2.3.0.dylib"
  IMPORTED_SONAME_DEBUG "@rpath/libcpplocated.2.dylib"
  )

list(APPEND _IMPORT_CHECK_TARGETS cpplocate::cpplocate )
list(APPEND _IMPORT_CHECK_FILES_FOR_cpplocate::cpplocate "${_IMPORT_PREFIX}/lib/libcpplocated.2.3.0.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
