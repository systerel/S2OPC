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

/*------------------------
   Exported Declarations
  ------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "session_core_orphaned_it_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_orphaned_it_bs__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_core_orphaned_it_bs__init_iter_orphaned_t_session(
   const constants__t_channel_i session_core_orphaned_it_bs__lost_channel,
   t_bool * const session_core_orphaned_it_bs__continue) {
  printf("session_core_orphaned_it_bs__init_iter_orphaned_t_session\n");
  exit(1);
}

void session_core_orphaned_it_bs__continue_iter_orphaned_t_session(
   constants__t_session_i * const session_core_orphaned_it_bs__session,
   t_bool * const session_core_orphaned_it_bs__continue) {
  printf("session_core_orphaned_it_bs__continue_iter_orphaned_t_session\n");
  exit(1);
}

