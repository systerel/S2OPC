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
static bool SOPC_Sub_Filter_Reader_NetworkMessage(const SOPC_DataSetReader* reader,
                                                  const SOPC_UADP_NetworkMessage* uadp_nm);
static inline bool SOPC_Sub_Filter_Reader_PublisherId(const SOPC_DataSetReader* reader,
                                                      const SOPC_Dataset_LL_NetworkMessage* nm,
                                                      const SOPC_Dataset_LL_NetworkMessage_Header* header,
                                                      const SOPC_UADP_Configuration* uadp_conf);
static inline bool SOPC_Sub_Filter_Reader_WriterGroupId(const SOPC_DataSetReader* reader,
                                                        const SOPC_Dataset_LL_NetworkMessage* nm,
                                                        const SOPC_UADP_Configuration* uadp_conf);
static inline bool SOPC_Sub_Filter_Reader_WriterGroupVersion(const SOPC_DataSetReader* reader,
                                                             const SOPC_Dataset_LL_NetworkMessage* nm,
                                                             const SOPC_UADP_Configuration* uadp_conf);

/**
 * Filter at DataSetMessage Level
 *
 * Only DataSetWriter id is filter for this version.
 *
 * Next Step is filtering on:
 *  - DataSetMetaData : version
 *  - DataSetFieldContentMask
 */
static bool SOPC_Sub_Filter_Reader_DataSetMessage(const SOPC_DataSetReader* reader,
                                                  const SOPC_UADP_Configuration* contentMask,
                                                  const SOPC_Dataset_LL_DataSetMessage* dsm);
static bool SOPC_Sub_Filter_Reader_DataSetWriter(const SOPC_DataSetReader* reader,
                                                 const SOPC_UADP_Configuration* contentMask,
                                                 const SOPC_Dataset_LL_DataSetMessage* dsm);
static bool SOPC_Sub_Filter_Reader_FieldMetaData(const SOPC_DataSetReader* reader,
                                                 const SOPC_Dataset_LL_DataSetMessage* dsm);

// Generic function to filter a writer Attribute
static bool SOPC_Sub_Filter_Reader_WriterAttr(uint32_t expectedValue, bool msgEnabled, uint32_t msgValue);

SOPC_ReturnStatus SOPC_Reader_Read_UADP(const SOPC_PubSubConnection* connection,
                                        SOPC_Buffer* buffer,
                                        SOPC_SubTargetVariableConfig* config,
                                        SOPC_UADP_GetSecurity_Func securityCBck)
{
    SOPC_UADP_NetworkMessage* uadp_nm = SOPC_UADP_NetworkMessage_Decode(buffer, securityCBck);
    if (NULL == uadp_nm)
    {
        /* TODO: have a more resilient behavior and avoid stopping the subscriber because of
         * random bytes found on the network */
        return SOPC_STATUS_ENCODING_ERROR;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Dataset_LL_NetworkMessage* nm = uadp_nm->nm;
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);
    SOPC_UADP_Configuration* uadp_conf = SOPC_Dataset_LL_NetworkMessage_GetHeaderConfig(header);

    uint16_t nbReaderGroup = SOPC_PubSubConnection_Nb_ReaderGroup(connection);

    // Iterate on DataSetReader through ReaderGroup
    for (uint16_t i = 0; i < nbReaderGroup; i++)
    {
        SOPC_ReaderGroup* readerGroup = SOPC_PubSubConnection_Get_ReaderGroup_At(connection, i);
        assert(NULL != readerGroup);

        uint8_t nbReader = SOPC_ReaderGroup_Nb_DataSetReader(readerGroup);
        for (uint8_t j = 0; j < nbReader; j++)
        {
            SOPC_DataSetReader* reader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, j);
            assert(NULL != reader);

            // Applies filter at Network Message Level
            if (SOPC_Sub_Filter_Reader_NetworkMessage(reader, uadp_nm))
            {
                // Applies filter at DataSetMessage Message Level
                uint16_t nbDsm = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm);
                for (uint8_t k = 0; k < nbDsm; k++)
                {
                    SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, k);
                    assert(NULL != dsm);
                    if (SOPC_Sub_Filter_Reader_DataSetMessage(reader, uadp_conf, dsm))
                    {
                        bool write_succes = SOPC_SubTargetVariable_SetVariables(config, reader, dsm);
                        if (!write_succes)
                        {
                            status = SOPC_STATUS_NOK;
                        }
                    }
                }
            }
        }
    }

    SOPC_UADP_NetworkMessage_Delete(uadp_nm);
    return status;
}

