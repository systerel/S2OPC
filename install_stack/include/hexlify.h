/** \file
 *
 * \brief Helpers for tests.
 *
 */
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


#ifndef SOPC_HEXLIFY_H_
#define SOPC_HEXLIFY_H_


#include <stddef.h> // size_t


int hexlify(const unsigned char *src, char *dst, size_t n);
int unhexlify(const char *src, unsigned char *dst, size_t n);


#endif /* SOPC_HEXLIFY_H_ */
