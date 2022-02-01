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

find_path(XDP_INCLUDE_DIR xdp/libxdp.h)

# redefine CMake behavior for find_library if needed
if(USE_STATIC_XDP_LIB)
  set(_xdp_orig_lib_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})

  if(WIN32)
    list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .lib .a)
  else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
  endif()
endif()

find_library(XDP_LIBRARY xdp)

# restore CMake behavior for find_library
if(USE_STATIC_XDP_LIB)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${_xdp_orig_lib_suffixes})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XDP
  REQUIRED_VARS XDP_INCLUDE_DIR XDP_LIBRARY)

mark_as_advanced(XDP_INCLUDE_DIR XDP_LIBRARY)

if(XDP_FOUND AND NOT TARGET XDP)
    add_library(xdp UNKNOWN IMPORTED)
    set_target_properties(xdp PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${XDP_INCLUDE_DIR}
      IMPORTED_LOCATION "${XDP_LIBRARY}"
      )
    target_link_libraries(xdp INTERFACE bpf)
endif()

if(XDP_FOUND)
  set(XDP_LIBRARIES ${XDP_LIBRARY})
  set(XDP_INCLUDE_DIRS ${XDP_INCLUDE_DIR})
endif()

message(STATUS " ${XDP_LIBRARIES}")
