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

/**
 * \file sopc_network_layer.h
 * UADP Encoding is divided in two stages
 *  - UADP header and UADP payload are mapped in two distinct buffers
 *  - Depending on security configuration we sign and encrypt and merge the two buffers in one buffer.
 *
 * The header buffer contains the final buffer to be sent. It is composed of non-encrypted headers followed by the
 * payload buffer copy, which may be encrypted
 *
 * The payload buffer contains the unencrypted payload
 */

#ifndef SOPC_NETWORK_LAYER_H_
#define SOPC_NETWORK_LAYER_H_

#include <inttypes.h>
#include "sopc_buffer.h"
#include "sopc_dataset_ll_layer.h"
#include "sopc_pub_fixed_buffer.h"
#include "sopc_pubsub_conf.h"
#include "sopc_pubsub_security.h"
#include "sopc_sub_scheduler.h"
#include "sopc_sub_target_variable.h"

/** Error code used by encoding and decoding functions */
typedef enum
{
    SOPC_NetworkMessage_Error_Code_None = 0,
    SOPC_NetworkMessage_Error_Code_InvalidParameters,
    SOPC_UADP_NetworkMessage_Error_Write_Buffer_Failed = 0x10000000,
    SOPC_UADP_NetworkMessage_Error_Write_PubId_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_GroupId_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_GroupVersion_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_WriterId_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_TokenId_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_SecuHdr_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_SecuFooter_Failed,
    SOPC_NetworkMessage_Error_Write_Alloc_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_DsmSize_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_DsmPreSize_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_DsmField_Failed,
    SOPC_UADP_NetworkMessage_Error_Write_DsmSeqNum_Failed,
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
    SOPC_UADP_NetworkMessage_Error_Read_DsmSeqNum_Failed,
    SOPC_UADP_NetworkMessage_Error_Read_DsmSeqNumCheck_Failed,
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
    SOPC_JSON_NetworkMessage_Error_Generate_Unique_MessageId = 0x50000000,
    SOPC_JSON_NetworkMessage_Error_Generate_Unique_VariantName,
    SOPC_JSON_NetworkMessage_Error_Encode,
    SOPC_JSON_NetworkMessage_Error_DataSetMessage_Encode,
    SOPC_JSON_NetworkMessage_Error_Variant_Encode,
    SOPC_JSON_NetworkMessage_Error_Write_Closing_Structure,
    SOPC_JSON_NetworkMessage_Error_Security_Unsupported,
} SOPC_NetworkMessage_Error_Code;

typedef struct SOPC_UADP_Network_Message
{
    SOPC_Dataset_LL_NetworkMessage* nm;
} SOPC_UADP_NetworkMessage;

/** \brief Function used to check if a DataSetMessage sequence number is newer for the identified DataSetWriter
 *
 * \param pubId       the publisher id associated to the DataSetWriter
 * \param groupId     the WriterGroupId
 * \param writerId    the DataSetWriter id
 * \param receivedSN  the dataset message sequence number
 *
 * \return True if \a receivedSn is newer (valid) for this dataSetWriter. False otherwise
 */
typedef bool SOPC_UADP_IsWriterSequenceNumberNewer_Func(const SOPC_Conf_PublisherId* pubId,
                                                        const uint16_t groupId,
                                                        const uint16_t writerId,
                                                        const uint16_t receivedSN);

/**
 * @brief Function used to update timeout state of an identified dataSetWriter. This function should be call if decode
 * process was succesfull
 *
 * @param pubId the publisher id associated to the DataSetWriter
 * @param groupId the WriterGroupId
 * @param writerId the DataSetWriter id
 */
typedef void SOPC_UADP_UpdateTimeout_Func(const SOPC_Conf_PublisherId* pubId,
                                          const uint16_t groupId,
                                          const uint16_t writerId);

/**
 * @brief check if received sequence number is newer than processsed sequence number following part 14 rules
 *
 * @param received  received sequence number
 * @param processed sequence number already processed
 * @return true in case received sequence number is newer than processed, false otherwise
 */
bool SOPC_Is_UInt16_Sequence_Number_Newer(uint16_t received, uint16_t processed);

/**
 * \brief Encode a NetworkMessage with JSON Mapping
 *
 * \param message is the NetworkMessage to encode
 * \param security is the data use to encrypt and sign. Can be NULL if security is not used.
 *                 No security possible for JSON encoding.
 * \param buffer   pointer to a newly allocated buffer containing the JSON string
 *
 * \return ::SOPC_NetworkMessage_Error_Code_None if buffer is successfully encoded. Appropriate error code otherwise
 */
SOPC_NetworkMessage_Error_Code SOPC_JSON_NetworkMessage_Encode(SOPC_Dataset_LL_NetworkMessage* message,
                                                               SOPC_PubSub_SecurityType* security,
                                                               SOPC_Buffer** buffer);

/**
 * @brief Encode a NetworkMessage with UADP Mapping. Buffer is split in a Header buffer and payload buffer
 *
 * @param nm is the NetworkMessage to encode
 * @param security is the data use to set security flags. Can be NULL if security is not used
 * @param buffer_header [OUT] pointer to a newly allocated buffer with header flags encoded.
 * @param buffer_payload [OUT] pointer to a newly allocated buffer with payload data encoded.
 * @return ::SOPC_NetworkMessage_Error_Code_None if header and payload buffer are successfully encoded. Appropriate
 * error code otherwise
 */
