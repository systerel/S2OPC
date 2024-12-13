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

#ifndef SOPC_PUB_SCHEDULER_H_
#define SOPC_PUB_SCHEDULER_H_

#include "sopc_pub_source_variable.h"
#include "sopc_pubsub_conf.h"

/**
 * \brief Starts the Publisher scheduler using the PubSub configuration data
 *        and the source of data information
 *
 * \param config            The PubSub configuration to be used to start the Publisher
 * \param sourceConfig      The source configured to retrieve up to date data referenced in \p config
 * \param threadPriority    Under linux, the publisher wakes up more regularly when the thread is created with
 *                          a higher priority, but this requires administrative rights.
 *                          This value must be 0 (thread created with usual priority) or 1 to 99
 *                          (thread created with FIFO scheduling policy requiring administrative rights)
 */
bool SOPC_PubScheduler_Start(SOPC_PubSubConfiguration* config,
                             SOPC_PubSourceVariableConfig* sourceConfig,
                             int threadPriority);

/**
 * @brief Launch sending process in acyclic mode. Content of the message can be modified in callback \p
 * SOPC_GetSourceVariables_Func set in function ::SOPC_PubSourceVariableConfig_Create. To select the content to be sent
 * \p publisherId and \p writerGroupId shall match the Publisher Id and WriterGroup Id of network message set in
 * configuration
 *
 * @param publisherId Publisher Id to identify the network message
 * @param writerGroupId Id of the writer group to send
 * @return true if succeed to match a writer group to send and send it
 * @return false no \p publisherId or \p writerGroupId match with configuration don't send anything
 */
bool SOPC_PubScheduler_AcyclicSend(SOPC_Conf_PublisherId* publisherId, uint16_t writerGroupId);

/**
 * @brief Enable the emission of a dataSetMessage identified by the tuple [pubId, writerGroupId, dataSetWriterId].
 *
 * @param pubId pointer to Publisher Id to identify the dataSetMessage. Must NOT be NULL
 * @param writerGroupId Writer Group Id to identify the dataSetMessage
 * @param dataSetWriterId DataSet Writer Id to identify the dataSetMessage
 * @return SOPC_STATUS_OK in case of success or if the DataSetMessage was already enabled.
 * SOPC_STATUS_NOK if no dataSetMessage was found with tuple [pubId, writerGroupId, dataSetWriterId].
 * SOPC_STATUS_INVALID_PARAMETERS in case of wrong parameter.
 */
SOPC_ReturnStatus SOPC_PubScheduler_Enable_DataSetMessage(SOPC_Conf_PublisherId* pubId,
                                                          uint16_t writerGroupId,
                                                          uint16_t dataSetWriterId);

/**
 * @brief Disable the emission of a dataSetMessage identified by the tuple [pubId, writerGroupId, dataSetWriterId].
 *
 * @param pubId pointer to Publisher Id to identify the dataSetMessage. Must NOT be NULL
 * @param writerGroupId Writer Group Id to identify the dataSetMessage
 * @param dataSetWriterId DataSet Writer Id to identify the dataSetMessage
 * @return SOPC_STATUS_OK in case of success or if the DataSetMessage was already disabled.
 * SOPC_STATUS_NOK if no dataSetMessage was found with tuple [pubId, writerGroupId, dataSetWriterId].
 * SOPC_STATUS_INVALID_PARAMETERS in case of wrong parameter.
 */
SOPC_ReturnStatus SOPC_PubScheduler_Disable_DataSetMessage(SOPC_Conf_PublisherId* pubId,
                                                           uint16_t writerGroupId,
                                                           uint16_t dataSetWriterId);

void SOPC_PubScheduler_Stop(void);

#endif /* SOPC_PUB_SCHEDULER_H_ */
