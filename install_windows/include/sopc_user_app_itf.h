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

/** \file
 *
 * \brief Contains the types to be used by the user application to use the Toolkit
 *
 */

#ifndef SOPC_USER_APP_ITF_H_
#define SOPC_USER_APP_ITF_H_

#include <stdbool.h>

#include "sopc_key_manager.h"
#include "sopc_types.h"

/* Client static configuration of a Secure Channel */
typedef struct SOPC_SecureChannel_Config
{
    uint8_t isClientSc;
    const char* url;
    const SOPC_Certificate* crt_cli;
    const SOPC_AsymmetricKey* key_priv_cli;
    const SOPC_Certificate* crt_srv;
    const SOPC_PKIProvider* pki;
    const char* reqSecuPolicyUri;
    uint32_t requestedLifetime;
    OpcUa_MessageSecurityMode msgSecurityMode;
} SOPC_SecureChannel_Config;

#define SOPC_SECURITY_MODE_NONE_MASK 0x01
#define SOPC_SECURITY_MODE_SIGN_MASK 0x02
#define SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK 0x04
#define SOPC_SECURITY_MODE_ANY_MASK 0x07

typedef struct SOPC_SecurityPolicy
{
    SOPC_String securityPolicy; /**< Security policy URI supported */
    uint16_t securityModes; /**< Mask of security modes supported (use combination of SECURITY_MODE_*_MASK values) */
    void* padding;          /**< Binary compatibility */
} SOPC_SecurityPolicy;

/* Server static configuration of a Endpoint listener */
typedef struct SOPC_Endpoint_Config
{
    char* endpointURL;
    SOPC_Certificate* serverCertificate;
    SOPC_AsymmetricKey* serverKey;
    SOPC_PKIProvider* pki;
    uint8_t nbSecuConfigs;
    SOPC_SecurityPolicy* secuConfigurations;
    OpcUa_ApplicationDescription serverDescription;
} SOPC_Endpoint_Config;

/* Client and Server communication events to be managed by applicative code*/
typedef enum SOPC_App_Com_Event {
    /* Client application events */
    SE_SESSION_ACTIVATION_FAILURE, /* id = session id (or 0 if not yet defined)
                                      auxParam = user application session context
                                   */
    SE_ACTIVATED_SESSION,          /* id = session id
                                      auxParam = user application session context
                                   */
    SE_SESSION_REACTIVATING,       /* automatic new SC or manual new user on same SC */
                                   /* id = session id
                                      auxParam = user application session context
                                   */
    SE_RCV_SESSION_RESPONSE,       /* id = session id
                                      params = (OpcUa_<MessageStruct>*) OPC UA message header + payload structure
                                      (deallocated by toolkit after callback call is terminated)
                                      auxParam = user application request context
                                   */
    SE_CLOSED_SESSION,             /* id = session id
                                      auxParam = user application session context
                                    */
    SE_RCV_DISCOVERY_RESPONSE, /* params = (OpcUa_<MessageStruct>*) OPC UA discovery message header + payload structure
                                  (deallocated by toolkit after callback call is terminated)
                                  auxParam = user application request context
                                */

    SE_SND_REQUEST_FAILED, /* idOrStatus = (SOPC_ReturnStatus) status,
                              params = (SOPC_EncodeableType*) request type (shall not be deallocated)
                              auxParam = user application request context
                            */

    /* Server application events */
    SE_CLOSED_ENDPOINT,       /* id = endpoint configuration index,
                                 auxParam = SOPC_ReturnStatus
                              */
    SE_LOCAL_SERVICE_RESPONSE /* id = endpoint configuration index,
                                 params = (OpcUa_<MessageStruct>*) OPC UA message header + payload structure
                                 (deallocated by toolkit after callback call is terminated)
                                 auxParam = user application request context
                               */
} SOPC_App_Com_Event;

/* Server only interfaces */

/* Address space structure */
typedef struct SOPC_AddressSpace
{
    const uint32_t nbVariables;
    const uint32_t nbVariableTypes;
    const uint32_t nbObjectTypes;
    const uint32_t nbReferenceTypes;
    const uint32_t nbDataTypes;
    const uint32_t nbMethods;
    const uint32_t nbObjects;
    const uint32_t nbViews;
    const uint32_t nbNodesTotal; /* Sum of precedent numbers */

    /* Note: node index is valid for [1, nbNodesTotal], index 0 is used for invalid node */
    /* Note 2: nodes shall be provided by node class and with a predefined order on their node class:
       View, Object, Variable, VariableType, ObjectType, ReferenceType, DataType, Method */

    SOPC_QualifiedName* browseNameArray; /* Browse name by node index */

    int* descriptionIdxArray_begin;       /* Given node index, provides the start index in descriptionArray*/
    int* descriptionIdxArray_end;         /* Given node index, provides the end index in descriptionArray*/
    SOPC_LocalizedText* descriptionArray; /* Given description index, provides the localized text */

    int* displayNameIdxArray_begin;       /* Given node index, provides the start index in displayNameArray*/
    int* displayNameIdxArray_end;         /* Given node index, provides the end index in displayNameArray*/
    SOPC_LocalizedText* displayNameArray; /* Given displayName index, provides the localized text */

    OpcUa_NodeClass* nodeClassArray; /* All nodes classes by node index */

    SOPC_NodeId** nodeIdArray; /* All nodes Ids by node index */

    int* referenceIdxArray_begin;     /* Given node index, provides the start reference index*/
    int* referenceIdxArray_end;       /* Given node index, provides the end in reference index */
    SOPC_NodeId** referenceTypeArray; /* Given reference index, provides the reference type node Id */
    SOPC_ExpandedNodeId**
        referenceTargetArray;      /* Given reference index, provides the reference target expended node Id */
    bool* referenceIsForwardArray; /* Given reference index, provides the reference isForward flag value */

    SOPC_Variant* valueArray; /* Given (node index - nbViews - nbObjects), provides the variable/variableType value */
    SOPC_StatusCode*
        valueStatusArray; /* Given (node index - nbViews - nbObjects), provides the variable/variableType value */

    SOPC_SByte* accessLevelArray; /* Given (node index - nbViews - nbObjects), provides the variable access level */

} SOPC_AddressSpace;

/* Server address space access/modification notifications to applicative code */
typedef enum SOPC_App_AddSpace_Event {
    /* Server application events */
    AS_WRITE_EVENT, /* opParam = (OpcUa_WriteValue*) single write value operation
                       opStatus = status of the write operation
                     */
} SOPC_App_AddSpace_Event;

/* Toolkit communication events application callback type */
typedef void SOPC_ComEvent_Fct(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t appContext);

/* Toolkit address space notification events callback type */
typedef void SOPC_AddressSpaceNotif_Fct(SOPC_App_AddSpace_Event event, void* opParam, SOPC_StatusCode opStatus);

typedef enum {
    SOPC_TOOLKIT_LOG_LEVEL_ERROR,
    SOPC_TOOLKIT_LOG_LEVEL_WARNING,
    SOPC_TOOLKIT_LOG_LEVEL_INFO,
    SOPC_TOOLKIT_LOG_LEVEL_DEBUG
} SOPC_Toolkit_Log_Level;

/* Toolkit build information */
typedef struct
{
    char* toolkitVersion;
    char* toolkitSrcCommit;
    char* toolkitDockerId;
    char* toolkitBuildDate;
} SOPC_Build_Info;

#endif // SOPC_USER_APP_ITF_H_
