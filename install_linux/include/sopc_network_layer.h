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

// TODO: use security mode instead of security enabled
//
typedef struct SOPC_UADP_Configuration
{
    bool PublisherIdFlag;
    bool GroupHeaderFlag;
    bool GroupIdFlag;
    bool GroupVersionFlag;
    bool NetworkMessageNumberFlag;
    bool SequenceNumberFlag;
    bool PayloadHeaderFlag;
    bool TimestampFlag;
    bool PicoSecondsFlag;
    bool DataSetClassIdFlag;
    bool SecurityFlag;
    bool PromotedFieldsFlag;

} SOPC_UADP_Configuration;

typedef struct SOPC_UADP_Network_Message
{
    SOPC_Dataset_LL_NetworkMessage* nm;
    SOPC_UADP_Configuration conf;
} SOPC_UADP_NetworkMessage;

SOPC_Buffer* SOPC_UADP_NetworkMessage_Encode(SOPC_Dataset_LL_NetworkMessage* nm);

SOPC_UADP_NetworkMessage* SOPC_UADP_NetworkMessage_Decode(SOPC_Buffer* buffer);

void SOPC_UADP_NetworkMessage_Delete(SOPC_UADP_NetworkMessage* uadp_nm);

#endif /* SOPC_NETWORK_LAYER_H_ */
