# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

find_path(CHECK_INCLUDE_DIR check.h)

if (CHECK_USE_STATIC_LIBS)
	set(_check_orig_lib_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})

	if(WIN32)
		list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .lib .a)
	else()
		set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
	endif()
endif()

find_library(CHECK_LIBRARY check)
find_library(CHECK_COMPAT_LIBRARY compat)

# We can't know if check was compiled against subunit, so we try to detect it as
# a non mandatory dependency of check.
# This is only relevant when doing static linking, since else check depends on
# subunit by itself.
find_path(SUBUNIT_INCLUDE_DIR subunit/child.h)

find_library(SUBUNIT_LIBRARY subunit)

if(CHECK_USE_STATIC_LIBS)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${_check_orig_lib_suffixes})
endif()

include(FindPackageHandleStandardArgs)
set(CHECK_FIND_REQUIRED ${Check_FIND_REQUIRED})
find_package_handle_standard_args(CHECK DEFAULT_MSG CHECK_INCLUDE_DIR CHECK_LIBRARY)

if (CHECK_FOUND)
	set(CHECK_LIBRARIES ${CHECK_LIBRARY})
	set(CHECK_INCLUDE_DIRS ${CHECK_INCLUDE_DIR})

	if (CHECK_COMPAT_LIBRARY)
		set(CHECK_LIBRARIES ${CHECK_LIBRARIES} ${CHECK_COMPAT_LIBRARY})
	endif()

	if (SUBUNIT_LIBRARY)
		set(CHECK_LIBRARIES ${CHECK_LIBRARIES} ${SUBUNIT_LIBRARY})
		set(CHECK_INCLUDE_DIRS ${CHECK_INCLUDE_DIRS} ${SUBUNIT_INCLUDE_DIR})
	endif()
endif()

mark_as_advanced(CHECK_INCLUDE_DIRS CHECK_LIBRARY)
