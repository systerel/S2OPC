#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "s2opc_common" for configuration ""
set_property(TARGET s2opc_common APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(s2opc_common PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libs2opc_common.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS s2opc_common )
list(APPEND _IMPORT_CHECK_FILES_FOR_s2opc_common "${_IMPORT_PREFIX}/lib/libs2opc_common.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
