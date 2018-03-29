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

 File Name            : channel_mgr_it.c

 Date                 : 29/03/2018 14:46:08

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "channel_mgr_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 channel_mgr_it__channel_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void channel_mgr_it__INITIALISATION(void)
{
    channel_mgr_it__channel_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void channel_mgr_it__init_iter_channel(t_bool* const channel_mgr_it__p_continue)
{
    constants__get_card_t_channel(&channel_mgr_it__channel_i);
    *channel_mgr_it__p_continue = (1 <= channel_mgr_it__channel_i);
}

void channel_mgr_it__continue_iter_channel(t_bool* const channel_mgr_it__p_continue,
                                           constants__t_channel_i* const channel_mgr_it__p_channel)
{
    constants__get_cast_t_channel(channel_mgr_it__channel_i, channel_mgr_it__p_channel);
    channel_mgr_it__channel_i = channel_mgr_it__channel_i - 1;
    *channel_mgr_it__p_continue = (1 <= channel_mgr_it__channel_i);
}
