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

#ifndef SOPC_SUB_SCHEDULER_H_
#define SOPC_SUB_SCHEDULER_H_

#include "sopc_pubsub_conf.h"
#include "sopc_sub_target_variable.h"
#include "sopc_pubsub_security.h"

/* TODO: Move me */
typedef enum SOPC_PubSubState
{
    SOPC_PubSubState_Disabled = 0,
    SOPC_PubSubState_Paused = 1,
    SOPC_PubSubState_Operational = 2,
    SOPC_PubSubState_Error = 3
} SOPC_PubSubState;

/* \param state   the new subscriber state */
typedef void SOPC_SubscriberStateChanged_Func(SOPC_PubSubState state);

/**
 * @brief Callback to notify gaps in received DataSetMessage sequence number.
 * DataSetMessage is identified by tuple [pubId,writerId] describe below.
 *
 * @param pubId Publisher Id of the networkMessage
 * @param writerId dataSetWriterId to filter dataSetMessage
 * @param prevSN previous DataSetMessage sequence number received
 * @param receivedSN current DataSetMessage sequence number received
 */
typedef void SOPC_SubscriberDataSetMessageSequenceNumberGap_Func(SOPC_Conf_PublisherId pubId,
                                                                 uint16_t writerId,
                                                                 uint16_t prevSN,
                                                                 uint16_t receivedSN);

/**
 * Search in Subscriber security context the data associated to a token id, publisher id and writer group id
 * If no context is found, it means the subscriber is not configured to manage security from this parameter.
 * This function is given as callback to the Network Message decoder.
 *
 * \param tokenId tokenId of a received message
 * \param pubCtx a context related to a Publisher found in the Subscriber security context
 * \param writerGroupId writer group id of a received message
 * \return security data or NULL if not found
 */
SOPC_PubSub_SecurityType* SOPC_SubScheduler_Get_Security_Infos(uint32_t tokenId,
                                                               const SOPC_Conf_PublisherId pubId,
                                                               uint16_t writerGroupId);

/**
 * @brief Check if sequence number is newer for the given tuple (PublisherId, DataSetWriterId) and
 * received sequence number.
 *
 * @param pubId PublisherId attach to a networkMessage
 * @param writerId DataSetW attach to a dataSetMessage
 * @param receivedSN received sequence number
 * @return true if sequence number received is newer and valid
 * @return false if sequence number received is older or invalid
 */
bool SOPC_SubScheduler_Is_Writer_SN_Newer(const SOPC_Conf_PublisherId* pubId,
                                          const uint16_t writerId,
                                          const uint16_t receivedSN);

/* Only state changed callback can be NULL */
bool SOPC_SubScheduler_Start(SOPC_PubSubConfiguration* config,
                             SOPC_SubTargetVariableConfig* targetConfig,
                             SOPC_SubscriberStateChanged_Func* pStateChangedCb,
                             SOPC_SubscriberDataSetMessageSequenceNumberGap_Func sdmSnGapCb,
                             int threadPriority);

void SOPC_SubScheduler_Stop(void);

void on_mqtt_message_received(uint8_t* data, uint16_t size, void* pInputIdentifier);

#endif /* SOPC_SUB_SCHEDULER_H_ */
