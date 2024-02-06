# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

find_path(MBEDTLS_INCLUDE_DIR mbedtls/ssl.h)

# redefine CMake behavior for find_library if needed
if(USE_STATIC_MBEDTLS_LIB)
  set(_mbedtls_orig_lib_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})

  if(WIN32)
    list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .lib .a)
  else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
  endif()
endif()

find_library(MBEDTLS_LIBRARY mbedtls)
find_library(MBEDX509_LIBRARY mbedx509)
find_library(MBEDCRYPTO_LIBRARY mbedcrypto)

# restore CMake behavior for find_library
if(USE_STATIC_MBEDTLS_LIB)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${_mbedtls_orig_lib_suffixes})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MBEDTLS
  REQUIRED_VARS MBEDTLS_INCLUDE_DIR MBEDTLS_LIBRARY MBEDX509_LIBRARY MBEDCRYPTO_LIBRARY)

mark_as_advanced(MBEDTLS_INCLUDE_DIR MBEDTLS_LIBRARY MBEDX509_LIBRARY MBEDCRYPTO_LIBRARY)

if(MBEDTLS_FOUND AND NOT TARGET MBEDTLS)
    add_library(mbedtls UNKNOWN IMPORTED)
    set_target_properties(mbedtls PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${MBEDTLS_INCLUDE_DIR}
      IMPORTED_LOCATION "${MBEDTLS_LIBRARY}"
      )
    add_library(mbedx509 UNKNOWN IMPORTED)
    set_target_properties(mbedx509 PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${MBEDTLS_INCLUDE_DIR}
      IMPORTED_LOCATION "${MBEDX509_LIBRARY}"
      )
    add_library(mbedcrypto UNKNOWN IMPORTED)
    set_target_properties(mbedcrypto PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${MBEDTLS_INCLUDE_DIR}
      IMPORTED_LOCATION "${MBEDCRYPTO_LIBRARY}"
      )
endif()

if(MBEDTLS_FOUND)
  set(MBEDTLS_LIBRARIES ${MBEDTLS_LIBRARY} ${MBEDX509_LIBRARY} ${MBEDCRYPTO_LIBRARY})
  set(MBEDTLS_INCLUDE_DIRS ${MBEDTLS_INCLUDE_DIR})
endif()

message(STATUS " ${MBEDTLS_LIBRARIES}")
