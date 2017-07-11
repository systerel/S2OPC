/*
 *  \file sopc_sc_events.h
 *
 *  \brief Event orientied API of the Secure Channel communication layer
 */
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

#ifndef _SOPC_SC_EVENTS_H_
#define _SOPC_SC_EVENTS_H_

#include <assert.h>
#include "key_manager.h"
#include "sopc_types.h"
#include "sopc_endpoint.h"
#include "sopc_event_dispatcher_manager.h"

extern SOPC_EventDispatcherManager* scEventDispatcherMgr;

// TODO:move config types in toolkit config

/* Client static configuration of a Secure Channel */
typedef struct SOPC_SecureChannel_Config {
    const uint8_t             isClientSc;
    const char*               url;
    const Certificate*        crt_cli;
    const AsymmetricKey*      key_priv_cli;
    const Certificate*        crt_srv;
    const PKIProvider*        pki;
    const char*               reqSecuPolicyUri;
    int32_t                   requestedLifetime;
    OpcUa_MessageSecurityMode msgSecurityMode;
} SOPC_SecureChannel_Config;

/* Server static configuration of a Endpoint listener */
typedef struct SOPC_Endpoint_Config{
    char*                endpointURL;
    Certificate*         serverCertificate;
    AsymmetricKey*       serverKey;
    PKIProvider*         pki;
    uint8_t              nbSecuConfigs;
    SOPC_SecurityPolicy* secuConfigurations;
} SOPC_Endpoint_Config;

typedef struct SOPC_SecureChannel_ConnectedConfig {
    uint32_t                   configIdx;
    uint32_t                   connectionId;
    uint32_t                   secureChannelId;
    SOPC_SecureChannel_Config* config;
} SOPC_SecureChannel_ConnectedConfig;

typedef struct SOPC_Toolkit_Msg {
  void* msg;
  SOPC_EncodeableType* encType;
  SOPC_EncodeableType* respEncType;
  uint8_t isRequest;
  void*   optContext;
} SOPC_Toolkit_Msg;

typedef enum SOPC_SC_Event {
  /** SC external events */
  /* Services to SC events */
  EP_OPEN,
  EP_CLOSE,
  SC_CONNECT,
  SC_DISCONNECT,
  EP_SC_SERVICE_SND_MSG, // TMP: needed to differentiate since we do not store SC same way for now
  SC_SERVICE_SND_MSG,
  /* Sockets to SC events */
  SOCKET_CONNECTION,
  SOCKET_FAILURE,
  SOCKET_RCV_BYTES,
  /** SC internal events */
  /* SC mgr to EP mgr */
  EP_SC_DISCONNECTED,
  /* EP mgr to SC mgr */
  EP_SC_CREATE,
  /* SC mgr to Chunks mgr */
  ENCODE_HEL,
  ENCODE_ACK,
  ENCODE_OPN_REQ,
  ENCODE_OPN_RESP,
  ENCODE_CLO_REQ,
  ENCODE_CLO_RESP,
  ENCODE_MSG_CHUNKS,
  /* Chunks mgr to SC mgr */
  SC_RCV_HEL,
  SC_RCV_ACK,
  SC_RCV_OPN_REQ,
  SC_RCV_OPN_RESP,
  SC_RCV_CLO_REQ,
  SC_RCV_CLO_RESP,
  SC_RCV_MSG,
  SC_RCV_FAILURE,
  SC_ENCODE_FAILURE

} SOPC_SC_Event;

void SOPC_TEMP_InitEventDispMgr(SOPC_EventDispatcherManager* toolkitMgr);

void SOPC_SecureChannelEventDispatcher(int32_t  scEvent,
                                       uint32_t id,
                                       void*    params,
                                       int32_t  auxParam);

#endif /* _SOPC_SC_EVENTS_H_ */
