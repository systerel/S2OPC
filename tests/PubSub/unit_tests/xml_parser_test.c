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

#include <check.h>
#include <stdlib.h>

#include "sopc_xml_loader.h"

START_TEST(test_pub_xml_parsing)
{
    fprintf(stderr, "opening 'config_pub.xml'\n"); // TODO JCH
    FILE* fd = fopen("./config_pub.xml", "r");
    ck_assert_ptr_nonnull(fd);

    SOPC_PubSubConfiguration* config = SOPC_PubSubConfig_ParseXML(fd);
    int closed = fclose(fd);
    ck_assert_int_eq(0, closed);

    const uint32_t nb_pubconnections = SOPC_PubSubConfiguration_Nb_PubConnection(config);
    ck_assert_uint_eq(1, nb_pubconnections);
    const uint32_t nb_subconnections = SOPC_PubSubConfiguration_Nb_SubConnection(config);
    ck_assert_uint_eq(0, nb_subconnections);

    const uint32_t nb_pub_datasets = SOPC_PubSubConfiguration_Nb_PublishedDataSet(config);
    ck_assert_uint_eq(3, nb_pub_datasets);

    SOPC_PubSubConnection* conn = SOPC_PubSubConfiguration_Get_PubConnection_At(config, 0);
    ck_assert_ptr_nonnull(conn);
    ck_assert_str_eq("opc.udp://232.1.2.100:4840", SOPC_PubSubConnection_Get_Address(conn));
    const SOPC_Conf_PublisherId* pubId = SOPC_PubSubConnection_Get_PublisherId(conn);
    ck_assert_int_eq(SOPC_UInteger_PublisherId, pubId->type);
    ck_assert_uint_eq(123, pubId->data.uint);
    ck_assert_ptr_null(SOPC_PubSubConnection_Get_Name(conn));
    ck_assert_int_eq(false, SOPC_PubSubConnection_Is_Enabled(conn));
    ck_assert_ptr_null(SOPC_PubSubConnection_Get_TransportProfileUri(conn)); // TODO ? Is there an URI to set ?
    const uint16_t nb_readerGroups = SOPC_PubSubConnection_Nb_ReaderGroup(conn);
    ck_assert_uint_eq(0, nb_readerGroups);
    const uint16_t nb_writerGroups = SOPC_PubSubConnection_Nb_WriterGroup(conn);
    ck_assert_uint_eq(2, nb_writerGroups);

    /* First WriteGroup */
    SOPC_WriterGroup* writerGroup = SOPC_PubSubConnection_Get_WriterGroup_At(conn, 0);
    ck_assert_ptr_nonnull(writerGroup);
    ck_assert_ptr_eq(conn, SOPC_WriterGroup_Get_Connection(writerGroup));
    ck_assert_uint_eq(14, SOPC_WriterGroup_Get_Id(writerGroup));
    // TODO: SOPC_WriterGroup_Get_NetworkMessageContentMask() ?
    ck_assert_double_eq(50., SOPC_WriterGroup_Get_PublishingInterval(writerGroup));
    ck_assert_int_eq(10, SOPC_WriterGroup_Get_PublishingOffset(writerGroup));
    ck_assert_uint_eq(1, SOPC_WriterGroup_Get_Version(writerGroup));
    ck_assert_uint_eq(SOPC_SecurityMode_None, SOPC_WriterGroup_Get_SecurityMode(writerGroup));
    ck_assert_uint_eq(2, SOPC_WriterGroup_Nb_DataSetWriter(writerGroup));
    // DataSetWriter 1
    SOPC_DataSetWriter* dataSetWriter = SOPC_WriterGroup_Get_DataSetWriter_At(writerGroup, 0);
    ck_assert_ptr_nonnull(dataSetWriter);
    ck_assert_uint_eq(50, SOPC_DataSetWriter_Get_Id(dataSetWriter));
    const SOPC_PublishedDataSet* pubDataSet1 = SOPC_DataSetWriter_Get_DataSet(dataSetWriter);
    ck_assert_ptr_eq(pubDataSet1, SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, 0));

    // DataSetWriter 2
    dataSetWriter = SOPC_WriterGroup_Get_DataSetWriter_At(writerGroup, 1);
    ck_assert_ptr_nonnull(dataSetWriter);
    ck_assert_uint_eq(51, SOPC_DataSetWriter_Get_Id(dataSetWriter));
    const SOPC_PublishedDataSet* pubDataSet2 = SOPC_DataSetWriter_Get_DataSet(dataSetWriter);
    ck_assert_ptr_eq(pubDataSet2, SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, 1));

    /* Second WriteGroup */
    writerGroup = SOPC_PubSubConnection_Get_WriterGroup_At(conn, 1);
    ck_assert_ptr_nonnull(writerGroup);
    ck_assert_ptr_eq(conn, SOPC_WriterGroup_Get_Connection(writerGroup));
    ck_assert_uint_eq(15, SOPC_WriterGroup_Get_Id(writerGroup));
    // TODO: SOPC_WriterGroup_Get_NetworkMessageContentMask() ?
    ck_assert_double_eq(30., SOPC_WriterGroup_Get_PublishingInterval(writerGroup));
    ck_assert_int_eq(-1, SOPC_WriterGroup_Get_PublishingOffset(writerGroup));
    // Check default value for groupVersion
    ck_assert_uint_eq(0, SOPC_WriterGroup_Get_Version(writerGroup));
    ck_assert_uint_eq(SOPC_SecurityMode_SignAndEncrypt, SOPC_WriterGroup_Get_SecurityMode(writerGroup));
    ck_assert_uint_eq(1, SOPC_WriterGroup_Nb_DataSetWriter(writerGroup));
    // DataSetWriter
    dataSetWriter = SOPC_WriterGroup_Get_DataSetWriter_At(writerGroup, 0);
    ck_assert_uint_eq(52, SOPC_DataSetWriter_Get_Id(dataSetWriter));
    const SOPC_PublishedDataSet* pubDataSet3 = SOPC_DataSetWriter_Get_DataSet(dataSetWriter);
    ck_assert_ptr_eq(pubDataSet3, SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, 2));

    /*****************************/
    /* Published Data Sets */
    /* First published data set */
    uint32_t nb_fields = SOPC_PublishedDataSet_Nb_FieldMetaData(pubDataSet1);
    ck_assert_uint_eq(1, nb_fields);
    SOPC_FieldMetaData* fieldMetaData = SOPC_PublishedDataSet_Get_FieldMetaData_At(pubDataSet1, 0);
    ck_assert_int_eq(SOPC_Boolean_Id, SOPC_FieldMetaData_Get_BuiltinType(fieldMetaData));
    ck_assert_int_eq(-1, SOPC_FieldMetaData_Get_ValueRank(fieldMetaData));
    // Variable source
    SOPC_PublishedVariable* pubVar = SOPC_FieldMetaData_Get_PublishedVariable(fieldMetaData);
    ck_assert_uint_eq(13, SOPC_PublishedVariable_Get_AttributeId(pubVar)); // Value => AttributeId = 13
    const SOPC_NodeId* nodeId = SOPC_PublishedVariable_Get_NodeId(pubVar);
    ck_assert_int_eq(SOPC_IdentifierType_Numeric, nodeId->IdentifierType);
    ck_assert_int_eq(1, nodeId->Namespace);
    ck_assert_int_eq(5, nodeId->Data.Numeric);
    ck_assert_ptr_null(SOPC_PublishedVariable_Get_IndexRange(pubVar)); // No index range in XML

    /*****************************/
    /* Second published data set */
    nb_fields = SOPC_PublishedDataSet_Nb_FieldMetaData(pubDataSet2);
    ck_assert_uint_eq(2, nb_fields);
    fieldMetaData = SOPC_PublishedDataSet_Get_FieldMetaData_At(pubDataSet2, 0);
    ck_assert_int_eq(SOPC_UInt32_Id, SOPC_FieldMetaData_Get_BuiltinType(fieldMetaData));
    ck_assert_int_eq(-1, SOPC_FieldMetaData_Get_ValueRank(fieldMetaData));

    pubVar = SOPC_FieldMetaData_Get_PublishedVariable(fieldMetaData);
    ck_assert_uint_eq(13, SOPC_PublishedVariable_Get_AttributeId(pubVar)); // Value => AttributeId = 13
    nodeId = SOPC_PublishedVariable_Get_NodeId(pubVar);
    ck_assert_int_eq(SOPC_IdentifierType_Numeric, nodeId->IdentifierType);
    ck_assert_int_eq(2, nodeId->Namespace);
    ck_assert_int_eq(6, nodeId->Data.Numeric);
    ck_assert_ptr_null(SOPC_PublishedVariable_Get_IndexRange(pubVar)); // No index range in XML

    /* Second variable */
    fieldMetaData = SOPC_PublishedDataSet_Get_FieldMetaData_At(pubDataSet2, 1);
    ck_assert_int_eq(SOPC_UInt16_Id, SOPC_FieldMetaData_Get_BuiltinType(fieldMetaData));
    ck_assert_int_eq(-1, SOPC_FieldMetaData_Get_ValueRank(fieldMetaData));

    pubVar = SOPC_FieldMetaData_Get_PublishedVariable(fieldMetaData);
    ck_assert_uint_eq(13, SOPC_PublishedVariable_Get_AttributeId(pubVar)); // Value => AttributeId = 13
    nodeId = SOPC_PublishedVariable_Get_NodeId(pubVar);
    ck_assert_int_eq(SOPC_IdentifierType_Numeric, nodeId->IdentifierType);
    ck_assert_int_eq(2, nodeId->Namespace);
    ck_assert_int_eq(7, nodeId->Data.Numeric);
    ck_assert_ptr_null(SOPC_PublishedVariable_Get_IndexRange(pubVar)); // No index range in XML

    /*****************************/
    /* Third published data set */
    nb_fields = SOPC_PublishedDataSet_Nb_FieldMetaData(pubDataSet3);
    ck_assert_uint_eq(1, nb_fields);
    fieldMetaData = SOPC_PublishedDataSet_Get_FieldMetaData_At(pubDataSet3, 0);
    ck_assert_int_eq(SOPC_Int16_Id, SOPC_FieldMetaData_Get_BuiltinType(fieldMetaData));
    ck_assert_int_eq(-1, SOPC_FieldMetaData_Get_ValueRank(fieldMetaData));

    pubVar = SOPC_FieldMetaData_Get_PublishedVariable(fieldMetaData);
    ck_assert_uint_eq(13, SOPC_PublishedVariable_Get_AttributeId(pubVar)); // Value => AttributeId = 13
    nodeId = SOPC_PublishedVariable_Get_NodeId(pubVar);
    ck_assert_int_eq(SOPC_IdentifierType_Numeric, nodeId->IdentifierType);
    ck_assert_int_eq(1, nodeId->Namespace);
    ck_assert_int_eq(2, nodeId->Data.Numeric);
    ck_assert_ptr_null(SOPC_PublishedVariable_Get_IndexRange(pubVar)); // No index range in XML

    // Delete config
    SOPC_PubSubConfiguration_Delete(config);
}
END_TEST

