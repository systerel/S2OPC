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

MACHINE
    write_value_pointer_bs

SEES
    constants

OPERATIONS
    
    is_valid <-- write_value_pointer_is_valid (p_write_value) =
    PRE
        p_write_value : t_WriteValuePointer_i
    THEN
        is_valid := bool(p_write_value : t_WriteValuePointer)
    END
    ;
    
    p_nid, p_aid <-- get_write_value_pointer_NodeId_AttributeId (p_write_value) =
    PRE
        p_write_value : t_WriteValuePointer_i &
        p_write_value : t_WriteValuePointer
    THEN
        p_nid, p_aid
        :(p_nid : t_NodeId_i & p_nid : t_NodeId &
            p_aid : t_AttributeId_i & p_aid : t_AttributeId)
    END
    
END
