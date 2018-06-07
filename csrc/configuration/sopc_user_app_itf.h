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
    const SOPC_Buffer* crt_cli;
    const SOPC_Buffer* key_priv_cli;
    const SOPC_Buffer* crt_srv;
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
    SOPC_Buffer* serverCertificate;
    SOPC_Buffer* serverKey;
    SOPC_PKIProvider* pki;
    uint8_t nbSecuConfigs;
    SOPC_SecurityPolicy* secuConfigurations;
    OpcUa_ApplicationDescription serverDescription;
} SOPC_Endpoint_Config;

/* Client and Server communication events to be managed by applicative code*/
typedef enum SOPC_App_Com_Event {
    /* Client application events */
    SE_SESSION_ACTIVATION_FAILURE, /* id = internal session id (or 0 if not yet defined)
                                      auxParam = user application session context
                                   */
    SE_ACTIVATED_SESSION,          /* id = internal session id
                                      auxParam = user application session context
                                   */
    SE_SESSION_REACTIVATING,       /* automatic new SC or manual new user on same SC */
                                   /* id = internal session id
                                      auxParam = user application session context
                                   */
    SE_RCV_SESSION_RESPONSE,       /* id = internal session id
                                      params = (OpcUa_<MessageStruct>*) OPC UA message header + payload structure
                                      (deallocated by toolkit after callback call is terminated)
                                      auxParam = user application request context
                                   */
    SE_CLOSED_SESSION,             /* id = internal session id
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
