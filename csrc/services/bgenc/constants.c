/*
 *  Copyright (C) 2018 Systerel and others.
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

/******************************************************************************

 File Name            : constants.c

 Date                 : 29/03/2018 14:46:08

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void constants__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void constants__read_cast_t_ReadValue(const t_entier4 constants__ii, constants__t_ReadValue_i* const constants__rvi)
{
    *constants__rvi = constants__ii;
}

void constants__get_cast_t_WriteValue(const t_entier4 constants__ii, constants__t_WriteValue_i* const constants__wvi)
{
    *constants__wvi = constants__ii;
}

void constants__get_cast_t_BrowseValue(const t_entier4 constants__p_ind,
                                       constants__t_BrowseValue_i* const constants__p_bvi)
{
    *constants__p_bvi = constants__p_ind;
}

void constants__get_cast_t_BrowseResult(const t_entier4 constants__p_ind,
                                        constants__t_BrowseResult_i* const constants__p_bri)
{
    *constants__p_bri = constants__p_ind;
}

void constants__get_Is_Dir_Forward_Compatible(const constants__t_BrowseDirection_i constants__p_dir,
                                              const t_bool constants__p_IsForward,
                                              t_bool* const constants__p_dir_compat)
{
    switch (constants__p_dir)
    {
    case constants__e_bd_forward:
        *constants__p_dir_compat = constants__p_IsForward;
        break;
    case constants__e_bd_inverse:
        *constants__p_dir_compat = (constants__p_IsForward == false);
        break;
    default:
        *constants__p_dir_compat = true;
        break;
    }
}
