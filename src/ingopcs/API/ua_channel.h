/*
 * ua_channel.h
 *
 *  Created on: Sep 29, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SOPC_CHANNEL_H_
#define INGOPCS_SOPC_CHANNEL_H_

#include <ua_stack_csts.h>

#ifdef OPCUA_HAVE_CLIENTAPI

#include <ua_types.h>
#include "pki.h"
#include "key_manager.h"

typedef void* SOPC_Channel;

typedef enum {
    ChannelSerializer_Invalid = 0x00,
    ChannelSerializer_Binary = 0x01
} SOPC_Channel_SerializerType;

typedef enum {
    ChannelEvent_Invalid = 0x00,
    ChannelEvent_Connected = 0x01,
    ChannelEvent_Disconnected = 0x02
} SOPC_Channel_Event;

typedef SOPC_StatusCode (SOPC_Channel_PfnConnectionStateChanged) (SOPC_Channel       channel,
                                                           void*            cbData,
                                                           SOPC_Channel_Event cEvent,
                                                           SOPC_StatusCode       status);

typedef SOPC_StatusCode (SOPC_Channel_PfnRequestComplete)(SOPC_Channel         channel,
                                                   void*              response,
                                                   SOPC_EncodeableType* responseType,
                                                   void*              cbData,
                                                   SOPC_StatusCode         status);

//TODO: API indicates namespace too but it is not the case in 1.03 foundation stack
SOPC_StatusCode SOPC_Channel_Create(SOPC_Channel*               channel,
                             SOPC_Channel_SerializerType serialType);
SOPC_StatusCode SOPC_Channel_Clear(SOPC_Channel channel);
SOPC_StatusCode SOPC_Channel_Delete(SOPC_Channel* channel);

SOPC_StatusCode SOPC_Channel_BeginConnect(SOPC_Channel                            channel,
                                   const char*                           url,
                                   const Certificate*                    crt_cli,
                                   const AsymmetricKey*                  key_priv,
                                   const Certificate*                    crt_srv,
                                   const PKIProvider*                    pki,
                                   const char*                           reqSecuPolicyUri,
                                   int32_t                               requestedLifetime,
                                   OpcUa_MessageSecurityMode             msgSecurityMode,
                                   uint32_t                              networkTimeout,
                                   SOPC_Channel_PfnConnectionStateChanged* cb,
                                   void*                                 cbData);

SOPC_StatusCode SOPC_Channel_BeginInvokeService(SOPC_Channel                     channel,
                                         char*                          debugName,
                                         void*                          request,
                                         SOPC_EncodeableType*             requestType,
                                         SOPC_EncodeableType*             responseType, // NOT IN API ! => efficiency
                                         SOPC_Channel_PfnRequestComplete* cb,
                                         void*                          cbData);

SOPC_StatusCode SOPC_Channel_InvokeService(SOPC_Channel          channel,
                                    char*               debugName,
                                    void*               request,
                                    SOPC_EncodeableType*  requestType,
                                    SOPC_EncodeableType*  expResponseType,
                                    void**              response,
                                    SOPC_EncodeableType** responseType);

SOPC_StatusCode SOPC_Channel_Disconnect(SOPC_Channel channel);

#endif /* CLIENT_API */

#endif /* INGOPCS_SOPC_CHANNEL_H_ */
