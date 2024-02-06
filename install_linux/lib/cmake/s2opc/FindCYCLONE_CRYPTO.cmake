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

find_path(CYCLONE_CRYPTO_INCLUDE_DIR core/crypto.h PATHS /usr/local/include/cyclone_crypto)

find_library(CYCLONE_COMMON_LIBRARY cyclone_common)
find_library(CYCLONE_CRYPTO_LIBRARY cyclone_crypto)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CYCLONE_CRYPTO
  REQUIRED_VARS CYCLONE_CRYPTO_INCLUDE_DIR CYCLONE_COMMON_LIBRARY CYCLONE_CRYPTO_LIBRARY)

mark_as_advanced(CYCLONE_CRYPTO_INCLUDE_DIR CYCLONE_COMMON_LIBRARY CYCLONE_CRYPTO_LIBRARY)

if(CYCLONE_CRYPTO_FOUND AND NOT TARGET CYCLONE_CRYPTO)
    add_library(cyclone_common UNKNOWN IMPORTED)
    set_target_properties(cyclone_common PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${CYCLONE_CRYPTO_INCLUDE_DIR}
        IMPORTED_LOCATION "${CYCLONE_COMMON_LIBRARY}"
        )
    add_library(cyclone_crypto UNKNOWN IMPORTED)
    set_target_properties(cyclone_crypto PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${CYCLONE_CRYPTO_INCLUDE_DIR}
        IMPORTED_LOCATION "${CYCLONE_CRYPTO_LIBRARY}"
        )
endif()

if (CYCLONE_CRYPTO_FOUND)
    set(CYCLONE_CRYPTO_LIBRARIES ${CYCLONE_CRYPTO_LIBRARY} ${CYCLONE_COMMON_LIBRARY})
    set(CYCLONE_CRYPTO_INCLUDE_DIRS ${CYCLONE_CRYPTO_INCLUDE_DIR})
endif()