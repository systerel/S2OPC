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
    browse_treatment_context

SEES
    constants,
    constants_statuscodes_bs

DEFINITIONS

    d_variables_in ==
        in_startIndex,
        in_session,
        in_maxReferencesPerNode,
        in_BrowseView,
        in_BrowseValue_NodeId,
        in_BrowseValue_BrowseDirection,
        in_BrowseValue_ReferenceTypeId,
        in_BrowseValue_IncludeSubtypes,
        in_BrowseValue_NodeClassMask,
        in_BrowseValue_NodeClassesInMask,
        in_BrowseValue_ResultMask,
        in_ReleasePrevContinuationPoint,
        isBrowseValueContextDefined;

    d_inv_in ==
        in_startIndex                    : NAT                   &
        in_startIndex                    : t_RefIndex            &
        in_session                       : t_session_i           &
        in_maxReferencesPerNode          : NAT                   &
        in_BrowseView                    : t_NodeId_i            &
        in_BrowseValue_NodeId            : t_NodeId_i            &
        in_BrowseValue_BrowseDirection   : t_BrowseDirection_i   &
        in_BrowseValue_ReferenceTypeId   : t_NodeId_i            &
        in_BrowseValue_IncludeSubtypes   : BOOL                  &
        in_BrowseValue_NodeClassMask     : t_BrowseNodeClassMask_i &
        in_BrowseValue_NodeClassesInMask <: t_NodeClass_i         &
        in_BrowseValue_ResultMask        : t_BrowseResultMask_i &
        in_ReleasePrevContinuationPoint  : BOOL & /* Automatic release of previous continuation flag */
        isBrowseValueContextDefined      : BOOL &
        (isBrowseValueContextDefined = TRUE =>
            in_BrowseValue_BrowseDirection : t_BrowseDirection &
            in_BrowseValue_NodeId : t_NodeId)
    ;
    d_init_in ==
        d_variables_in
        :(
            in_startIndex                   = 0                          &
            in_session                      = c_session_indet            &
            in_maxReferencesPerNode         = 0                          &
            in_BrowseView                   = c_NodeId_indet             &
            in_BrowseValue_NodeId           = c_NodeId_indet             &
            in_BrowseValue_BrowseDirection  = e_bd_indet                 &
            in_BrowseValue_ReferenceTypeId  = c_NodeId_indet             &
            in_BrowseValue_IncludeSubtypes  = FALSE                      &
            in_BrowseValue_NodeClassMask    = c_BrowseNodeClassMask_indet &
            in_BrowseValue_NodeClassesInMask = {c_NodeClass_indet}       &
            in_BrowseValue_ResultMask       = c_BrowseResultMask_indet &
            in_ReleasePrevContinuationPoint = FALSE &
            isBrowseValueContextDefined     = FALSE
        )

ABSTRACT_VARIABLES
    d_variables_in

INVARIANT
    d_inv_in

INITIALISATION
    d_init_in

OPERATIONS

    p_service_StatusCode <-- setall_browse_value_context (p_startIndex, p_session, p_maxTargetRef,
                                                          p_browseView, p_nodeId, p_browseDirection,
                                                          p_referenceType, p_includeSubtypes,
                                                          p_nodeClassMask, p_resultMask, p_autoReleaseCP) =
   PRE
        isBrowseValueContextDefined = FALSE &
        p_startIndex    : NAT &
        p_startIndex    : t_RefIndex &
        p_session : t_session_i                     &
        p_maxTargetRef : NAT                        &
        p_maxTargetRef <= k_n_BrowseTarget_max      &
        p_browseView : t_NodeId_i                   &
        p_nodeId : t_NodeId_i                       &
        p_nodeId : t_NodeId                         &
        p_browseDirection : t_BrowseDirection_i     &
        p_browseDirection : t_BrowseDirection       &
        p_referenceType : t_NodeId_i                &
        p_nodeClassMask : t_BrowseNodeClassMask_i   &
        p_resultMask    : t_BrowseResultMask_i   &
        p_autoReleaseCP : BOOL &
        p_includeSubtypes : BOOL
   THEN
        p_service_StatusCode,
        d_variables_in
        :(
            p_service_StatusCode : {e_sc_ok, e_sc_bad_out_of_memory} &
            d_inv_in &
            (p_service_StatusCode = e_sc_ok =>
                in_startIndex                    = p_startIndex      &
                in_session                       = p_session         &
                in_maxReferencesPerNode          = p_maxTargetRef    &
                in_BrowseView                    = p_browseView      &
                in_BrowseValue_NodeId            = p_nodeId          &
                in_BrowseValue_BrowseDirection   = p_browseDirection &
                in_BrowseValue_ReferenceTypeId   = p_referenceType   &
                in_BrowseValue_IncludeSubtypes   = p_includeSubtypes &
                in_BrowseValue_NodeClassMask     = p_nodeClassMask   &
                in_BrowseValue_NodeClassesInMask <: t_NodeClass_i &
                in_BrowseValue_NodeClassesInMask$0 <: in_BrowseValue_NodeClassesInMask &
                in_BrowseValue_ResultMask        = p_resultMask   &
                in_ReleasePrevContinuationPoint  = p_autoReleaseCP &

                isBrowseValueContextDefined = TRUE) &
            (p_service_StatusCode /= e_sc_ok =>
                isBrowseValueContextDefined = FALSE)
        )
   END
   ;

   p_startIndex, p_session, p_maxTargetRef, p_browseView,
   p_nodeId, p_browseDirection, p_refType_defined, p_referenceType,
   p_includeSubtypes, p_nodeClassMask, p_resultMask, p_autoReleaseCP
      <-- getall_browse_value_context =
   PRE
       isBrowseValueContextDefined = TRUE
   THEN
        p_startIndex := in_startIndex                        ||
        p_session := in_session                              ||
        p_maxTargetRef := in_maxReferencesPerNode            ||
        p_browseView := in_BrowseView                        ||
        p_nodeId := in_BrowseValue_NodeId                    ||
        p_browseDirection := in_BrowseValue_BrowseDirection  ||
        p_refType_defined := bool(in_BrowseValue_ReferenceTypeId : t_NodeId) ||
        p_referenceType := in_BrowseValue_ReferenceTypeId    ||
        p_includeSubtypes := in_BrowseValue_IncludeSubtypes ||
        p_nodeClassMask := in_BrowseValue_NodeClassMask ||
        p_resultMask := in_BrowseValue_ResultMask ||
        p_autoReleaseCP := in_ReleasePrevContinuationPoint
   END
   ;

   bres <-- is_NodeClass_in_NodeClassMask(p_nodeClass) =
   PRE
        p_nodeClass : t_NodeClass_i &
        p_nodeClass : t_NodeClass
   THEN
        bres := bool(p_nodeClass : in_BrowseValue_NodeClassesInMask)
   END
   ;

   clear_browse_value_context =
   BEGIN
       d_init_in
   END

END
