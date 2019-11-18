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

/** \file
 *
 * \brief Entry point for threads tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>
#include <stdio.h>

#include "check_helpers.h"

#include "embedded/sopc_addspace_loader.h"
#ifdef WITH_EXPAT
#include "xml_expat/sopc_config_loader.h"
#include "xml_expat/sopc_uanodeset_loader.h"
#endif

#include "sopc_macros.h"
#include "sopc_user_app_itf.h"

#define XML_UA_NODESET_NAME "s2opc.xml"
#define XML_SRV_CONFIG_NAME "S2OPC_Test_XML_Config.xml"

// Avoid unused functions and variables when EXPAT is not available
#ifdef WITH_EXPAT

static const int64_t SOPC_SECOND_TO_100_NANOSECONDS = 10000000; // 10^7

static void check_variable_and_type_common(SOPC_AddressSpace* leftSpace,
                                           SOPC_AddressSpace_Node* left,
                                           SOPC_AddressSpace* rightSpace,
                                           SOPC_AddressSpace_Node* right)
{
    /* Check value metadata (should be only valid for Variable/VariableType) */
    if (!SOPC_AddressSpace_AreReadOnlyNodes(leftSpace) && !SOPC_AddressSpace_AreReadOnlyNodes(rightSpace))
    {
        ck_assert_uint_eq(SOPC_AddressSpace_Get_StatusCode(leftSpace, left),
                          SOPC_AddressSpace_Get_StatusCode(rightSpace, right));

        // Note: timestamps are dynamically set to current date in both case => not the exact same date
        // Check delta < 1 second  between the 2 timestamps (=> do not check picoSeconds)
        SOPC_Value_Timestamp left_ts = SOPC_AddressSpace_Get_SourceTs(leftSpace, left);
        SOPC_Value_Timestamp right_ts = SOPC_AddressSpace_Get_SourceTs(rightSpace, right);
        if (left_ts.timestamp > right_ts.timestamp)
        {
            ck_assert_int_gt(SOPC_SECOND_TO_100_NANOSECONDS, left_ts.timestamp - right_ts.timestamp);
        }
        else
        {
            ck_assert_int_gt(SOPC_SECOND_TO_100_NANOSECONDS, right_ts.timestamp - left_ts.timestamp);
        }
    }

    if (SOPC_AddressSpace_AreReadOnlyNodes(leftSpace))
    {
        ck_assert_uint_eq(SOPC_GoodGenericStatus, SOPC_AddressSpace_Get_StatusCode(leftSpace, left));
    }

    if (SOPC_AddressSpace_AreReadOnlyNodes(rightSpace))
    {
        ck_assert_uint_eq(SOPC_GoodGenericStatus, SOPC_AddressSpace_Get_StatusCode(rightSpace, right));
    }

    int32_t compare = -1;

    SOPC_ReturnStatus status = SOPC_Variant_Compare(SOPC_AddressSpace_Get_Value(leftSpace, left),
                                                    SOPC_AddressSpace_Get_Value(rightSpace, right), &compare);
    // Note: we can't compare value if comparison not supported
    if (SOPC_STATUS_NOT_SUPPORTED != status)
    {
        ck_assert_int_eq(SOPC_STATUS_OK, status);
    }
    ck_assert_int_eq(0, compare);

    status = SOPC_NodeId_Compare(SOPC_AddressSpace_Get_DataType(leftSpace, left),
                                 SOPC_AddressSpace_Get_DataType(rightSpace, right), &compare);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, compare);

    ck_assert_int_eq(*SOPC_AddressSpace_Get_ValueRank(leftSpace, left),
                     *SOPC_AddressSpace_Get_ValueRank(rightSpace, right));

    ck_assert_int_eq(SOPC_AddressSpace_Get_NoOfArrayDimensions(leftSpace, left),
                     SOPC_AddressSpace_Get_NoOfArrayDimensions(rightSpace, right));

    uint32_t* left_arr = SOPC_AddressSpace_Get_ArrayDimensions(leftSpace, left);
    uint32_t* right_arr = SOPC_AddressSpace_Get_ArrayDimensions(rightSpace, right);
    for (int32_t i = 0; i < SOPC_AddressSpace_Get_NoOfArrayDimensions(leftSpace, left); i++)
    {
        ck_assert_uint_eq(left_arr[i], right_arr[i]);
    }
}

