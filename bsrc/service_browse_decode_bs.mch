/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
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
    service_browse_decode_bs

SEES
    constants,
    message_in_bs

DEFINITIONS
    d_variables ==
        nb_BrowseValue,
        nb_BrowseTargetMax, /* requestedMaxReferencesPerNode */
        BrowseView,
        BrowseValue_NodeId,
        BrowseValue_BrowseDirection,
        BrowseValue_ReferenceTypeId,
        BrowseValue_IncludedSubtypes
    ;
    d_inv ==
        nb_BrowseValue                : NAT                                 &
        nb_BrowseValue               <= k_n_BrowseResponse_max              &
        nb_BrowseTargetMax            : NAT                                 &
        BrowseView                    : t_NodeId_i                          &
        BrowseValue_NodeId            : t_BrowseValue +-> t_NodeId          &
        BrowseValue_BrowseDirection   : t_BrowseValue +-> t_BrowseDirection &
        BrowseValue_ReferenceTypeId   : t_BrowseValue +-> t_NodeId          &
        BrowseValue_IncludedSubtypes <: t_BrowseValue
    ;
    d_init ==
        d_variables
        :(
            nb_BrowseValue               = 0  &
            nb_BrowseTargetMax           = 0  &
            BrowseView                   = c_NodeId_indet &
            BrowseValue_NodeId           = {} &
            BrowseValue_BrowseDirection  = {} &
            BrowseValue_ReferenceTypeId  = {} &
            BrowseValue_IncludedSubtypes = {}
         )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_init

OPERATIONS
    StatusCode_service <-- decode_browse_request(req_payload) =
    PRE
        /*req_payload : t_byte_buffer_i &
        req_payload : t_byte_buffer*/
        req_payload : t_msg_i &
        req_payload = c_msg_in
    THEN
        StatusCode_service,
        d_variables
        :(
            StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode &
            d_inv
        )
    END
    ;

    free_browse_request =
    BEGIN
        d_init
    END
    ;

    p_nid_view <-- get_BrowseView =
    BEGIN
        p_nid_view := BrowseView
    END
    ;

    nb_req <-- get_nb_BrowseValue =
    BEGIN
        nb_req := nb_BrowseValue
    END
    ;

    p_nb_BrowseTargetMax <-- get_nb_BrowseTargetMax =
    BEGIN
        p_nb_BrowseTargetMax := nb_BrowseTargetMax
    END
    ;

    p_isvalid, p_NodeId, p_dir, p_isreftype, p_reftype, p_inc_subtype <-- getall_BrowseValue(p_bvi) =
    PRE
        p_bvi : t_BrowseValue_i &
        p_bvi : t_BrowseValue
    THEN
        IF  p_bvi : dom(BrowseValue_NodeId) &
            p_bvi : dom(BrowseValue_BrowseDirection)
        THEN
            p_isvalid := TRUE ||
            p_NodeId,
            p_dir,
            p_isreftype,
            p_reftype,
            p_inc_subtype
            :(  p_NodeId      : t_NodeId_i                                     &
                p_dir         : t_BrowseDirection_i                            &
                p_reftype     : t_NodeId_i                                     &
                p_isreftype   = bool(p_bvi : dom(BrowseValue_ReferenceTypeId)) &
                p_inc_subtype = bool(p_bvi : BrowseValue_IncludedSubtypes)     &

                (p_isreftype = TRUE
                 =>
                 p_reftype = BrowseValue_ReferenceTypeId(p_bvi))
              )
        ELSE
            p_isvalid     := FALSE               ||
            p_NodeId      :: t_NodeId_i          ||
            p_dir         :: t_BrowseDirection_i ||
            p_reftype     :: t_NodeId_i          ||
            p_isreftype   :: BOOL                ||
            p_inc_subtype :: BOOL
        END
    END

END
