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

#include <assert.h>

#include "sopc_dataset_ll_layer.h"
#include "sopc_network_layer.h"
#include "sopc_pubsub_helpers.h"
#include "sopc_reader_layer.h"

/**
 * Filter at NetworkMessage Level
 *
 */
static inline bool SOPC_Sub_Filter_Reader_PublisherId(const SOPC_Conf_PublisherId* conf_pubid,
                                                      const SOPC_Dataset_LL_PublisherId* nm_pubid);

static bool SOPC_Sub_Filter_Reader_FieldMetaData(const SOPC_DataSetReader* reader,
                                                 const SOPC_Dataset_LL_DataSetMessage* dsm);
/** \brief
 *      Identify a reader in connection matching received parameters
 * \param connection The connection configuration
 * \param uadp_conf The received message configuration
 * \param pubid The Publisher Id received
 * \param groupVersion The GroupVersion received
 * \param groupId The GroupId received
 * \return the reader group, or NULL if no matching group found
 */
static const SOPC_ReaderGroup* SOPC_Sub_GetReaderGroup(const SOPC_PubSubConnection* connection,
                                                       const SOPC_UADP_Configuration* uadp_conf,
                                                       const SOPC_Dataset_LL_PublisherId* pubid,
                                                       const uint32_t groupVersion,
                                                       const uint32_t groupId);

/** \brief
 *      Identify a reader in a group matching received parameters in nm
 * \param group The group configuration
 * \param uadp_conf The received message configuration
 * \param writerId The received dataset writerId
 * \param dataSetIndex The index of the DataSet in the received message
 *
 * \return the dataset reader, or NULL if no matching reader found
 */
static const SOPC_DataSetReader* SOPC_Sub_GetReader(const SOPC_ReaderGroup* group,
                                                    const SOPC_UADP_Configuration* uadp_conf,
                                                    const uint16_t writerId,
                                                    const uint8_t dataSetIndex);

/** \brief
 *      Received a DSM and applies changes to target variables.
 * \pre Groups & Writer Id must have been checked prior to call.
 * \param dsm The received DataSetMesasge
 * \param uadp_conf The received message configuration
 * \param writerId The received dataset writerId
 *
 * \return the dataset reader, or NULL if no matching reader found
 */
static SOPC_ReturnStatus SOPC_Sub_ReceiveDsm(const SOPC_Dataset_LL_DataSetMessage* dsm,
                                             SOPC_SubTargetVariableConfig* config,
                                             const SOPC_DataSetReader* reader);

const SOPC_UADP_NetworkMessage_Reader_Callbacks SOPC_Reader_NetworkMessage_Default_Readers = {
    .pGetGroup_Func = &SOPC_Sub_GetReaderGroup,
    .pGetReader_Func = &SOPC_Sub_GetReader,
    .pSetDsm_Func = &SOPC_Sub_ReceiveDsm};

SOPC_ReturnStatus SOPC_Reader_Read_UADP(const SOPC_PubSubConnection* connection,
                                        SOPC_Buffer* buffer,
                                        SOPC_SubTargetVariableConfig* config,
                                        SOPC_UADP_GetSecurity_Func securityCBck)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const SOPC_UADP_NetworkMessage_Reader_Configuration readerConf = {
        .pGetSecurity_Func = securityCBck,
        .callbacks = SOPC_Reader_NetworkMessage_Default_Readers,
        .targetConfig = config};
    SOPC_UADP_NetworkMessage* uadp_nm = SOPC_UADP_NetworkMessage_Decode(buffer, &readerConf, connection);
    if (NULL == uadp_nm)
    {
        /* TODO: have a more resilient behavior and avoid stopping the subscriber because of
         * random bytes found on the network */
        return SOPC_STATUS_ENCODING_ERROR;
    }

    SOPC_UADP_NetworkMessage_Delete(uadp_nm);
    return status;
}

static const SOPC_ReaderGroup* SOPC_Sub_GetReaderGroup(const SOPC_PubSubConnection* connection,
                                                       const SOPC_UADP_Configuration* uadp_conf,
                                                       const SOPC_Dataset_LL_PublisherId* pubid,
                                                       const uint32_t groupVersion,
                                                       const uint32_t groupId)
{
    assert(NULL != connection && uadp_conf != NULL);
    // Find a matching ReaderGroup in connection
    const uint16_t nbReaderGroup = SOPC_PubSubConnection_Nb_ReaderGroup(connection);

    SOPC_ReaderGroup* result = NULL;

    for (uint16_t i = 0; i < nbReaderGroup && NULL == result; i++)
    {
        bool match = true;
        SOPC_ReaderGroup* readerGroup = SOPC_PubSubConnection_Get_ReaderGroup_At(connection, i);
        assert(NULL != readerGroup);

        if (uadp_conf->GroupVersionFlag)
        {
            // Check group version
            const uint32_t confVersion = SOPC_ReaderGroup_Get_GroupVersion(readerGroup);

            match &= (groupVersion == confVersion) || (0 == confVersion);
        }

        if (match && uadp_conf->GroupIdFlag)
        {
            // Check group Id
            const uint16_t confGroupId = SOPC_ReaderGroup_Get_GroupId(readerGroup);

            match &= (confGroupId == groupId) || (0 == confGroupId);
        }

        if (match && uadp_conf->PublisherIdFlag)
        {
            // Check PublisherIdFlag
            const SOPC_Conf_PublisherId* expPubId = SOPC_ReaderGroup_Get_PublisherId(readerGroup);
            match &= SOPC_Sub_Filter_Reader_PublisherId(expPubId, pubid);
        }
        if (match)
        {
            result = readerGroup;
        }
    }
    return result;
}

