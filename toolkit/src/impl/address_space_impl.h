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
 * Defines the structure behind the AddressSpace implementation.
 * These structs and constants bridges C and B models.
 */


#ifndef address_space_impl_h_
#define address_space_impl_h_


/* AttributeIds: they are #defined by the SDK... */
typedef enum {
    c_aid_indet = 0,
    e_aid_NodeId,
    e_aid_NodeClass,
    e_aid_Value = 13,
    e_aid_UserExecutable = 22
} AttributeId;

#define e_aid_min e_aid_NodeId
#define e_aid_max e_aid_UserExecutable


#endif // address_space_impl_h_
