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

find_path(BPF_INCLUDE_DIR bpf/libbpf.h)

# redefine CMake behavior for find_library if needed
if(USE_STATIC_BPF_LIB)
  set(_bpf_orig_lib_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})

  if(WIN32)
    list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .lib .a)
  else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
  endif()
endif()

find_library(BPF_LIBRARY bpf)

# restore CMake behavior for find_library
if(USE_STATIC_BPF_LIB)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${_bpf_orig_lib_suffixes})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BPF
  REQUIRED_VARS BPF_INCLUDE_DIR BPF_LIBRARY)

mark_as_advanced(BPF_INCLUDE_DIR BPF_LIBRARY)

if(BPF_FOUND AND NOT TARGET BPF)
    add_library(bpf UNKNOWN IMPORTED)
    set_target_properties(bpf PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${BPF_INCLUDE_DIR}
      IMPORTED_LOCATION "${BPF_LIBRARY}"
      )
endif()

if(BPF_FOUND)
  set(BPF_LIBRARIES ${BPF_LIBRARY})
  set(BPF_INCLUDE_DIRS ${BPF_INCLUDE_DIR})
endif()

message(STATUS " ${BPF_LIBRARIES}")