static const SOPC_DataSetReader* SOPC_Sub_GetReader(const SOPC_ReaderGroup* group,
                                                    const SOPC_UADP_Configuration* uadp_conf,
                                                    const uint16_t writerId,
                                                    const uint8_t dataSetIndex)
{
    assert(NULL != group && uadp_conf != NULL);
    // Find a matching reader in group
    const SOPC_DataSetReader* result = NULL;

    // Note: it has been checked previously that the group does not contain both zero and non-zero writerId
    if (SOPC_ReaderGroup_HasNonZeroDataSetWriterId(group))
    {
        const uint16_t nbReaders = SOPC_ReaderGroup_Nb_DataSetReader(group);
        for (uint8_t i = 0; i < nbReaders && NULL == result; i++)
        {
            // Find matching WriterId
            const SOPC_DataSetReader* reader = SOPC_ReaderGroup_Get_DataSetReader_At(group, i);
            const uint16_t readerWriterId = SOPC_DataSetReader_Get_DataSetWriterId(reader);

            if (writerId == readerWriterId && writerId != 0)
            {
                result = reader;
            }
        }
    }
    else
    {
        // Use configuration order
        result = SOPC_ReaderGroup_Get_DataSetReader_At(group, dataSetIndex);
    }
    return result;
}

// See SOPC_UADP_NetworkMessage_SetDsm
static SOPC_ReturnStatus SOPC_Sub_ReceiveDsm(const SOPC_Dataset_LL_DataSetMessage* dsm,
                                             SOPC_SubTargetVariableConfig* targetConfig,
                                             const SOPC_DataSetReader* reader)
{
    assert(NULL != dsm && NULL != reader);
    SOPC_ReturnStatus result = SOPC_STATUS_ENCODING_ERROR;

    if (SOPC_Sub_Filter_Reader_FieldMetaData(reader, dsm))
    {
        bool write_succes = (targetConfig == NULL || SOPC_SubTargetVariable_SetVariables(targetConfig, reader, dsm));
        if (write_succes)
        {
            result = SOPC_STATUS_OK;
        }
    }
    return result;
}

static bool SOPC_Sub_Filter_Reader_PublisherId(const SOPC_Conf_PublisherId* conf_pubid,
                                               const SOPC_Dataset_LL_PublisherId* nm_pubid)
{
    assert(NULL != conf_pubid && NULL != nm_pubid);

    switch (conf_pubid->type)
    {
    case SOPC_String_PublisherId:
        // Not managed
        assert(DataSet_LL_PubId_String_Id != nm_pubid->type);
        return false;
    case SOPC_UInteger_PublisherId:
    {
        uint64_t nm_pubid64;

        switch (nm_pubid->type)
        {
        case DataSet_LL_PubId_Byte_Id:
            nm_pubid64 = nm_pubid->data.byte;
            break;
        case DataSet_LL_PubId_UInt16_Id:
            nm_pubid64 = nm_pubid->data.uint16;
            break;
        case DataSet_LL_PubId_UInt32_Id:
            nm_pubid64 = nm_pubid->data.uint32;
            break;
        case DataSet_LL_PubId_UInt64_Id:
            nm_pubid64 = nm_pubid->data.uint64;
            break;
        default:
            // should not happen
            return false;
        }

        return conf_pubid->data.uint == nm_pubid64;
    }
    case SOPC_Null_PublisherId:
        // if there is no expected publisher id, this filter is passed
        return true;
    default:
        return false;
    }
}

static bool SOPC_Sub_Filter_Reader_FieldMetaData(const SOPC_DataSetReader* reader,
                                                 const SOPC_Dataset_LL_DataSetMessage* dsm)
{
    uint16_t datasetFieldsNb = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);

    bool result = datasetFieldsNb == SOPC_DataSetReader_Nb_FieldMetaData(reader);

    for (uint16_t i = 0; result && i < datasetFieldsNb; i++)
    {
        // Get field config
        SOPC_FieldMetaData* fmd = SOPC_DataSetReader_Get_FieldMetaData_At(reader, i);
        // Get field value in message
        const SOPC_Variant* variant = SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(dsm, i);

        result = SOPC_PubSubHelpers_IsCompatibleVariant(fmd, variant, NULL);
    }

    return result;
}