START_TEST(test_sub_xml_parsing)
{
    fprintf(stderr, "opening 'config_sub.xml'\n"); // TODO JCH
    FILE* fd = fopen("./config_sub.xml", "r");
    ck_assert_ptr_nonnull(fd);

    SOPC_PubSubConfiguration* config = SOPC_PubSubConfig_ParseXML(fd);
    int closed = fclose(fd);
    ck_assert_int_eq(0, closed);

    const uint32_t nb_pubconnections = SOPC_PubSubConfiguration_Nb_PubConnection(config);
    ck_assert_uint_eq(0, nb_pubconnections);
    const uint32_t nb_subconnections = SOPC_PubSubConfiguration_Nb_SubConnection(config);
    ck_assert_uint_eq(1, nb_subconnections);

    ck_assert_uint_eq(0, SOPC_PubSubConfiguration_Nb_PublishedDataSet(config)); // No PublishedDataSets in Subscriber

    SOPC_PubSubConnection* conn = SOPC_PubSubConfiguration_Get_SubConnection_At(config, 0);
    ck_assert_ptr_nonnull(conn);
    ck_assert_str_eq("opc.udp://232.1.2.100:4840", SOPC_PubSubConnection_Get_Address(conn));

    const SOPC_Conf_PublisherId* pubId = SOPC_PubSubConnection_Get_PublisherId(conn);
    ck_assert_int_eq(SOPC_Null_PublisherId, pubId->type);
    ck_assert_uint_eq(0, pubId->data.uint); // No publisherId associated to connection in Subscriber (default value 0)

    ck_assert_ptr_null(SOPC_PubSubConnection_Get_Name(conn));
    ck_assert_int_eq(false, SOPC_PubSubConnection_Is_Enabled(conn));
    ck_assert_ptr_null(SOPC_PubSubConnection_Get_TransportProfileUri(conn)); // TODO ? Is there an URI to set ?

    const uint16_t nb_writerGroups = SOPC_PubSubConnection_Nb_WriterGroup(conn);
    ck_assert_uint_eq(0, nb_writerGroups);

    const uint16_t nb_readerGroups = SOPC_PubSubConnection_Nb_ReaderGroup(conn);
    ck_assert_uint_eq(2, nb_readerGroups);

    /* First ReaderGroup */
    SOPC_ReaderGroup* readerGroup = SOPC_PubSubConnection_Get_ReaderGroup_At(conn, 0);
    ck_assert_ptr_nonnull(readerGroup);
    ck_assert_uint_eq(SOPC_SecurityMode_None, SOPC_ReaderGroup_Get_SecurityMode(readerGroup));
    SOPC_DataSetReader* dataSetReader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, 0);
    ck_assert_ptr_nonnull(dataSetReader);

    ck_assert_uint_eq(1, SOPC_ReaderGroup_Get_GroupVersion(readerGroup));
    uint16_t writerGroupId = SOPC_ReaderGroup_Get_GroupId(readerGroup);
    ck_assert_uint_eq(14, writerGroupId); // message Id
    uint16_t writerId = SOPC_DataSetReader_Get_DataSetWriterId(dataSetReader);
    ck_assert_uint_eq(0, writerId); // same as writeGroupId

    double timeoutMs = SOPC_DataSetReader_Get_ReceiveTimeout(dataSetReader);
    ck_assert_double_eq(2 * 50., timeoutMs); // 2*publishingInternval

    ck_assert_int_eq(SOPC_TargetVariablesDataType, SOPC_DataSetReader_Get_DataSet_TargetType(dataSetReader));

    pubId = SOPC_ReaderGroup_Get_PublisherId(readerGroup);
    ck_assert_int_eq(SOPC_UInteger_PublisherId, pubId->type);
    ck_assert_uint_eq(123, pubId->data.uint);

    // Variable type
    SOPC_FieldMetaData* fieldMetaData = SOPC_DataSetReader_Get_FieldMetaData_At(dataSetReader, 0);
    ck_assert_int_eq(-1, SOPC_FieldMetaData_Get_ValueRank(fieldMetaData));
    ck_assert_int_eq(SOPC_Boolean_Id, SOPC_FieldMetaData_Get_BuiltinType(fieldMetaData));

    // Variable target
    SOPC_FieldTarget* fieldTarget = SOPC_FieldMetaData_Get_TargetVariable(fieldMetaData);
    ck_assert_uint_eq(13, SOPC_FieldTarget_Get_AttributeId(fieldTarget)); // Value => AttributeId = 13
    const SOPC_NodeId* nodeId = SOPC_FieldTarget_Get_NodeId(fieldTarget);
    ck_assert_int_eq(SOPC_IdentifierType_String, nodeId->IdentifierType);
    ck_assert_int_eq(1, nodeId->Namespace);
    ck_assert_str_eq("Toto", SOPC_String_GetRawCString(&nodeId->Data.String));

    ck_assert_ptr_null(SOPC_FieldTarget_Get_SourceIndexRange(fieldTarget));
    ck_assert_ptr_null(SOPC_FieldTarget_Get_TargetIndexRange(fieldTarget));

    /* Second Dataset Reader of first group */
    dataSetReader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, 1);
    ck_assert_ptr_nonnull(dataSetReader);

    ck_assert_uint_eq(1, SOPC_ReaderGroup_Get_GroupVersion(readerGroup));
    writerGroupId = SOPC_ReaderGroup_Get_GroupId(readerGroup);
    ck_assert_uint_eq(14, writerGroupId); // message Id
    writerId = SOPC_DataSetReader_Get_DataSetWriterId(dataSetReader);
    ck_assert_uint_eq(0, writerId); // same as writeGroupId (default value)

    timeoutMs = SOPC_DataSetReader_Get_ReceiveTimeout(dataSetReader);
    ck_assert_double_eq(2 * 50., timeoutMs); // 2*publishingInternval

    ck_assert_int_eq(SOPC_TargetVariablesDataType, SOPC_DataSetReader_Get_DataSet_TargetType(dataSetReader));

    pubId = SOPC_ReaderGroup_Get_PublisherId(readerGroup);
    ck_assert_int_eq(SOPC_UInteger_PublisherId, pubId->type);
    ck_assert_uint_eq(123, pubId->data.uint);

    // Variable type
    fieldMetaData = SOPC_DataSetReader_Get_FieldMetaData_At(dataSetReader, 0);
    ck_assert_int_eq(-1, SOPC_FieldMetaData_Get_ValueRank(fieldMetaData));
    ck_assert_int_eq(SOPC_UInt32_Id, SOPC_FieldMetaData_Get_BuiltinType(fieldMetaData));

    // Variable target
    fieldTarget = SOPC_FieldMetaData_Get_TargetVariable(fieldMetaData);
    ck_assert_uint_eq(13, SOPC_FieldTarget_Get_AttributeId(fieldTarget)); // Value => AttributeId = 13
    nodeId = SOPC_FieldTarget_Get_NodeId(fieldTarget);
    ck_assert_int_eq(SOPC_IdentifierType_Numeric, nodeId->IdentifierType);
    ck_assert_int_eq(2, nodeId->Namespace);
    ck_assert_int_eq(6, nodeId->Data.Numeric);

    ck_assert_ptr_null(SOPC_FieldTarget_Get_SourceIndexRange(fieldTarget));
    ck_assert_ptr_null(SOPC_FieldTarget_Get_TargetIndexRange(fieldTarget));

    // Variable type
    fieldMetaData = SOPC_DataSetReader_Get_FieldMetaData_At(dataSetReader, 1);
    ck_assert_int_eq(-1, SOPC_FieldMetaData_Get_ValueRank(fieldMetaData));
    ck_assert_int_eq(SOPC_UInt16_Id, SOPC_FieldMetaData_Get_BuiltinType(fieldMetaData));

    // Variable target
    fieldTarget = SOPC_FieldMetaData_Get_TargetVariable(fieldMetaData);
    ck_assert_uint_eq(13, SOPC_FieldTarget_Get_AttributeId(fieldTarget)); // Value => AttributeId = 13
    nodeId = SOPC_FieldTarget_Get_NodeId(fieldTarget);
    ck_assert_int_eq(SOPC_IdentifierType_Numeric, nodeId->IdentifierType);
    ck_assert_int_eq(2, nodeId->Namespace);
    ck_assert_int_eq(7, nodeId->Data.Numeric);

    ck_assert_ptr_null(SOPC_FieldTarget_Get_SourceIndexRange(fieldTarget));
    ck_assert_ptr_null(SOPC_FieldTarget_Get_TargetIndexRange(fieldTarget));

    /////////////////
    // Second group
    readerGroup = SOPC_PubSubConnection_Get_ReaderGroup_At(conn, 1);
    ck_assert_ptr_nonnull(readerGroup);
    ck_assert_uint_eq(SOPC_SecurityMode_Sign, SOPC_ReaderGroup_Get_SecurityMode(readerGroup));
    dataSetReader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, 0);
    ck_assert_ptr_nonnull(dataSetReader);

    ck_assert_uint_eq(1, SOPC_ReaderGroup_Get_GroupVersion(readerGroup));
    writerGroupId = SOPC_ReaderGroup_Get_GroupId(readerGroup);
    ck_assert_uint_eq(15, writerGroupId); // message Id
    writerId = SOPC_DataSetReader_Get_DataSetWriterId(dataSetReader);
    ck_assert_uint_eq(52, writerId); // same as writeGroupId

    timeoutMs = SOPC_DataSetReader_Get_ReceiveTimeout(dataSetReader);
    ck_assert_double_eq(2 * 100., timeoutMs); // 2*publishingInternval

    ck_assert_int_eq(SOPC_TargetVariablesDataType, SOPC_DataSetReader_Get_DataSet_TargetType(dataSetReader));

    pubId = SOPC_ReaderGroup_Get_PublisherId(readerGroup);
    ck_assert_int_eq(SOPC_UInteger_PublisherId, pubId->type);
    ck_assert_uint_eq(456, pubId->data.uint);

    // Variable type
    fieldMetaData = SOPC_DataSetReader_Get_FieldMetaData_At(dataSetReader, 0);
    ck_assert_int_eq(-1, SOPC_FieldMetaData_Get_ValueRank(fieldMetaData));
    ck_assert_int_eq(SOPC_Int16_Id, SOPC_FieldMetaData_Get_BuiltinType(fieldMetaData));

    // Variable target
    fieldTarget = SOPC_FieldMetaData_Get_TargetVariable(fieldMetaData);
    ck_assert_uint_eq(13, SOPC_FieldTarget_Get_AttributeId(fieldTarget)); // Value => AttributeId = 13
    nodeId = SOPC_FieldTarget_Get_NodeId(fieldTarget);
    ck_assert_int_eq(SOPC_IdentifierType_String, nodeId->IdentifierType);
    ck_assert_int_eq(1, nodeId->Namespace);
    ck_assert_str_eq("Tata", SOPC_String_GetRawCString(&nodeId->Data.String));

    ck_assert_ptr_null(SOPC_FieldTarget_Get_SourceIndexRange(fieldTarget));
    ck_assert_ptr_null(SOPC_FieldTarget_Get_TargetIndexRange(fieldTarget));

    // Delete config
    SOPC_PubSubConfiguration_Delete(config);
}
END_TEST

int main(void)
{
    int number_failed;
    SRunner* sr;

    Suite* suite = suite_create("PubSub XML parsing tests");

    TCase* tc_pub_xml_parsing = tcase_create("Publisher XML parsing test");
    suite_add_tcase(suite, tc_pub_xml_parsing);
    tcase_add_test(tc_pub_xml_parsing, test_pub_xml_parsing);

    TCase* tc_sub_xml_parsing = tcase_create("Subscriber XML parsing test");
    suite_add_tcase(suite, tc_sub_xml_parsing);
    tcase_add_test(tc_sub_xml_parsing, test_sub_xml_parsing);

    sr = srunner_create(suite);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