static void addspace_for_each_equal(const void* key, const void* value, void* user_data)
{
    // Uncomment for debugging purpose:
    /*
    const SOPC_NodeId* id = key;
    #include <stdio.h>
    printf("Checking node: %s\n", SOPC_NodeId_ToCString(id));
    */
    bool found = false;
    /* Note: we do not have read-only accessors for SOPC_AddressSpace_Node even if in this case we do not modify
     * accessed values */
    SOPC_AddressSpace** addSpaces = user_data;

    SOPC_AddressSpace* leftSpace = addSpaces[0];
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_AddressSpace_Node* left = (SOPC_AddressSpace_Node*) value;
    SOPC_GCC_DIAGNOSTIC_RESTORE

    SOPC_AddressSpace* rightSpace = addSpaces[1];
    SOPC_AddressSpace_Node* right = SOPC_AddressSpace_Get_Node(rightSpace, key, &found);
    ck_assert(found);

    /* Check item type */
    ck_assert_int_eq(left->node_class, right->node_class);

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    int32_t compare = -1;

    /* Check common attributes (NodeClass, NodeId, BrowseName, DisplayName, Description, *WriteMask) */
    status = SOPC_NodeId_Compare(SOPC_AddressSpace_Get_NodeId(leftSpace, left),
                                 SOPC_AddressSpace_Get_NodeId(rightSpace, right), &compare);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, compare);

    ck_assert_int_eq(*SOPC_AddressSpace_Get_NodeClass(leftSpace, left),
                     *SOPC_AddressSpace_Get_NodeClass(rightSpace, right));

    status = SOPC_QualifiedName_Compare(SOPC_AddressSpace_Get_BrowseName(leftSpace, left),
                                        SOPC_AddressSpace_Get_BrowseName(rightSpace, right), &compare);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, compare);

    status = SOPC_LocalizedText_Compare(SOPC_AddressSpace_Get_DisplayName(leftSpace, left),
                                        SOPC_AddressSpace_Get_DisplayName(rightSpace, right), &compare);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, compare);

    status = SOPC_LocalizedText_Compare(SOPC_AddressSpace_Get_Description(leftSpace, left),
                                        SOPC_AddressSpace_Get_Description(rightSpace, right), &compare);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, compare);

    ck_assert_uint_eq(*SOPC_AddressSpace_Get_WriteMask(leftSpace, left),
                      *SOPC_AddressSpace_Get_WriteMask(rightSpace, right));

    ck_assert_uint_eq(*SOPC_AddressSpace_Get_UserWriteMask(leftSpace, left),
                      *SOPC_AddressSpace_Get_UserWriteMask(rightSpace, right));

    /* Check References */
    ck_assert_int_eq(*SOPC_AddressSpace_Get_NoOfReferences(leftSpace, left),
                     *SOPC_AddressSpace_Get_NoOfReferences(rightSpace, right));

    OpcUa_ReferenceNode* left_refs = *SOPC_AddressSpace_Get_References(leftSpace, left);
    OpcUa_ReferenceNode* right_refs = *SOPC_AddressSpace_Get_References(rightSpace, right);

    for (int32_t i = 0; i < *SOPC_AddressSpace_Get_NoOfReferences(leftSpace, left); i++)
    {
        ck_assert_uint_eq(left_refs[i].IsInverse, right_refs[i].IsInverse);

        status = SOPC_NodeId_Compare(&left_refs[i].ReferenceTypeId, &right_refs[i].ReferenceTypeId, &compare);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        ck_assert_int_eq(0, compare);

        status = SOPC_ExpandedNodeId_Compare(&left_refs[i].TargetId, &right_refs[i].TargetId, &compare);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        ck_assert_int_eq(0, compare);
    }

    /* Check specific attributes */
    switch (left->node_class)
    {
    case OpcUa_NodeClass_Object:
        ck_assert_uint_eq(left->data.object.EventNotifier, right->data.object.EventNotifier);
        break;
    case OpcUa_NodeClass_Variable:
        check_variable_and_type_common(leftSpace, left, rightSpace, right);

        ck_assert_uint_eq(SOPC_AddressSpace_Get_AccessLevel(leftSpace, left),
                          SOPC_AddressSpace_Get_AccessLevel(rightSpace, right));

        ck_assert_uint_eq(left->data.variable.UserAccessLevel, right->data.variable.UserAccessLevel);

        ck_assert_double_eq(left->data.variable.MinimumSamplingInterval, right->data.variable.MinimumSamplingInterval);

        ck_assert_uint_eq(left->data.variable.Historizing, right->data.variable.Historizing);
        break;
    case OpcUa_NodeClass_Method:
        ck_assert_uint_eq(left->data.method.Executable, right->data.method.Executable);

        ck_assert_uint_eq(left->data.method.UserExecutable, right->data.method.UserExecutable);
        break;
    case OpcUa_NodeClass_ObjectType:
        ck_assert_uint_eq(left->data.object_type.IsAbstract, right->data.object_type.IsAbstract);
        break;
    case OpcUa_NodeClass_VariableType:
        check_variable_and_type_common(leftSpace, left, rightSpace, right);

        ck_assert_uint_eq(left->data.variable_type.IsAbstract, right->data.variable_type.IsAbstract);
        break;
    case OpcUa_NodeClass_ReferenceType:
        ck_assert_uint_eq(left->data.reference_type.Symmetric, right->data.reference_type.Symmetric);

        status = SOPC_LocalizedText_Compare(&left->data.reference_type.InverseName,
                                            &right->data.reference_type.InverseName, &compare);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        ck_assert_int_eq(0, compare);

        ck_assert_uint_eq(left->data.reference_type.IsAbstract, right->data.reference_type.IsAbstract);
        break;
    case OpcUa_NodeClass_DataType:
        ck_assert_uint_eq(left->data.data_type.IsAbstract, right->data.data_type.IsAbstract);
        break;
    case OpcUa_NodeClass_View:
        ck_assert_uint_eq(left->data.view.ContainsNoLoops, right->data.view.ContainsNoLoops);

        ck_assert_uint_eq(left->data.view.EventNotifier, right->data.view.EventNotifier);
        break;
    default:
        ck_assert(false);
    }
}

