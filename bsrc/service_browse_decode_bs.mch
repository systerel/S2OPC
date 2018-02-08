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
