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

/** \file
 *
 * Generating the Address Space content. The provided arrays must be allocated with constants__t_Node_i_max elements.
 */

#ifndef _gen_addspace_h
#define _gen_addspace_h


#include "constants.h"

#include "sopc_base_types.h"
#include "sopc_types.h"


/**
 * Fills the pre-alocated arrays with the address space.
 */
SOPC_StatusCode gen_addspace(constants__t_NodeId_i     *pnids,
                             constants__t_NodeClass_i  *pncls,
                             constants__t_Variant_i    *pvars,
                             constants__t_StatusCode_i *pscs);

/**
 * Frees the alocated values generated with gen_addspace.
 * Does not free the arrays.
 */
SOPC_StatusCode free_addspace(constants__t_NodeId_i     *pnids,
                              constants__t_NodeClass_i  *pncls,
                              constants__t_Variant_i    *pvars,
                              constants__t_StatusCode_i *pscs);

/* Helpers, which shows that this code is ill-organised */
#ifndef NB_NODES
#define NB_NODES    constants__t_Node_i_max
#endif


#endif /* _gen_addspace_h */
