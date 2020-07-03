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

#ifndef SOPC_NETWORK_LAYER_H_
#define SOPC_NETWORK_LAYER_H_

#include "sopc_buffer.h"
#include "sopc_dataset_ll_layer.h"
#include "sopc_pubsub_security.h"

// TODO: use security mode instead of security enabled
//
typedef struct SOPC_UADP_Network_Message
{
    SOPC_Dataset_LL_NetworkMessage* nm;
    SOPC_UADP_Configuration conf;
} SOPC_UADP_NetworkMessage;

/**
 * \brief Encode a NetworkMessage with UADP Mapping
 *
 * \param nm is the NetworkMessage to encode
 * \param securityCtx is the data use to encrypt and sign. Can be NULL if security is not used
 *
 * \return A buffer containing the UADP bytes or NULL if the operation does not successed
 *
 */
SOPC_Buffer* SOPC_UADP_NetworkMessage_Encode(SOPC_Dataset_LL_NetworkMessage* nm,     //
                                             SOPC_PubSub_SecurityType* securityCtx); //

/**
 * \brief Decode a UADP packet into a NetworkMessage
 *
 * \param buffer the UADP byte to decode
 * \param getSecurity_Func is a callback to retrieve Security information related to the security token and publisher of
 * the message
 *
 * \return A NetworkMessage corresponding to the UADP packet or NULL if the operation does not successed
 *
 */
SOPC_UADP_NetworkMessage* SOPC_UADP_NetworkMessage_Decode(SOPC_Buffer* buffer,
                                                          SOPC_UADP_GetSecurity_Func getSecurity_Func);

void SOPC_UADP_NetworkMessage_Delete(SOPC_UADP_NetworkMessage* uadp_nm);

#endif /* SOPC_NETWORK_LAYER_H_ */