#endif

START_TEST(test_same_address_space_results)
{
// Without EXPAT test cannot be done
#ifdef WITH_EXPAT

    /* Embedded address space (parsed prior to compilation) */
    SOPC_AddressSpace* spaceEmbedded = SOPC_Embedded_AddressSpace_Load();

    /* Dynamic parsing of address space */
    FILE* fd = fopen(XML_UA_NODESET_NAME, "r");

    ck_assert_ptr_nonnull(fd);

    SOPC_AddressSpace* spaceDynamic = SOPC_UANodeSet_Parse(fd);
    ck_assert_ptr_nonnull(spaceDynamic);
    fclose(fd);

    /* Check all data present in embedded are present in dynamic */
    SOPC_AddressSpace* spaces[2] = {spaceEmbedded, spaceDynamic};
    SOPC_AddressSpace_ForEach(spaceEmbedded, addspace_for_each_equal, spaces);

    /* Check all data present in dynamic are present in embedded */
    SOPC_AddressSpace* spaces2[2] = {spaceDynamic, spaceEmbedded};
    SOPC_AddressSpace_ForEach(spaceDynamic, addspace_for_each_equal, spaces2);

    SOPC_AddressSpace_Delete(spaceEmbedded);
    SOPC_AddressSpace_Delete(spaceDynamic);
#else
    printf("Test test_same_address_space_results ignored since EXPAT is not available\n");
#endif
}
END_TEST

const char* expectedNamespaces[3] = {"urn:S2OPC:MY_SERVER_HOST", "urn:S2OPC:MY_SERVER_HOST:2", NULL};
const char* expectedLocales[4] = {"en", "es-ES", "fr-FR", NULL};

