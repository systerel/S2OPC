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
#include "sopc_sub_target_variable.h"

// TODO: use security mode instead of security enabled
// TODO: remove SOPC_UADP_NetworkMessage_Get_Last_Error and provide a call-specific return value
//
typedef struct SOPC_UADP_Network_Message
{
    SOPC_Dataset_LL_NetworkMessage* nm;
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
 * \brief A callback for ReaderGroup identification. When a network message is received, this
 *      callback is expected to return a matching ::SOPC_ReaderGroup. It is used to check if the remaining
 *      parts of the message has to be decoded or not.
 *
 * \param connection is the Network connection
 * \param uadp_conf Configuration of the received message
 * \param pubid Publisher Id received
 * \param groupVersion Group Version received
 * \param groupId Group Id received
 *
 * \return A ::SOPC_ReaderGroup matching the received message or NULL if no configured connection matches.
 */
typedef const SOPC_ReaderGroup* SOPC_UADP_NetworkMessage_GetReaderGroup(const SOPC_PubSubConnection* connection,
                                                                        const SOPC_UADP_Configuration* uadp_conf,
                                                                        const SOPC_Dataset_LL_PublisherId* pubid,
                                                                        const uint32_t groupVersion,
                                                                        const uint32_t groupId);

/**
 * \brief A callback for DataSetReader identification. When a DataSet content is received, this
 *      callback is expected to return a matching ::SOPC_DataSetReader. It is used to check if the content of
 *      the dataset has to be decoded or not, and to route decoding to the correct reader.
 *
 * \param group is the Reader Group on which the DataSet is received
 * \param uadp_conf Configuration of the received message
 * \param dataSetWriterId DataSetWriter Id received
 * \param dataSetIndex The index of the DataSet in the received message
 *
 * \return A ::SOPC_DataSetReader matching the received DataSet or \c NULL if no matching Reader is configured for the
 * given group.
 */
typedef const SOPC_DataSetReader* SOPC_UADP_NetworkMessage_GetReader(const SOPC_ReaderGroup* group,
                                                                     const SOPC_UADP_Configuration* uadp_conf,
                                                                     const uint16_t dataSetWriterId,
                                                                     const uint8_t dataSetIndex);

/**
 * \brief A callback for DataSet message application. When a compliant DataSet is received, this
 *      callback is expected to forward all variables received in \a dsm to \a targetConfig, depending on filter
 * parameters in \a reader
 *
 * \param dsm The received \c DataSetMessage
 * \param targetConfig Configuration of the Subscriber receive event
 * \param reader The matching reader.
 *
 * \return - ::SOPC_STATUS_OK if all variables were written
 *         - ::SOPC_STATUS_ENCODING_ERROR if the received DataSetMessage does not match expected \a reader
 * configuration.
 *         - ::SOPC_STATUS_ENCODING_ERROR if the user-level ::SOPC_SubTargetVariable_SetVariables fails.
 */
typedef SOPC_ReturnStatus SOPC_UADP_NetworkMessage_SetDsm(const SOPC_Dataset_LL_DataSetMessage* dsm,
                                                          SOPC_SubTargetVariableConfig* targetConfig,
                                                          const SOPC_DataSetReader* reader);

typedef struct
{
    SOPC_UADP_NetworkMessage_GetReaderGroup* pGetGroup_Func;
    SOPC_UADP_NetworkMessage_GetReader* pGetReader_Func;
    SOPC_UADP_NetworkMessage_SetDsm* pSetDsm_Func;
} SOPC_UADP_NetworkMessage_Reader_Callbacks;

typedef struct
{
    SOPC_UADP_NetworkMessage_Reader_Callbacks callbacks;
    SOPC_UADP_GetSecurity_Func* pGetSecurity_Func;
    SOPC_SubTargetVariableConfig* targetConfig;
} SOPC_UADP_NetworkMessage_Reader_Configuration;

/**
 * \brief Decode a UADP packet into a NetworkMessage
 *
 * \param buffer the UADP byte to decode

 * \param reader_config The configuration for message parsing/filtering
 * \param connection The related connection
 *
 * \return a pointer to a new network message (must be freed by caller) if decoding succeeded and led
 *          to at least 1 variable update.
 *          Otherwise, call ::SOPC_UADP_NetworkMessage_Get_Last_Error to identify decoding error.
 *          Note that this can simply caused by the fact that the received message does not
 *          fit any Group/PublisherId/WriterId filters.
 */
SOPC_UADP_NetworkMessage* SOPC_UADP_NetworkMessage_Decode(
    SOPC_Buffer* buffer,
    const SOPC_UADP_NetworkMessage_Reader_Configuration* reader_config,
    const SOPC_PubSubConnection* connection);

void SOPC_UADP_NetworkMessage_Delete(SOPC_UADP_NetworkMessage* uadp_nm);

/** Error code used by ::SOPC_UADP_NetworkMessage_Encode
 * or ::SOPC_UADP_NetworkMessage_Decode. Call ::SOPC_UADP_NetworkMessage_Get_Last_Error to
 * retrieve the last encountered error */
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
    SOPC_UADP_NetworkMessage_Error_Read_DsmSkip_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_DsmSize_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_InvalidBit,
    SOPC_UADP_NetworkMessage_Error_Read_DsmFields_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_DsmSizeCheck_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_NoMatchingGroup = 0x30000000,
    SOPC_UADP_NetworkMessage_Error_Read_NoMatchingReader,
    SOPC_UADP_NetworkMessage_Error_Read_BadMetaData,
    SOPC_UADP_NetworkMessage_Error_Unsupported_Version = 0x40000000,
    SOPC_UADP_NetworkMessage_Error_Unsupported_Flags1,
    SOPC_UADP_NetworkMessage_Error_Unsupported_Flags2,
    SOPC_UADP_NetworkMessage_Error_Unsupported_PubIdType,
    SOPC_UADP_NetworkMessage_Error_Unsupported_ClassId,
    SOPC_UADP_NetworkMessage_Error_Unsupported_MessageNum,
    SOPC_UADP_NetworkMessage_Error_Unsupported_SeqNum,
    SOPC_UADP_NetworkMessage_Error_Unsupported_Timestamp,
    SOPC_UADP_NetworkMessage_Error_Unsupported_Picoseconds,
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
 * This function should be called after a \c NULL returned value in a call to ::SOPC_UADP_NetworkMessage_Encode
 * or ::SOPC_UADP_NetworkMessage_Decode
 * This value is not thread-safe, and is only intended for debugging/maintenance purpose
 * @return The last error code.
 */
SOPC_UADP_NetworkMessage_Error_Code SOPC_UADP_NetworkMessage_Get_Last_Error(void);

#endif /* SOPC_NETWORK_LAYER_H_ */
