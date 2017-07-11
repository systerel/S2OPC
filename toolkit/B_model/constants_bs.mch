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

MACHINE
    constants_bs

SETS
    t_Node_i;
    t_NodeId_i;
    t_Variant_i;
    t_ByteString_i;
    t_UserId_i;

    t_channel_i;
    t_session_i;
    t_session_token_i;
    t_user_i;
    t_msg_i;
    t_request_handle_i;
    t_endpoint_i

CONCRETE_CONSTANTS
    k_n_nodes_max,

    c_Variant_indet,
    c_Node_indet,
    c_NodeId_indet,
    c_ByteString_indet,
    c_UserId_indet,

    c_channel_indet,
    c_user_indet,
    c_session_indet,
    c_session_token_indet,
    c_msg_indet,
    c_endpoint_indet,
    c_request_handle_indet

ABSTRACT_CONSTANTS
    t_Variant,
    t_Node,
    t_NodeId,
    t_ByteString,
    t_UserId,

    t_channel,
    t_session,
    t_session_token,
    t_user,
    t_msg,
    t_endpoint,
    t_request_handle

PROPERTIES
    k_n_nodes_max : NAT &

    t_Variant <: t_Variant_i &
    c_Variant_indet : t_Variant_i &
    c_Variant_indet /: t_Variant &

    t_Node <: t_Node_i &
    c_Node_indet : t_Node_i &
    c_Node_indet /: t_Node &

    t_NodeId <: t_NodeId_i &
    c_NodeId_indet : t_NodeId_i &
    c_NodeId_indet /: t_NodeId &

    t_ByteString <: t_ByteString_i &
    c_ByteString_indet : t_ByteString_i &
    c_ByteString_indet /: t_ByteString &

    t_UserId <: t_UserId_i &
    c_UserId_indet : t_UserId_i &
    c_UserId_indet /: t_UserId &

    t_channel <: t_channel_i &
    c_channel_indet : t_channel_i &
    c_channel_indet /: t_channel &

    t_session <: t_session_i &
    c_session_indet : t_session_i &
    c_session_indet /: t_session &

    t_session_token <: t_session_token_i &
    c_session_token_indet : t_session_token_i &
    c_session_token_indet /: t_session_token &

    t_user <: t_user_i &
    c_user_indet : t_user_i &
    c_user_indet /: t_user &

    t_msg <: t_msg_i &
    c_msg_indet : t_msg_i &
    c_msg_indet /: t_msg &

    t_request_handle <: t_request_handle_i &
    c_request_handle_indet : t_request_handle_i &
    c_request_handle_indet /: t_request_handle &

    t_endpoint <: t_endpoint_i &
    c_endpoint_indet : t_endpoint_i &
    c_endpoint_indet /: t_endpoint

END
