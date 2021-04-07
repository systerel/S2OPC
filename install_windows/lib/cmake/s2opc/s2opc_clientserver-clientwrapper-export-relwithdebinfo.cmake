#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "s2opc_clientwrapper" for configuration "RelWithDebInfo"
set_property(TARGET s2opc_clientwrapper APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(s2opc_clientwrapper PROPERTIES
  IMPORTED_IMPLIB_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libs2opc_clientwrapper.dll.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS s2opc_clientwrapper )
list(APPEND _IMPORT_CHECK_FILES_FOR_s2opc_clientwrapper "${_IMPORT_PREFIX}/lib/libs2opc_clientwrapper.dll.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
