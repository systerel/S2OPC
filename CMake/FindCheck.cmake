# Copyright (C) 2018 Systerel and others.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

	if (SUBUNIT_LIBRARY)
		set(CHECK_LIBRARIES ${CHECK_LIBRARIES} ${SUBUNIT_LIBRARY})
		set(CHECK_INCLUDE_DIRS ${CHECK_INCLUDE_DIRS} ${SUBUNIT_INCLUDE_DIR})
	endif()
endif()

mark_as_advanced(CHECK_INCLUDE_DIRS CHECK_LIBRARY)
