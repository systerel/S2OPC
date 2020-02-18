
# Copyright (C) Systerel SAS 2019, all rights reserved.

find_path(PAHO_INCLUDE_DIR MQTTClient.h)
set(_paho_orig_lib_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)

find_library(PAHO3C_LIBRARY paho-mqtt3c-static)
find_library(PAHO3A_LIBRARY paho-mqtt3a-static)
find_library(PAHO3CS_LIBRARY paho-mqtt3cs-static)
find_library(PAHO3AS_LIBRARY paho-mqtt3as-static)
set(CMAKE_FIND_LIBRARY_SUFFIXES ${_paho_orig_lib_suffixes})

include(FindPackageHandleStandardArgs)
set(PAHO_FIND_REQUIRED ${Paho_FIND_REQUIRED})
find_package_handle_standard_args(PAHO DEFAULT_MSG PAHO_INCLUDE_DIR PAHO3C_LIBRARY PAHO3A_LIBRARY PAHO3CS_LIBRARY PAHO3AS_LIBRARY)

if (PAHO_FOUND)
	set(PAHO_LIBRARIES "${PAHO3C_LIBRARY}" "${PAHO3A_LIBRARY}" "${PAHO3CS_LIBRARY}" "${PAHO3AS_LIBRARY}")
	set(PAHO_INCLUDE_DIRS ${PAHO_INCLUDE_DIR})

endif()

mark_as_advanced(PAHO_INCLUDE_DIRS PAHO3C_LIBRARY PAHO3A_LIBRARY PAHO3CS_LIBRARY PAHO3AS_LIBRARY)
