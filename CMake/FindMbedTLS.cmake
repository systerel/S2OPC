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

if (MBEDTLS_USE_STATIC_LIBS)
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

if(MBEDTLS_USE_STATIC_LIBS)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${_mbedtls_orig_lib_suffixes})
endif()

include(FindPackageHandleStandardArgs)
set(MBEDTLS_FIND_REQUIRED ${MbedTLS_FIND_REQUIRED})
find_package_handle_standard_args(MBEDTLS DEFAULT_MSG
    MBEDTLS_INCLUDE_DIR MBEDTLS_LIBRARY MBEDX509_LIBRARY MBEDCRYPTO_LIBRARY)

if (MBEDTLS_FOUND)
	set(MBEDTLS_LIBRARIES "${MBEDTLS_LIBRARY}" "${MBEDX509_LIBRARY}" "${MBEDCRYPTO_LIBRARY}")
	set(MBEDTLS_INCLUDE_DIRS ${MBEDTLS_INCLUDE_DIR})
endif()

mark_as_advanced(MBEDTLS_INCLUDE_DIR MBEDTLS_LIBRARY MBEDX509_LIBRARY MBEDCRYPTO_LIBRARY)
