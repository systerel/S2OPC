/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SOPC_HELPER_STRING_H_
#define SOPC_HELPER_STRING_H_

#include <stdlib.h>

/**
 *  \brief Compare 2 string in a case-insensitive manner.
 *  Comparison returns 0 if \p size characters were considered identical
 *  or \p s1 and \p s2 were identical and terminated by a '\0' character.
 *
 *  \param s1    A non null string terminated by '\0' character
 *  \param s2    A non null string terminated by '\0' character
 *  \param size  Maximum number of characters compared for computing result.
 *
 *  \return      0 if string are identical in a case-insensitive way, -1 if s1 < s2 and +1 if s1 > s2
 *              (based on first lower case character value comparison).
 */
int SOPC_strncmp_ignore_case(const char *s1, const char *s2, size_t size);

#endif /* SOPC_HELPER_STRING_H_ */
