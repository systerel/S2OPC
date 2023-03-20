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

void SOPC_PubScheduler_Stop(void);

#endif /* SOPC_PUB_SCHEDULER_H_ */