SOPC_NetworkMessage_Error_Code SOPC_UADP_NetworkMessage_Encode_Buffers(SOPC_Dataset_LL_NetworkMessage* nm,
                                                                       SOPC_PubSub_SecurityType* security,
                                                                       SOPC_Buffer** buffer_header,
                                                                       SOPC_Buffer** buffer_payload);

/**
 * @brief Sign and encrypt encoded buffer if necessary and merge header and payload buffer in one buffer.
 *
 * @param security is the data used to encrypt and sign. Can be NULL if security is not used
 * @param buffer_header [IN/OUT] encoded header buffer which will become the final buffer
 * @param buffer_payload encoded payload buffer. Freed in all cases by this function
 * @return SOPC_NetworkMessage_Error_Code_None in case of succes another code otherwise
 */
SOPC_NetworkMessage_Error_Code SOPC_UADP_NetworkMessage_BuildFinalMessage(SOPC_PubSub_SecurityType* security,
                                                                          SOPC_Buffer* buffer_header,
                                                                          SOPC_Buffer** buffer_payload);

/**
 * @brief Get updated preencoded buffer.
 *
 * @param nm NetworkMessage containing preencoded structure
 * @param security the data used to encrypt and sign. Must be NULL, preencode mechanism cannot handle sign and encrypt
 *
 * @return SOPC_Buffer* pointer to updated preencoded buffer, NULL if an error occur.
 */
SOPC_Buffer* SOPC_UADP_NetworkMessage_Get_PreencodedBuffer(SOPC_Dataset_LL_NetworkMessage* nm,
                                                           SOPC_PubSub_SecurityType* security);

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
 *
 * \return A ::SOPC_DataSetReader matching the received DataSet or \c NULL if no matching Reader is configured for the
 * given group.
 */
typedef const SOPC_DataSetReader* SOPC_UADP_NetworkMessage_GetReader(const SOPC_ReaderGroup* group,
                                                                     const SOPC_UADP_Configuration* uadp_conf,
                                                                     const uint16_t dataSetWriterId);

/**
 * \brief A callback for DataSet message application. When a compliant DataSet is received, this
 *      callback is expected to forward all variables received in \a dsm to \a targetConfig, depending on filter
 * parameters in \a reader
 *
 * \param dsm The received \c DataSetMessage
 * \param targetConfig Configuration of the Subscriber receive event
 * \param reader The matching reader.
 * \param targetVariable Object containing the data provided to user
 *
 * \return - ::SOPC_STATUS_OK if all variables were written
 *         - ::SOPC_STATUS_ENCODING_ERROR if the received DataSetMessage does not match expected \a reader
 * configuration.
 *         - ::SOPC_STATUS_ENCODING_ERROR if the user-level ::SOPC_SubTargetVariable_SetVariables fails.
 */
typedef SOPC_ReturnStatus SOPC_UADP_NetworkMessage_SetDsm(SOPC_Dataset_LL_DataSetMessage* dsm,
                                                          SOPC_SubTargetVariableConfig* targetConfig,
                                                          const SOPC_DataSetReader* reader,
                                                          SOPC_TargetVariableCtx* targetVariable);

/**
 * @brief A callback to retrieve a ::SOPC_TargetVariableCtx provided to user. The ::SOPC_TargetVariableCtx
 *      provided to the user cannot be modified. Ideally a ::SOPC_TargetVariableCtx has been created per
 * dataSetMessage and can be identify by the tuple PublisherId, WriterId
 *
 * @param pubId Publisher Id of the networkMessage received
 * @param groupId WriterGroup Id of the received message
 * @param writerId  DataSetWriter Id of the received message
 * @return An initialized and fill ::SOPC_TargetVariableCtx object associated to this dataSetMessage. NULL in case of
 * error
 */
typedef SOPC_TargetVariableCtx* SOPC_UADP_GetTargetVariable_Func(const SOPC_Conf_PublisherId* pubId,
                                                                 const uint16_t groupId,
                                                                 const uint16_t writerId);

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
    SOPC_UADP_IsWriterSequenceNumberNewer_Func* checkDataSetMessageSN_Func;
    SOPC_SubTargetVariableConfig* targetConfig;
    SOPC_UADP_UpdateTimeout_Func* updateTimeout_Func;
    SOPC_UADP_GetTargetVariable_Func* targetVariable_Func;
} SOPC_UADP_NetworkMessage_Reader_Configuration;

/**
 * \brief Decode a UADP packet into a NetworkMessage
 *
 * \param buffer the UADP byte to decode

 * \param reader_config The configuration for message parsing/filtering
 * \param connection The related connection
 * \param uadp_nm a pointer to a new network message (must be freed by caller) if decoding succeeded
 *
 * \return  ::SOPC_NetworkMessage_Error_Code_None if buffer is successfully decoded  and led to at least 1 variable
 update.
 *          Appropriate error code otherwise.
 *          Note that this can simply caused by the fact that the received message does not
 *          fit any Group/PublisherId/WriterId filters.
 */
SOPC_NetworkMessage_Error_Code SOPC_UADP_NetworkMessage_Decode(
    SOPC_Buffer* buffer,
    const SOPC_UADP_NetworkMessage_Reader_Configuration* reader_config,
    const SOPC_PubSubConnection* connection,
    SOPC_UADP_NetworkMessage** uadp_nm);

void SOPC_UADP_NetworkMessage_Delete(SOPC_UADP_NetworkMessage* uadp_nm);

#endif /* SOPC_NETWORK_LAYER_H_ */
