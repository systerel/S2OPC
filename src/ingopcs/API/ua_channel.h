/*
 * ua_channel.h
 *
 *  Created on: Sep 29, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_UA_CHANNEL_H_
#define INGOPCS_UA_CHANNEL_H_

#include <ua_stack_csts.h>

#ifdef OPCUA_HAVE_CLIENTAPI

#include <ua_types.h>
#include "pki.h"
#include "key_manager.h"

typedef void* UA_Channel;

typedef enum {
    ChannelSerializer_Invalid = 0x00,
    ChannelSerializer_Binary = 0x01
} UA_Channel_SerializerType;

typedef enum {
    ChannelEvent_Invalid = 0x00,
    ChannelEvent_Connected = 0x01,
    ChannelEvent_Disconnected = 0x02
} UA_Channel_Event;

typedef StatusCode (UA_Channel_PfnConnectionStateChanged) (UA_Channel       channel,
                                                           void*            cbData,
                                                           UA_Channel_Event cEvent,
                                                           StatusCode       status);

typedef StatusCode (UA_Channel_PfnRequestComplete)(UA_Channel         channel,
                                                   void*              response,
                                                   UA_EncodeableType* responseType,
                                                   void*              cbData,
                                                   StatusCode         status);

//TODO: API indicates namespace too but it is not the case in 1.03 foundation stack
StatusCode UA_Channel_Create(UA_Channel*               channel,
                             UA_Channel_SerializerType serialType);
StatusCode UA_Channel_Clear(UA_Channel channel);
StatusCode UA_Channel_Delete(UA_Channel* channel);

StatusCode UA_Channel_BeginConnect(UA_Channel                            channel,
                                   const char*                           url,
                                   const Certificate*                    crt_cli,
                                   const AsymmetricKey*                  key_priv,
                                   const Certificate*                    crt_srv,
                                   const PKIProvider*                    pki,
                                   const char*                           reqSecuPolicyUri,
                                   int32_t                               requestedLifetime,
                                   OpcUa_MessageSecurityMode             msgSecurityMode,
                                   uint32_t                              networkTimeout,
                                   UA_Channel_PfnConnectionStateChanged* cb,
                                   void*                                 cbData);

StatusCode UA_Channel_BeginInvokeService(UA_Channel                     channel,
                                         char*                          debugName,
                                         void*                          request,
                                         UA_EncodeableType*             requestType,
                                         UA_EncodeableType*             responseType, // NOT IN API ! => efficiency
                                         UA_Channel_PfnRequestComplete* cb,
                                         void*                          cbData);

StatusCode UA_Channel_InvokeService(UA_Channel          channel,
                                    char*               debugName,
                                    void*               request,
                                    UA_EncodeableType*  requestType,
                                    UA_EncodeableType*  expResponseType,
                                    void**              response,
                                    UA_EncodeableType** responseType);

StatusCode UA_Channel_Disconnect(UA_Channel channel);

#endif /* CLIENT_API */

#endif /* INGOPCS_UA_CHANNEL_H_ */