static bool SOPC_Sub_Filter_Reader_NetworkMessage(const SOPC_DataSetReader* reader,
                                                  const SOPC_UADP_NetworkMessage* uadp_nm)
{
    SOPC_Dataset_LL_NetworkMessage* nm = uadp_nm->nm;
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);
    const SOPC_UADP_Configuration* uadp_conf = SOPC_Dataset_LL_NetworkMessage_GetHeaderConfig(header);

    return (SOPC_Sub_Filter_Reader_PublisherId(reader, nm, header, uadp_conf) &&
            SOPC_Sub_Filter_Reader_WriterGroupId(reader, nm, uadp_conf) &&
            SOPC_Sub_Filter_Reader_WriterGroupVersion(reader, nm, uadp_conf));
}

static bool SOPC_Sub_Filter_Reader_PublisherId(const SOPC_DataSetReader* reader,
                                               const SOPC_Dataset_LL_NetworkMessage* nm,
                                               const SOPC_Dataset_LL_NetworkMessage_Header* header,
                                               const SOPC_UADP_Configuration* uadp_conf)
{
    assert(NULL != uadp_conf && NULL != nm);

    SOPC_DataSet_LL_PublisherIdType nm_pubid_type = SOPC_Dataset_LL_NetworkMessage_Get_PublisherIdType(header);
    const SOPC_Dataset_LL_PublisherId* nm_pubid = SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header);
    const SOPC_Conf_PublisherId* conf_pubid = SOPC_DataSetReader_Get_PublisherId(reader);
    assert(NULL != nm_pubid && NULL != conf_pubid);

    switch (conf_pubid->type)
    {
    case SOPC_String_PublisherId:
        // Not managed
        assert(DataSet_LL_PubId_String_Id != nm_pubid_type);
        return false;
    case SOPC_UInteger_PublisherId:
    {
        uint64_t nm_pubid64;

        // Publisher id is expected in the Network Message
        if (!uadp_conf->PublisherIdFlag)
        {
            return false;
        }

        switch (nm_pubid_type)
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

static bool SOPC_Sub_Filter_Reader_WriterAttr(const uint32_t expectedValue,
                                              const bool msgEnabled,
                                              const uint32_t msgValue)
{
    // if there is no expected writer group attribute, this filter is passed
    if (0 == expectedValue)
    {
        return true;
    }

    // Group attribute is expected in the Network Message
    if (!msgEnabled)
    {
        return false;
    }
    return (expectedValue == msgValue);
}

static inline bool SOPC_Sub_Filter_Reader_WriterGroupId(const SOPC_DataSetReader* reader,
                                                        const SOPC_Dataset_LL_NetworkMessage* nm,
                                                        const SOPC_UADP_Configuration* uadp_conf)
{
    return SOPC_Sub_Filter_Reader_WriterAttr(SOPC_DataSetReader_Get_WriterGroupId(reader),
                                             uadp_conf->GroupIdFlag,
                                             SOPC_Dataset_LL_NetworkMessage_Get_GroupId(nm));
}

static inline bool SOPC_Sub_Filter_Reader_WriterGroupVersion(const SOPC_DataSetReader* reader,
                                                             const SOPC_Dataset_LL_NetworkMessage* nm,
                                                             const SOPC_UADP_Configuration* uadp_conf)
{
    return SOPC_Sub_Filter_Reader_WriterAttr(SOPC_DataSetReader_Get_WriterGroupVersion(reader),
                                             uadp_conf->GroupVersionFlag,
                                             SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(nm));
}

static bool SOPC_Sub_Filter_Reader_DataSetWriter(const SOPC_DataSetReader* reader,
                                                 const SOPC_UADP_Configuration* contentMask,
                                                 const SOPC_Dataset_LL_DataSetMessage* dsm)
{
    return SOPC_Sub_Filter_Reader_WriterAttr(SOPC_DataSetReader_Get_DataSetWriterId(reader),
                                             contentMask->PayloadHeaderFlag,
                                             SOPC_Dataset_LL_DataSetMsg_Get_WriterId(dsm));
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

static bool SOPC_Sub_Filter_Reader_DataSetMessage(const SOPC_DataSetReader* reader,
                                                  const SOPC_UADP_Configuration* contentMask,
                                                  const SOPC_Dataset_LL_DataSetMessage* dsm)
{
    return (SOPC_Sub_Filter_Reader_FieldMetaData(reader, dsm) &&
            SOPC_Sub_Filter_Reader_DataSetWriter(reader, contentMask, dsm));
}
