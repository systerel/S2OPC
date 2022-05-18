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

#ifndef SOPC_SUB_READER_H_
#define SOPC_SUB_READER_H_

#include "sopc_buffer.h"
#include "sopc_network_layer.h"
#include "sopc_pubsub_conf.h"
#include "sopc_pubsub_security.h"
#include "sopc_sub_target_variable.h"

/**
 * Decode a UADP message and write data
 * \param connection : configuration element of the connection associated to the received data
 * \param buffer : data to decode
 * \param config : configuration to provide to the target module which consumes the decoded data
 * \param securityCBck : function to retrieve security information needed to decrypt Payload and check signature
 */
SOPC_ReturnStatus SOPC_Reader_Read_UADP(const SOPC_PubSubConnection* connection,
                                        SOPC_Buffer* buffer,
                                        SOPC_SubTargetVariableConfig* config,
                                        SOPC_UADP_GetSecurity_Func securityCBck);

/**
 * Return default reception filtering functions.
 */
extern const SOPC_UADP_NetworkMessage_Reader_Callbacks SOPC_Reader_NetworkMessage_Default_Readers;
#endif /* SOPC_SUB_READER_H_ */