// Without EXPAT function is detected as unused and compilation fails
#ifdef WITH_EXPAT
static void check_parsed_s2opc_config(SOPC_S2OPC_Config* s2opcConfig)
{
    SOPC_Server_Config* sConfig = &s2opcConfig->serverConfig;
    /* Check namespaces */
    int nsCounter = 0;
    while (sConfig->namespaces[nsCounter] != NULL && expectedNamespaces[nsCounter] != NULL)
    {
        ck_assert_int_eq(0, strcmp(sConfig->namespaces[nsCounter], expectedNamespaces[nsCounter]));
        nsCounter++;
    }
    ck_assert_ptr_null(sConfig->namespaces[nsCounter]);
    ck_assert_ptr_null(expectedNamespaces[nsCounter]);

    /* Check locales */
    int localeCounter = 0;
    while (sConfig->localeIds[localeCounter] != NULL && expectedLocales[localeCounter] != NULL)
    {
        ck_assert_int_eq(0, strcmp(sConfig->localeIds[localeCounter], expectedLocales[localeCounter]));
        localeCounter++;
    }
    ck_assert_ptr_null(sConfig->localeIds[localeCounter]);
    ck_assert_ptr_null(expectedLocales[localeCounter]);

    /* Check application certificates */
    ck_assert_int_eq(0, strcmp("/mypath/mycert.der", sConfig->serverCertPath));
    ck_assert_int_eq(0, strcmp("/mypath/mykey.pem", sConfig->serverKeyPath));
    ck_assert_int_eq(0, strcmp("/mypath/cacert.der", sConfig->certificateAuthorityPath));

    /* Check application description */
    int res =
        strcmp("urn:S2OPC:MY_SERVER_HOST:app", SOPC_String_GetRawCString(&sConfig->serverDescription.ApplicationUri));
    ck_assert_int_eq(0, res);
    res = strcmp("urn:S2OPC:MY_SERVER_HOST:prod", SOPC_String_GetRawCString(&sConfig->serverDescription.ProductUri));
    ck_assert_int_eq(0, res);
    /* Application names */
    // default name
    res = strcmp("S2OPC toolkit configuration example",
                 SOPC_String_GetRawCString(&sConfig->serverDescription.ApplicationName.defaultText));
    ck_assert_int_eq(0, res);
    res = strcmp("en", SOPC_String_GetRawCString(&sConfig->serverDescription.ApplicationName.defaultLocale));
    ck_assert_int_eq(0, res);

    // other names
    localeCounter = 1;
    ck_assert_ptr_nonnull(sConfig->serverDescription.ApplicationName.localizedTextList);
    SOPC_SLinkedListIterator it =
        SOPC_SLinkedList_GetIterator(sConfig->serverDescription.ApplicationName.localizedTextList);
    while (SOPC_SLinkedList_HasNext(&it))
    {
        localeCounter++;
        SOPC_LocalizedText* lt = SOPC_SLinkedList_Next(&it);
        ck_assert_ptr_nonnull(lt);
        if (2 == localeCounter)
        {
            res = strcmp("S2OPC toolkit: ejemplo de configuración", SOPC_String_GetRawCString(&lt->defaultText));
            ck_assert_int_eq(0, res);
            res = strcmp("es-ES", SOPC_String_GetRawCString(&lt->defaultLocale));
            ck_assert_int_eq(0, res);
        }
        else if (3 == localeCounter)
        {
            res = strcmp("S2OPC toolkit: exemple de configuration", SOPC_String_GetRawCString(&lt->defaultText));
            ck_assert_int_eq(0, res);
            res = strcmp("fr-FR", SOPC_String_GetRawCString(&lt->defaultLocale));
            ck_assert_int_eq(0, res);
        }
    }
    ck_assert_int_eq(3, localeCounter);

    /* Check endpoints */
    ck_assert_uint_eq(2, sConfig->nbEndpoints);

    /* 1st endpoint */
    SOPC_Endpoint_Config* epConfig = &sConfig->endpoints[0];
    ck_assert_ptr_eq(epConfig->serverConfigPtr, sConfig);
    ck_assert_int_eq(0, strcmp("opc.tcp://MY_SERVER_HOST:4841/MY_ENPOINT_NAME", epConfig->endpointURL));

    /* Check security policies */
    ck_assert_uint_eq(3, epConfig->nbSecuConfigs);

    /* 1st secu policy */
    SOPC_SecurityPolicy* secuPolicy = &epConfig->secuConfigurations[0];
    int strEqual = strcmp("http://opcfoundation.org/UA/SecurityPolicy#None",
                          SOPC_String_GetRawCString(&secuPolicy->securityPolicy));
    ck_assert_int_eq(0, strEqual);
    ck_assert_int_eq(secuPolicy->securityModes & SOPC_SECURITY_MODE_NONE_MASK, SOPC_SECURITY_MODE_NONE_MASK);
    ck_assert_int_eq(secuPolicy->securityModes | SOPC_SECURITY_MODE_NONE_MASK, SOPC_SECURITY_MODE_NONE_MASK);
    ck_assert_uint_eq(1, secuPolicy->nbOfUserTokenPolicies);

    OpcUa_UserTokenPolicy* userPolicy = &secuPolicy->userTokenPolicies[0];
    ck_assert_int_ge(0, userPolicy->IssuedTokenType.Length);
    ck_assert_int_ge(0, userPolicy->IssuerEndpointUrl.Length);
    ck_assert_int_eq(0, strcmp("anon", SOPC_String_GetRawCString(&userPolicy->PolicyId)));
    ck_assert_int_eq(OpcUa_UserTokenType_Anonymous, userPolicy->TokenType);
    ck_assert_int_ge(0, userPolicy->SecurityPolicyUri.Length);

    /* 2nd secu policy */
    secuPolicy = &epConfig->secuConfigurations[1];
    strEqual = strcmp("http://opcfoundation.org/UA/SecurityPolicy#Basic256",
                      SOPC_String_GetRawCString(&secuPolicy->securityPolicy));
    ck_assert_int_eq(0, strEqual);
    // Support None security mode
    ck_assert_int_eq(secuPolicy->securityModes & SOPC_SECURITY_MODE_SIGN_MASK, SOPC_SECURITY_MODE_SIGN_MASK);
    // And only None security mode
    ck_assert_int_eq(secuPolicy->securityModes | SOPC_SECURITY_MODE_SIGN_MASK, SOPC_SECURITY_MODE_SIGN_MASK);
    ck_assert_uint_eq(1, secuPolicy->nbOfUserTokenPolicies);

    userPolicy = &secuPolicy->userTokenPolicies[0];
    ck_assert_int_ge(0, userPolicy->IssuedTokenType.Length);
    ck_assert_int_ge(0, userPolicy->IssuerEndpointUrl.Length);
    ck_assert_int_eq(0, strcmp("anon1", SOPC_String_GetRawCString(&userPolicy->PolicyId)));
    ck_assert_int_eq(OpcUa_UserTokenType_Anonymous, userPolicy->TokenType);
    ck_assert_int_ge(0, userPolicy->SecurityPolicyUri.Length);

    /* 3rd secu policy */
    secuPolicy = &epConfig->secuConfigurations[2];
    strEqual = strcmp("http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256",
                      SOPC_String_GetRawCString(&secuPolicy->securityPolicy));
    ck_assert_int_eq(0, strEqual);
    ck_assert_int_eq(secuPolicy->securityModes & SOPC_SECURITY_MODE_SIGN_MASK, SOPC_SECURITY_MODE_SIGN_MASK);
    ck_assert_int_eq(secuPolicy->securityModes & SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK,
                     SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK);
    ck_assert_int_eq(secuPolicy->securityModes | SOPC_SECURITY_MODE_SIGN_MASK | SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK,
                     SOPC_SECURITY_MODE_SIGN_MASK | SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK);
    ck_assert_uint_eq(2, secuPolicy->nbOfUserTokenPolicies);

    // 1st user policy
    userPolicy = &secuPolicy->userTokenPolicies[0];
    ck_assert_int_ge(0, userPolicy->IssuedTokenType.Length);
    ck_assert_int_ge(0, userPolicy->IssuerEndpointUrl.Length);
    ck_assert_int_eq(0, strcmp("anon2", SOPC_String_GetRawCString(&userPolicy->PolicyId)));
    ck_assert_int_eq(OpcUa_UserTokenType_Anonymous, userPolicy->TokenType);
    ck_assert_int_ge(0, userPolicy->SecurityPolicyUri.Length);
    // 2nd user policy
    userPolicy = &secuPolicy->userTokenPolicies[1];
    ck_assert_int_ge(0, userPolicy->IssuedTokenType.Length);
    ck_assert_int_ge(0, userPolicy->IssuerEndpointUrl.Length);
    ck_assert_int_eq(0, strcmp("user1", SOPC_String_GetRawCString(&userPolicy->PolicyId)));
    ck_assert_int_eq(OpcUa_UserTokenType_UserName, userPolicy->TokenType);
    ck_assert_int_eq(0, strcmp("http://opcfoundation.org/UA/SecurityPolicy#None",
                               SOPC_String_GetRawCString(&userPolicy->SecurityPolicyUri)));

    /* 2nd endpoint */
    epConfig = &sConfig->endpoints[1];
    ck_assert_ptr_eq(epConfig->serverConfigPtr, sConfig);
    ck_assert_int_eq(0, strcmp("opc.tcp://MY_SERVER_HOST:4841/MY_ENPOINT_NAME_2", epConfig->endpointURL));

    /* Check security policies */
    ck_assert_uint_eq(2, epConfig->nbSecuConfigs);

    /* 1st secu policy */
    secuPolicy = &epConfig->secuConfigurations[0];
    strEqual = strcmp("http://opcfoundation.org/UA/SecurityPolicy#None",
                      SOPC_String_GetRawCString(&secuPolicy->securityPolicy));
    ck_assert_int_eq(0, strEqual);
    ck_assert_int_eq(secuPolicy->securityModes & SOPC_SECURITY_MODE_NONE_MASK, SOPC_SECURITY_MODE_NONE_MASK);
    ck_assert_int_eq(secuPolicy->securityModes | SOPC_SECURITY_MODE_NONE_MASK, SOPC_SECURITY_MODE_NONE_MASK);
    ck_assert_uint_eq(1, secuPolicy->nbOfUserTokenPolicies);

    userPolicy = &secuPolicy->userTokenPolicies[0];
    ck_assert_int_ge(0, userPolicy->IssuedTokenType.Length);
    ck_assert_int_ge(0, userPolicy->IssuerEndpointUrl.Length);
    ck_assert_int_eq(0, strcmp("anon", SOPC_String_GetRawCString(&userPolicy->PolicyId)));
    ck_assert_int_eq(OpcUa_UserTokenType_Anonymous, userPolicy->TokenType);
    ck_assert_int_ge(0, userPolicy->SecurityPolicyUri.Length);

    /* 2nd secu policy */
    secuPolicy = &epConfig->secuConfigurations[1];
    strEqual = strcmp("http://opcfoundation.org/UA/SecurityPolicy#Basic256",
                      SOPC_String_GetRawCString(&secuPolicy->securityPolicy));
    ck_assert_int_eq(0, strEqual);
    // Support None security mode
    ck_assert_int_eq(secuPolicy->securityModes & SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK,
                     SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK);
    // And only None security mode
    ck_assert_int_eq(secuPolicy->securityModes | SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK,
                     SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK);
    ck_assert_uint_eq(1, secuPolicy->nbOfUserTokenPolicies);

    userPolicy = &secuPolicy->userTokenPolicies[0];
    ck_assert_int_ge(0, userPolicy->IssuedTokenType.Length);
    ck_assert_int_ge(0, userPolicy->IssuerEndpointUrl.Length);
    ck_assert_int_eq(0, strcmp("username", SOPC_String_GetRawCString(&userPolicy->PolicyId)));
    ck_assert_int_eq(OpcUa_UserTokenType_UserName, userPolicy->TokenType);
    strEqual = strcmp("http://opcfoundation.org/UA/SecurityPolicy#None",
                      SOPC_String_GetRawCString(&userPolicy->SecurityPolicyUri));
    ck_assert_int_eq(0, strEqual);
}
#endif

START_TEST(test_XML_server_configuration)
{
// Without EXPAT test cannot be done
#ifdef WITH_EXPAT
    FILE* fd = fopen(XML_SRV_CONFIG_NAME, "r");

    ck_assert_ptr_nonnull(fd);

    SOPC_S2OPC_Config s2opcConfig;
    SOPC_S2OPC_Config_Initialize(&s2opcConfig);

    bool result = SOPC_Config_Parse(fd, &s2opcConfig);
    fclose(fd);
    ck_assert(result);

    check_parsed_s2opc_config(&s2opcConfig);

    SOPC_S2OPC_Config_Clear(&s2opcConfig);

#else
    printf("Test test_XML_server_configuration ignored since EXPAT is not available\n");
#endif
}
END_TEST

Suite* tests_make_suite_XML_parsers(void)
{
    Suite* s;
    TCase* tc_XML_parsers;

    s = suite_create("XML parsers tests");
    tc_XML_parsers = tcase_create("XML parsers");
    tcase_add_test(tc_XML_parsers, test_same_address_space_results);
    tcase_add_test(tc_XML_parsers, test_XML_server_configuration);
    suite_add_tcase(s, tc_XML_parsers);

    return s;
}
