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
    gen_subscription_event_bs
    
SEES
    constants,
    message_in_bs,
    message_out_bs
    
OPERATIONS    

    gen_data_changed_event (p_prev_value, p_prev_value_status, p_new_value_pointer) =
    PRE
        p_prev_value : t_Variant_i &
        p_prev_value : t_Variant &
        p_prev_value_status : t_StatusCode_i &
        p_prev_value_status : t_StatusCode &
        p_new_value_pointer : t_WriteValuePointer_i &
        p_new_value_pointer : t_WriteValuePointer
    THEN
        /* Generates a data changed event */
        skip
    END

END
