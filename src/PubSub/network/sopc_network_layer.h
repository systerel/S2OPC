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
 * \return A buffer containing the UADP bytes or NULL if the operation does not succeed
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
 * \return A NetworkMessage corresponding to the UADP packet or NULL if the operation does not succeed
 *
 */
SOPC_UADP_NetworkMessage* SOPC_UADP_NetworkMessage_Decode(SOPC_Buffer* buffer,
                                                          SOPC_UADP_GetSecurity_Func getSecurity_Func);

void SOPC_UADP_NetworkMessage_Delete(SOPC_UADP_NetworkMessage* uadp_nm);

/** Error code returned by ::SOPC_UADP_NetworkMessage_Encode
 * or ::SOPC_UADP_NetworkMessage_Encode */
typedef enum
{
    SOPC_UADP_NetworkMessage_Error_Code_None = 0,
    SOPC_UADP_NetworkMessage_Error_Write_Buffer_Failed = 0x10000000,
    SOPC_UADP_NetworkMessage_Error_Write_PubId_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_GroupId_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_GroupVersion_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_WriterId_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_TokenId_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_SecuHdr_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_SecuFooter_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_Alloc_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_DsmSize_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_DsmPreSize_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_DsmField_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_EncryptPaylod_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_PayloadFlush_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_Sign_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_Byte_Failed = 0x20000000,
    SOPC_UADP_NetworkMessage_Error_Read_Short_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_Int_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_Alloc_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_Security_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_SecurityConf_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_SecuritySign_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_SecuritySignSize_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_SecurityNonce_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_SecurityDecrypt_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_SecurityNone_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_SeqNum_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_DsmSize_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_InvalidBit,
    SOPC_UADP_NetworkMessage_Error_Read_DsmFields_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_DsmSizeCheck_Failed,
    SOPC_UADP_NetworkMessage_Error_Unsupported_Flags1 = 0x30000000,
    SOPC_UADP_NetworkMessage_Error_Unsupported_PubIdType,
    SOPC_UADP_NetworkMessage_Error_Unsupported_ClassId,
    SOPC_UADP_NetworkMessage_Error_Unsupported_MessageNum,
    SOPC_UADP_NetworkMessage_Error_Unsupported_SeqNum,
    SOPC_UADP_NetworkMessage_Error_Unsupported_Timestamp,
    SOPC_UADP_NetworkMessage_Error_Unsupported_Picosceonds,
    SOPC_UADP_NetworkMessage_Error_Unsupported_SecurityFooterReset,
    SOPC_UADP_NetworkMessage_Error_Unsupported_PromotedFields,
    SOPC_UADP_NetworkMessage_Error_Unsupported_EncodingType,
    SOPC_UADP_NetworkMessage_Error_Unsupported_DsmType,
    SOPC_UADP_NetworkMessage_Error_Unsupported_DsmSeqNum,
    SOPC_UADP_NetworkMessage_Error_Unsupported_DsmTimeStamp,
    SOPC_UADP_NetworkMessage_Error_Unsupported_DsmPicoseconds,
} SOPC_UADP_NetworkMessage_Error_Code;

/**
 * Return the last decode/encode error and reset its value.
 * This function should be called after a NULL returned value in a call to ::SOPC_UADP_NetworkMessage_Encode
 * or ::SOPC_UADP_NetworkMessage_Encode
 * This value is not thread-safe, and is only intended for debugging/maintenance purpose
 * @return The last error code.
 * @FIXME Use an additional parameter to encode/decode function rather than this context-less function.
 */
SOPC_UADP_NetworkMessage_Error_Code SOPC_UADP_NetworkMessage_Get_Last_Error(void);

#endif /* SOPC_NETWORK_LAYER_H_ */
