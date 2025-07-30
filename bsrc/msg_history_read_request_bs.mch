/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

MACHINE
    msg_history_read_request_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs

ABSTRACT_VARIABLES
    nbNodesToRead,
    ReadRawmodifiedDetails,
    HistReadValueID,
    HistReadValueID_NodeId

INVARIANT
    nbNodesToRead : NAT &
    nbNodesToRead <= k_n_histRead_max &
    ReadRawmodifiedDetails : t_readRawModifiedDetails &
    HistReadValueID : t_historyReadReqIndex +-> t_historyReadValueId &
    HistReadValueID_NodeId : t_historyReadReqIndex +-> t_NodeId

INITIALISATION
    nbNodesToRead := 0 ||
    ReadRawmodifiedDetails :: t_readRawModifiedDetails ||
    HistReadValueID :( HistReadValueID = {}) ||
    HistReadValueID_NodeId :( HistReadValueID_NodeId = {})

OPERATIONS

    p_nb_nodes_to_read <-- get_msg_hist_read_req_nb_nodes_to_read (p_req_msg) =
    PRE
        p_req_msg : t_msg_i  &
        p_req_msg = c_msg_in &
        c_msg_in : t_msg     &
        a_msg_in_type = e_msg_attribute_history_read_req
    THEN
        p_nb_nodes_to_read, nbNodesToRead, HistReadValueID, HistReadValueID_NodeId :(
            p_nb_nodes_to_read : NAT &
            nbNodesToRead : NAT &
            dom(HistReadValueID) = 1..nbNodesToRead &
            dom(HistReadValueID_NodeId) = 1..nbNodesToRead &
            (p_nb_nodes_to_read <= k_n_histRead_max => nbNodesToRead = p_nb_nodes_to_read) &
            (p_nb_nodes_to_read >  k_n_histRead_max => nbNodesToRead = 0)
        )
    END
    ;

    p_sc, p_hist_read_details <-- getall_msg_hist_read_req_read_details (p_req_msg) =
    PRE
        p_req_msg : t_msg_i  &
        p_req_msg = c_msg_in &
        c_msg_in : t_msg     &
        a_msg_in_type = e_msg_attribute_history_read_req
    THEN
        p_sc, p_hist_read_details, ReadRawmodifiedDetails :(
            p_sc : t_StatusCode_i &
            p_sc : {e_sc_ok, e_sc_bad_history_operation_invalid, e_sc_bad_history_operation_unsupported} &
            p_hist_read_details : t_readRawModifiedDetails_i &

            (p_sc = e_sc_ok =>
                p_hist_read_details : t_readRawModifiedDetails_i &
                p_hist_read_details : t_readRawModifiedDetails &
                ReadRawmodifiedDetails = p_hist_read_details ) &
            (p_sc : {e_sc_bad_history_operation_invalid, e_sc_bad_history_operation_unsupported} =>
                p_hist_read_details = c_readRawModifiedDetails_indet &
                ReadRawmodifiedDetails = ReadRawmodifiedDetails$0)
        )
    END
    ;

    p_tsToReturn <-- get_msg_hist_read_req_TSToReturn(p_req_msg) =
    PRE
        p_req_msg : t_msg_i  &
        p_req_msg = c_msg_in &
        c_msg_in : t_msg     &
        a_msg_in_type = e_msg_attribute_history_read_req
    THEN
        p_tsToReturn :( p_tsToReturn : t_TimestampsToReturn_i & p_tsToReturn : t_TimestampsToReturn )
    END
    ;

    p_continuation_point <-- get_msg_hist_read_req_release_CP(p_req_msg) =
    PRE
        p_req_msg : t_msg_i  &
        p_req_msg = c_msg_in &
        c_msg_in : t_msg     &
        a_msg_in_type = e_msg_attribute_history_read_req
    THEN
        p_continuation_point :: BOOL
    END
    ;

    p_sc, p_singleValueId, p_nodeId <-- getall_msg_hist_read_req_singleValueId (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i  &
        p_req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_attribute_history_read_req &
        p_index : NAT &
        p_index : t_historyReadReqIndex &
        p_index <= nbNodesToRead
    THEN
        p_singleValueId, p_sc, p_nodeId :(
            p_singleValueId : t_historyReadValueId_i &
            p_sc : {e_sc_ok, e_sc_bad_data_encoding_invalid, e_sc_bad_data_encoding_unsupported,
                    e_sc_bad_index_range_invalid, e_sc_bad_index_range_no_data, e_sc_bad_history_operation_unsupported,
                    e_sc_bad_node_id_unknown} &
            p_nodeId : t_NodeId_i &
            (p_sc = e_sc_ok => HistReadValueID(p_index) = p_singleValueId &
                               p_nodeId : t_NodeId &
                               HistReadValueID_NodeId(p_index) = p_nodeId) &
            (p_sc /= e_sc_ok => p_nodeId = c_NodeId_indet)
        )
    END

END
