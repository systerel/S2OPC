/*
 *  Copyright (C) 2017 Systerel and others.
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

#ifndef _b2c_h
#define _b2c_h
#include <stdint.h>
#include <stdbool.h>


/* t_bool */
#ifndef _t_bool_
#define _t_bool_
typedef bool t_bool;
#endif /* t_bool */

/* t_entier4 */
#ifndef _t_entier4_
#define _t_entier4_
typedef int32_t t_entier4;
#endif  /* t_entier4 */

/* MAXINT */
#ifndef MAXINT
#define MAXINT (2147483647)
#endif /* MAXINT */

/* MININT */
#ifndef MININT
#define MININT (-MAXINT)
#endif /* MININT */


#endif /* _b2c_h */
