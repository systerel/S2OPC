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

#ifndef SOPC_PUB_FIXED_BUFFER_H_
#define SOPC_PUB_FIXED_BUFFER_H_

#include <stdint.h>

#include "sopc_buffer.h"
#include "sopc_dataset_ll_layer.h"
#include "sopc_enums.h"
#include "sopc_pubsub_security.h"

typedef struct SOPC_PubFixedBuffer_Buffer_ctx SOPC_PubFixedBuffer_Buffer_ctx;
typedef struct SOPC_PubFixedBuffer_DataSetField_Position SOPC_PubFixedBuffer_DataSetField_Position;

SOPC_ReturnStatus SOPC_DataSet_LL_NetworkMessage_Create_Preencode_Buffer_ctx(SOPC_Dataset_LL_NetworkMessage* nm);

void SOPC_PubFixedBuffer_Delete_Preencode_Buffer_ctx(SOPC_PubFixedBuffer_Buffer_ctx* preencode);

/* Return pointer to updated preencode buffer stored in preencode. This buffer should'nt be free by user */
SOPC_Buffer* SOPC_PubFixedBuffer_Get_UpdatedBuffers(SOPC_PubFixedBuffer_Buffer_ctx* preencode);

/* Set position of dataSetMessage sequence number in final buffer */
bool SOPC_PubFixedBuffer_Set_DSM_SequenceNumber_Position_At(SOPC_PubFixedBuffer_Buffer_ctx* preencode,
                                                            uint32_t position,
                                                            size_t index);

SOPC_PubFixedBuffer_DataSetField_Position* SOPC_PubFixedBuffer_Get_DataSetField_Position_At(
    SOPC_PubFixedBuffer_Buffer_ctx* preencode,
    size_t index);

/* Set position of dataSetField in final buffer */
void SOPC_PubFixedBuffer_DataSetFieldPosition_Set_Position(SOPC_PubFixedBuffer_DataSetField_Position* dsfPos,
                                                           uint32_t position);
#endif /* SOPC_PUB_FIXED_BUFFER_H_ */
