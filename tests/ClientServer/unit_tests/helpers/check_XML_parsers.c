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
#include "xml_expat/sopc_users_loader.h"
#endif

#include "opcua_identifiers.h"
#include "sopc_encodeable.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_user_app_itf.h"

#define XML_UA_NODESET_NAME "S2OPC_Test_NodeSet.xml"
#define XML_SRV_CONFIG_NAME "S2OPC_Test_XML_Config.xml"
#define XML_USERS_CONFIG_NAME "S2OPC_Test_Users.xml"

// Avoid unused functions and variables when EXPAT is not available
#ifdef WITH_EXPAT
#ifdef WITH_CONST_ADDSPACE
#else
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
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    if (compare != 0)
    {
        char* nodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(leftSpace, left));
        printf("ERROR: AddressSpace variants from Python and Expat parser are differents for nodeId='%s':\n",
               nodeIdStr);
        SOPC_Free(nodeIdStr);
        SOPC_Variant_Print(SOPC_AddressSpace_Get_Value(leftSpace, left));
        SOPC_Variant_Print(SOPC_AddressSpace_Get_Value(rightSpace, right));
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

static void addspace_for_each_equal(const uintptr_t key, const uintptr_t value, uintptr_t user_data)
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
    SOPC_AddressSpace** addSpaces = (SOPC_AddressSpace**) user_data;

    SOPC_AddressSpace* leftSpace = addSpaces[0];
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_AddressSpace_Node* left = (SOPC_AddressSpace_Node*) value;
    SOPC_GCC_DIAGNOSTIC_RESTORE

    SOPC_AddressSpace* rightSpace = addSpaces[1];
    SOPC_AddressSpace_Node* right = SOPC_AddressSpace_Get_Node(rightSpace, (SOPC_NodeId*) key, &found);
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
#endif // WITH_CONST_ADDSPACE
#endif // WITH_EXPAT

START_TEST(test_same_address_space_results)
{
// Without EXPAT test cannot be done
#ifdef WITH_EXPAT
#ifdef WITH_CONST_ADDSPACE
    printf("Test test_same_address_space_results ignored since WITH_CONST_ADDSPACE is set\n");
#else
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
    SOPC_AddressSpace_ForEach(spaceEmbedded, addspace_for_each_equal, (uintptr_t) spaces);

    /* Check all data present in dynamic are present in embedded */
    SOPC_AddressSpace* spaces2[2] = {spaceDynamic, spaceEmbedded};
    SOPC_AddressSpace_ForEach(spaceDynamic, addspace_for_each_equal, (uintptr_t) spaces2);

    SOPC_AddressSpace_Delete(spaceEmbedded);
    SOPC_AddressSpace_Delete(spaceDynamic);
#endif // WITH_CONST_ADDSPACE
#else
    printf("Test test_same_address_space_results ignored since EXPAT is not available\n");
#endif // WITH_EXPAT
}
END_TEST

const char* expectedNamespaces[3] = {"urn:S2OPC:MY_SERVER_HOST", "urn:S2OPC:MY_SERVER_HOST:2", NULL};
const char* serverExpectedLocales[4] = {"en", "es-ES", "fr-FR", NULL};
const char* clientExpectedLocales[3] = {"en-US", "fr-FR", NULL};

const char* expectedTrustedRootIssuers[3] = {"/mypath/cacert.der", "/mypath/othercacert.der", NULL};
const char* expectedTrustedIntermediateIssuers[2] = {"/mypath/intermediate_cacert.der", NULL};

const char* expectedIssuedCerts[3] = {"/mypath/self_signed.der", "/mypath/signed_by_not_trusted_ca.der", NULL};
const char* expectedUntrustedRootIssuers[2] = {"/mypath/not_trusted_ca.der", NULL};
const char* expectedUntrustedIntermediateIssuers[2] = {"/mypath/not_trusted_intermediate_ca.der", NULL};
const char* expectedIssuersCRLs[6] = {"/mypath/cacrl.der",
                                      "/mypath/othercacrl.der",
                                      "/mypath/intermediate_cacrl.der",
                                      "/mypath/not_trusted_revoked.crl",
                                      "/mypath/not_trusted_intermediate_revoked.crl",
                                      NULL};

// Without EXPAT function is detected as unused and compilation fails
#ifdef WITH_EXPAT
static void check_parsed_s2opc_server_config(SOPC_S2OPC_Config* s2opcConfig)
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
    while (sConfig->localeIds[localeCounter] != NULL && serverExpectedLocales[localeCounter] != NULL)
    {
        int cmp_res = strcmp(sConfig->localeIds[localeCounter], serverExpectedLocales[localeCounter]);
        ck_assert_int_eq(0, cmp_res);
        localeCounter++;
    }
    ck_assert_ptr_null(sConfig->localeIds[localeCounter]);
    ck_assert_ptr_null(serverExpectedLocales[localeCounter]);

    /* Check application certificates */
    ck_assert_int_eq(0, strcmp("/mypath/mycert.der", sConfig->serverCertPath));
    ck_assert_int_eq(0, strcmp("/mypath/mykey.pem", sConfig->serverKeyPath));
    /* Check whether the server's key private is encryted or not. */
    ck_assert_int_eq(true, sConfig->serverKeyEncrypted);

    /* Check trusted CAs */
    int caCounter = 0;
    // Root CA
    for (caCounter = 0; sConfig->trustedRootIssuersList[caCounter] != NULL && expectedTrustedRootIssuers[caCounter];
         caCounter++)
    {
        int cmp_res = strcmp(sConfig->trustedRootIssuersList[caCounter], expectedTrustedRootIssuers[caCounter]);
        ck_assert_int_eq(0, cmp_res);
    }
    ck_assert_ptr_null(sConfig->trustedRootIssuersList[caCounter]);
    ck_assert_ptr_null(expectedTrustedRootIssuers[caCounter]);
    // Intermediate CA:
    for (caCounter = 0;
         sConfig->trustedIntermediateIssuersList[caCounter] != NULL && expectedTrustedIntermediateIssuers[caCounter];
         caCounter++)
    {
        int cmp_res =
            strcmp(sConfig->trustedIntermediateIssuersList[caCounter], expectedTrustedIntermediateIssuers[caCounter]);
        ck_assert_int_eq(0, cmp_res);
    }
    ck_assert_ptr_null(sConfig->trustedIntermediateIssuersList[caCounter]);
    ck_assert_ptr_null(expectedTrustedIntermediateIssuers[caCounter]);

    /* Check trusted issued certificates */
    int issuedCounter = 0;
    for (issuedCounter = 0;
         sConfig->issuedCertificatesList[issuedCounter] != NULL && expectedIssuedCerts[issuedCounter]; issuedCounter++)
    {
        int cmp_res = strcmp(sConfig->issuedCertificatesList[issuedCounter], expectedIssuedCerts[issuedCounter]);
        ck_assert_int_eq(0, cmp_res);
    }
    ck_assert_ptr_null(sConfig->issuedCertificatesList[issuedCounter]);
    ck_assert_ptr_null(expectedIssuedCerts[issuedCounter]);

    /* Check untrusted CAs (used to check issued certificate trust chain) */
    int untrustedCAcounter = 0;
    // Root CA:
    for (untrustedCAcounter = 0; sConfig->untrustedRootIssuersList[untrustedCAcounter] != NULL &&
                                 expectedUntrustedRootIssuers[untrustedCAcounter];
         untrustedCAcounter++)
    {
        int cmp_res = strcmp(sConfig->untrustedRootIssuersList[untrustedCAcounter],
                             expectedUntrustedRootIssuers[untrustedCAcounter]);
        ck_assert_int_eq(0, cmp_res);
    }
    ck_assert_ptr_null(sConfig->untrustedRootIssuersList[untrustedCAcounter]);
    ck_assert_ptr_null(expectedUntrustedRootIssuers[untrustedCAcounter]);
    // Intermediate CA:
    for (untrustedCAcounter = 0; sConfig->untrustedIntermediateIssuersList[untrustedCAcounter] != NULL &&
                                 expectedUntrustedIntermediateIssuers[untrustedCAcounter];
         untrustedCAcounter++)
    {
        int cmp_res = strcmp(sConfig->untrustedIntermediateIssuersList[untrustedCAcounter],
                             expectedUntrustedIntermediateIssuers[untrustedCAcounter]);
        ck_assert_int_eq(0, cmp_res);
    }
    ck_assert_ptr_null(sConfig->untrustedIntermediateIssuersList[untrustedCAcounter]);
    ck_assert_ptr_null(expectedUntrustedIntermediateIssuers[untrustedCAcounter]);

    /* Check CRLs */
    int crlCounter = 0;
    for (crlCounter = 0; sConfig->certificateRevocationPathList[crlCounter] != NULL && expectedIssuersCRLs[crlCounter];
         crlCounter++)
    {
        int cmp_res = strcmp(sConfig->certificateRevocationPathList[crlCounter], expectedIssuersCRLs[crlCounter]);
        ck_assert_int_eq(0, cmp_res);
    }
    ck_assert_ptr_null(sConfig->certificateRevocationPathList[crlCounter]);
    ck_assert_ptr_null(expectedIssuersCRLs[crlCounter]);

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
        SOPC_LocalizedText* lt = (SOPC_LocalizedText*) SOPC_SLinkedList_Next(&it);
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

    ck_assert_int_eq(OpcUa_ApplicationType_Server, sConfig->serverDescription.ApplicationType);

    /* Check endpoints */
    ck_assert_uint_eq(2, sConfig->nbEndpoints);

    /* 1st endpoint */
    SOPC_Endpoint_Config* epConfig = &sConfig->endpoints[0];
    ck_assert_ptr_eq(epConfig->serverConfigPtr, sConfig);
    ck_assert_int_eq(0, strcmp("opc.tcp://MY_SERVER_HOST:4841/MY_ENPOINT_NAME", epConfig->endpointURL));
    ck_assert(epConfig->noListening);

    /* Check reverse connections */
    ck_assert_uint_eq(2, epConfig->nbClientsToConnect);
    /* 1st reverse connection */
    ck_assert_ptr_null(epConfig->clientsToConnect[0].clientApplicationURI);
    int strEqual = strcmp("opc.tcp://localhost:4842", epConfig->clientsToConnect[0].clientEndpointURL);
    ck_assert_int_eq(0, strEqual);
    /* 2nd reverse connection */
    strEqual = strcmp("urn:S2OPC:client", epConfig->clientsToConnect[1].clientApplicationURI);
    ck_assert_int_eq(0, strEqual);
    strEqual = strcmp("opc.tcp://localhost:4843", epConfig->clientsToConnect[1].clientEndpointURL);
    ck_assert_int_eq(0, strEqual);

    /* Check security policies */
    ck_assert_uint_eq(5, epConfig->nbSecuConfigs);

    /* 1st secu policy */
    SOPC_SecurityPolicy* secuPolicy = &epConfig->secuConfigurations[0];
    strEqual = strcmp("http://opcfoundation.org/UA/SecurityPolicy#None",
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

    /* 4th secu policy */
    secuPolicy = &epConfig->secuConfigurations[3];
    strEqual = strcmp("http://opcfoundation.org/UA/SecurityPolicy#Aes128_Sha256_RsaOaep",
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

    /* 5th secu policy */
    secuPolicy = &epConfig->secuConfigurations[4];
    strEqual = strcmp("http://opcfoundation.org/UA/SecurityPolicy#Aes256_Sha256_RsaPss",
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

static void check_parsed_s2opc_client_config(SOPC_S2OPC_Config* s2opcConfig)
{
    SOPC_Client_Config* cConfig = &s2opcConfig->clientConfig;
    int localeCounter = 0;
    while (cConfig->clientLocaleIds[localeCounter] != NULL && clientExpectedLocales[localeCounter] != NULL)
    {
        int cmp_res = strcmp(cConfig->clientLocaleIds[localeCounter], clientExpectedLocales[localeCounter]);
        ck_assert_int_eq(0, cmp_res);
        localeCounter++;
    }
    ck_assert_ptr_null(cConfig->clientLocaleIds[localeCounter]);
    ck_assert_ptr_null(clientExpectedLocales[localeCounter]);

    /* Check application certificates */
    ck_assert_int_eq(true, cConfig->isConfigFromPathsNeeded);
    ck_assert_int_eq(0, strcmp("/mypath/mycert.der", cConfig->configFromPaths->clientCertPath));
    ck_assert_int_eq(0, strcmp("/mypath/mykey.pem", cConfig->configFromPaths->clientKeyPath));
    /* Check whether the server's key private is encryted or not. */
    ck_assert_int_eq(true, cConfig->configFromPaths->clientKeyEncrypted);

    /* Check trusted CAs */
    int caCounter = 0;
    // Root CA
    for (caCounter = 0;
         cConfig->configFromPaths->trustedRootIssuersList[caCounter] != NULL && expectedTrustedRootIssuers[caCounter];
         caCounter++)
    {
        int cmp_res =
            strcmp(cConfig->configFromPaths->trustedRootIssuersList[caCounter], expectedTrustedRootIssuers[caCounter]);
        ck_assert_int_eq(0, cmp_res);
    }
    ck_assert_ptr_null(cConfig->configFromPaths->trustedRootIssuersList[caCounter]);
    ck_assert_ptr_null(expectedTrustedRootIssuers[caCounter]);
    // Intermediate CA:
    for (caCounter = 0; cConfig->configFromPaths->trustedIntermediateIssuersList[caCounter] != NULL &&
                        expectedTrustedIntermediateIssuers[caCounter];
         caCounter++)
    {
        int cmp_res = strcmp(cConfig->configFromPaths->trustedIntermediateIssuersList[caCounter],
                             expectedTrustedIntermediateIssuers[caCounter]);
        ck_assert_int_eq(0, cmp_res);
    }
    ck_assert_ptr_null(cConfig->configFromPaths->trustedIntermediateIssuersList[caCounter]);
    ck_assert_ptr_null(expectedTrustedIntermediateIssuers[caCounter]);

    /* Check trusted issued certificates */
    int issuedCounter = 0;
    for (issuedCounter = 0;
         cConfig->configFromPaths->issuedCertificatesList[issuedCounter] != NULL && expectedIssuedCerts[issuedCounter];
         issuedCounter++)
    {
        int cmp_res =
            strcmp(cConfig->configFromPaths->issuedCertificatesList[issuedCounter], expectedIssuedCerts[issuedCounter]);
        ck_assert_int_eq(0, cmp_res);
    }
    ck_assert_ptr_null(cConfig->configFromPaths->issuedCertificatesList[issuedCounter]);
    ck_assert_ptr_null(expectedIssuedCerts[issuedCounter]);

    /* Check untrusted CAs (used to check issued certificate trust chain) */
    int untrustedCAcounter = 0;
    // Root CA:
    for (untrustedCAcounter = 0; cConfig->configFromPaths->untrustedRootIssuersList[untrustedCAcounter] != NULL &&
                                 expectedUntrustedRootIssuers[untrustedCAcounter];
         untrustedCAcounter++)
    {
        int cmp_res = strcmp(cConfig->configFromPaths->untrustedRootIssuersList[untrustedCAcounter],
                             expectedUntrustedRootIssuers[untrustedCAcounter]);
        ck_assert_int_eq(0, cmp_res);
    }
    ck_assert_ptr_null(cConfig->configFromPaths->untrustedRootIssuersList[untrustedCAcounter]);
    ck_assert_ptr_null(expectedUntrustedRootIssuers[untrustedCAcounter]);
    // Intermediate CA:
    for (untrustedCAcounter = 0;
         cConfig->configFromPaths->untrustedIntermediateIssuersList[untrustedCAcounter] != NULL &&
         expectedUntrustedIntermediateIssuers[untrustedCAcounter];
         untrustedCAcounter++)
    {
        int cmp_res = strcmp(cConfig->configFromPaths->untrustedIntermediateIssuersList[untrustedCAcounter],
                             expectedUntrustedIntermediateIssuers[untrustedCAcounter]);
        ck_assert_int_eq(0, cmp_res);
    }
    ck_assert_ptr_null(cConfig->configFromPaths->untrustedIntermediateIssuersList[untrustedCAcounter]);
    ck_assert_ptr_null(expectedUntrustedIntermediateIssuers[untrustedCAcounter]);

    /* Check CRLs */
    int crlCounter = 0;
    for (crlCounter = 0;
         cConfig->configFromPaths->certificateRevocationPathList[crlCounter] != NULL && expectedIssuersCRLs[crlCounter];
         crlCounter++)
    {
        int cmp_res = strcmp(cConfig->configFromPaths->certificateRevocationPathList[crlCounter],
                             expectedIssuersCRLs[crlCounter]);
        ck_assert_int_eq(0, cmp_res);
    }
    ck_assert_ptr_null(cConfig->configFromPaths->certificateRevocationPathList[crlCounter]);
    ck_assert_ptr_null(expectedIssuersCRLs[crlCounter]);

    /* Check application description */
    int res =
        strcmp("urn:S2OPC:MY_CLIENT_HOST:app", SOPC_String_GetRawCString(&cConfig->clientDescription.ApplicationUri));
    ck_assert_int_eq(0, res);
    res = strcmp("urn:S2OPC:MY_CLIENT_HOST:prod", SOPC_String_GetRawCString(&cConfig->clientDescription.ProductUri));
    ck_assert_int_eq(0, res);
    /* Application names */
    // default name
    res = strcmp("S2OPC toolkit client example",
                 SOPC_String_GetRawCString(&cConfig->clientDescription.ApplicationName.defaultText));
    ck_assert_int_eq(0, res);
    res = strcmp("en-US", SOPC_String_GetRawCString(&cConfig->clientDescription.ApplicationName.defaultLocale));
    ck_assert_int_eq(0, res);

    ck_assert_int_eq(OpcUa_ApplicationType_ClientAndServer, cConfig->clientDescription.ApplicationType);

    /* Check connections */
    ck_assert_uint_eq(3, cConfig->nbSecureConnections);

    /* 1st connection */
    SOPC_SecureConnection_Config* secConnConfig = cConfig->secureConnections[0];
    ck_assert_int_eq(0, secConnConfig->secureConnectionIdx);
    ck_assert_int_eq(0, strcmp("FirstConnection", secConnConfig->userDefinedId));
    ck_assert_int_eq(true, secConnConfig->scConfig.isClientSc);
    ck_assert_ptr_eq(secConnConfig->scConfig.clientConfigPtr, cConfig);
    ck_assert_int_eq(0, strcmp("opc.tcp://MY_SERVER_HOST:4840", secConnConfig->scConfig.url));
    ck_assert_ptr_null(secConnConfig->scConfig.serverUri);
    ck_assert_uint_eq(3600000, secConnConfig->scConfig.requestedLifetime);
    ck_assert_ptr_nonnull(secConnConfig->reverseURL);
    ck_assert_int_eq(0, strcmp("opc.tcp://MY_CLIENT_HOST:4844", secConnConfig->reverseURL));

    ck_assert_ptr_null(secConnConfig->serverCertPath);
    ck_assert_int_eq(OpcUa_MessageSecurityMode_None, secConnConfig->scConfig.msgSecurityMode);
    ck_assert_int_eq(
        0, strcmp("http://opcfoundation.org/UA/SecurityPolicy#None", secConnConfig->scConfig.reqSecuPolicyUri));
    ck_assert_int_eq(OpcUa_UserTokenType_Anonymous, secConnConfig->sessionConfig.userTokenType);
    ck_assert_ptr_nonnull(secConnConfig->sessionConfig.userPolicyId);
    ck_assert_int_eq(0, strcmp("anon", secConnConfig->sessionConfig.userPolicyId));

    /* 2nd connection */
    secConnConfig = cConfig->secureConnections[1];
    ck_assert_int_eq(1, secConnConfig->secureConnectionIdx);
    ck_assert_ptr_null(secConnConfig->userDefinedId);
    ck_assert_int_eq(true, secConnConfig->scConfig.isClientSc);
    ck_assert_ptr_eq(secConnConfig->scConfig.clientConfigPtr, cConfig);
    ck_assert_int_eq(0, strcmp("opc.tcp://MY_SERVER_HOST:4841/MY_ENDPOINT_NAME", secConnConfig->scConfig.url));
    ck_assert_ptr_nonnull(secConnConfig->scConfig.serverUri);
    ck_assert_int_eq(0, strcmp("urn:S2OPC:MY_SERVER_HOST:app", secConnConfig->scConfig.serverUri));
    ck_assert_uint_eq(7200000, secConnConfig->scConfig.requestedLifetime);
    ck_assert_ptr_null(secConnConfig->reverseURL);

    ck_assert_ptr_nonnull(secConnConfig->serverCertPath);
    ck_assert_int_eq(0, strcmp("server_cert.der", secConnConfig->serverCertPath));

    ck_assert_int_eq(OpcUa_MessageSecurityMode_SignAndEncrypt, secConnConfig->scConfig.msgSecurityMode);
    ck_assert_int_eq(0, strcmp("http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256",
                               secConnConfig->scConfig.reqSecuPolicyUri));
    ck_assert_int_eq(OpcUa_UserTokenType_UserName, secConnConfig->sessionConfig.userTokenType);
    ck_assert_ptr_nonnull(secConnConfig->sessionConfig.userPolicyId);
    ck_assert_int_eq(0, strcmp("user", secConnConfig->sessionConfig.userPolicyId));
    ck_assert_ptr_nonnull(secConnConfig->sessionConfig.userToken.userName.userName);
    ck_assert_int_eq(0, strcmp("user1", secConnConfig->sessionConfig.userToken.userName.userName));
    ck_assert_ptr_null(secConnConfig->sessionConfig.userToken.userName.userPwd);

    /* 3rd connection */
    secConnConfig = cConfig->secureConnections[2];
    ck_assert_int_eq(2, secConnConfig->secureConnectionIdx);
    ck_assert_int_eq(0, strcmp("ThirdConnection", secConnConfig->userDefinedId));
    ck_assert_int_eq(true, secConnConfig->scConfig.isClientSc);
    ck_assert_ptr_eq(secConnConfig->scConfig.clientConfigPtr, cConfig);
    ck_assert_int_eq(0, strcmp("opc.tcp://MY_SERVER_HOST:4841/MY_ENDPOINT_NAME", secConnConfig->scConfig.url));
    ck_assert_ptr_null(secConnConfig->scConfig.serverUri);
    ck_assert_uint_eq(3600000, secConnConfig->scConfig.requestedLifetime);
    ck_assert_ptr_nonnull(secConnConfig->reverseURL);
    ck_assert_int_eq(0, strcmp("opc.tcp://MY_CLIENT_HOST:4844", secConnConfig->reverseURL));

    ck_assert_ptr_nonnull(secConnConfig->serverCertPath);
    ck_assert_int_eq(0, strcmp("server_cert.der", secConnConfig->serverCertPath));

    ck_assert_int_eq(OpcUa_MessageSecurityMode_Sign, secConnConfig->scConfig.msgSecurityMode);
    ck_assert_int_eq(0, strcmp("http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256",
                               secConnConfig->scConfig.reqSecuPolicyUri));
    ck_assert_int_eq(OpcUa_UserTokenType_Certificate, secConnConfig->sessionConfig.userTokenType);
    ck_assert_ptr_nonnull(secConnConfig->sessionConfig.userPolicyId);
    ck_assert_int_eq(0, strcmp("x509", secConnConfig->sessionConfig.userPolicyId));
    ck_assert_int_eq(true, secConnConfig->sessionConfig.userToken.userX509.isConfigFromPathNeeded);
    ck_assert_ptr_nonnull(secConnConfig->sessionConfig.userToken.userX509.configFromPaths);

    ck_assert_ptr_nonnull(secConnConfig->sessionConfig.userToken.userX509.configFromPaths->userCertPath);
    ck_assert_int_eq(
        0, strcmp("user_cert.der", secConnConfig->sessionConfig.userToken.userX509.configFromPaths->userCertPath));
    ck_assert_ptr_nonnull(secConnConfig->sessionConfig.userToken.userX509.configFromPaths->userCertPath);
    ck_assert_int_eq(
        0, strcmp("user_key.pem", secConnConfig->sessionConfig.userToken.userX509.configFromPaths->userKeyPath));
    ck_assert_int_eq(true, secConnConfig->sessionConfig.userToken.userX509.configFromPaths->userKeyEncrypted);

    /* global reverse endpoints for all connections */
    ck_assert_uint_eq(1, cConfig->nbReverseEndpointURLs);
    ck_assert_int_eq(0, strcmp("opc.tcp://MY_CLIENT_HOST:4844", cConfig->reverseEndpointURLs[0]));
}
#endif

START_TEST(test_XML_config_configuration)
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

    check_parsed_s2opc_server_config(&s2opcConfig);
    check_parsed_s2opc_client_config(&s2opcConfig);

    SOPC_S2OPC_Config_Clear(&s2opcConfig);
#else
    printf("Test test_XML_server_configuration ignored since EXPAT is not available\n");
#endif
}
END_TEST

// Without EXPAT function is detected as unused and compilation fails
#ifdef WITH_EXPAT
static SOPC_ExtensionObject* build_user_name_token(char* username, char* password)
{
    SOPC_ExtensionObject* extObject = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));
    SOPC_ReturnStatus status;
    OpcUa_UserNameIdentityToken* token = NULL;

    ck_assert_ptr_nonnull(extObject);
    SOPC_ExtensionObject_Initialize(extObject);
    status = SOPC_Encodeable_CreateExtension(extObject, &OpcUa_UserNameIdentityToken_EncodeableType, (void**) &token);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_String_AttachFromCstring(&token->UserName, username);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_String_AttachFromCstring((SOPC_String*) &token->Password, password);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    return extObject;
}

static SOPC_ExtensionObject* build_x509_token(char* pathCert)
{
    SOPC_ExtensionObject* extObject = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));
    SOPC_ReturnStatus status;
    OpcUa_X509IdentityToken* token = NULL;

    ck_assert_ptr_nonnull(extObject);
    SOPC_ExtensionObject_Initialize(extObject);
    status = SOPC_Encodeable_CreateExtension(extObject, &OpcUa_X509IdentityToken_EncodeableType, (void**) &token);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_Buffer* buffer = NULL;
    status = SOPC_Buffer_ReadFile(pathCert, &buffer);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_ByteString_CopyFromBytes(&token->CertificateData, buffer->data, (int32_t) buffer->length);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_Buffer_Delete(buffer);

    return extObject;
}

static const SOPC_ExtensionObject anonymousIdentityToken = {
    .Encoding = SOPC_ExtObjBodyEncoding_Object,
    .TypeId.NodeId.IdentifierType = SOPC_IdentifierType_Numeric,
    .TypeId.NodeId.Data.Numeric = OpcUaId_AnonymousIdentityToken_Encoding_DefaultBinary,
    .Body.Object.ObjType = &OpcUa_AnonymousIdentityToken_EncodeableType,
    .Body.Object.Value = NULL};

static void check_parsed_users_config(SOPC_UserAuthentication_Manager* authenticationManager,
                                      SOPC_UserAuthorization_Manager* authorizationManager)
{
    SOPC_ExtensionObject* invalidUserNameToken = build_user_name_token("invalid user name", "12345");
    SOPC_ExtensionObject* invalidPasswordToken = build_user_name_token("user1", "01234");
    SOPC_ExtensionObject* noAccessToken = build_user_name_token("noaccess", "secret");
    SOPC_ExtensionObject* writeExecAddToken = build_user_name_token("user1", "12345");
    SOPC_ExtensionObject* readExecAddToken = build_user_name_token("user2", "password2");
    SOPC_ExtensionObject* readWriteAddToken = build_user_name_token("user3", "42");
    SOPC_ExtensionObject* readWriteExecToken = build_user_name_token("user4", "user4");
    SOPC_ExtensionObject* x509_untrustedIssued_defaultAccessToken = build_x509_token("user_public/user_2k_cert.der");
    SOPC_ExtensionObject* x509_defaultAccessToken = build_x509_token("issued_usr/ctt_usrT.der");
    SOPC_ExtensionObject* x509_readExecAddToken = build_x509_token("issued_usr/ctt_usrTE.der");
    SOPC_ExtensionObject* x509_writeExecAddToken = build_x509_token("issued_usr/ctt_usrTSincorrect.der");
    SOPC_ExtensionObject* x509_readWriteAddToken = build_x509_token("issued_usr/ctt_usrTV.der");
    SOPC_ExtensionObject* x509_readWriteExecToken = build_x509_token("issued_usr/ctt_ca1I_usrT.der");
    SOPC_UserAuthentication_Status authenticationRes;
    SOPC_ReturnStatus status =
        SOPC_UserAuthentication_IsValidUserIdentity(authenticationManager, invalidUserNameToken, &authenticationRes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(SOPC_USER_AUTHENTICATION_REJECTED_TOKEN, authenticationRes);
    status =
        SOPC_UserAuthentication_IsValidUserIdentity(authenticationManager, invalidPasswordToken, &authenticationRes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(SOPC_USER_AUTHENTICATION_REJECTED_TOKEN, authenticationRes);
    status = SOPC_UserAuthentication_IsValidUserIdentity(authenticationManager, noAccessToken, &authenticationRes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(SOPC_USER_AUTHENTICATION_ACCESS_DENIED, authenticationRes);
    status = SOPC_UserAuthentication_IsValidUserIdentity(authenticationManager, writeExecAddToken, &authenticationRes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(SOPC_USER_AUTHENTICATION_OK, authenticationRes);
    status = SOPC_UserAuthentication_IsValidUserIdentity(authenticationManager, readExecAddToken, &authenticationRes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(SOPC_USER_AUTHENTICATION_OK, authenticationRes);
    status = SOPC_UserAuthentication_IsValidUserIdentity(authenticationManager, readWriteAddToken, &authenticationRes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(SOPC_USER_AUTHENTICATION_OK, authenticationRes);
    status = SOPC_UserAuthentication_IsValidUserIdentity(authenticationManager, readWriteExecToken, &authenticationRes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(SOPC_USER_AUTHENTICATION_OK, authenticationRes);

    /* The validity of the x509Identity Token is check with the PKI during the UACTT test */

    SOPC_UserWithAuthorization* invalidNoAccesses =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(invalidUserNameToken, authorizationManager);
    ck_assert_ptr_nonnull(invalidNoAccesses);
    SOPC_UserWithAuthorization* anonNoAccesses =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(&anonymousIdentityToken, authorizationManager);
    ck_assert_ptr_nonnull(anonNoAccesses);
    SOPC_UserWithAuthorization* noAccess =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(noAccessToken, authorizationManager);
    ck_assert_ptr_nonnull(noAccess);
    SOPC_UserWithAuthorization* writeExecAdd =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(writeExecAddToken, authorizationManager);
    ck_assert_ptr_nonnull(writeExecAdd);
    SOPC_UserWithAuthorization* readExecAdd =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(readExecAddToken, authorizationManager);
    ck_assert_ptr_nonnull(readExecAdd);
    SOPC_UserWithAuthorization* readWriteAdd =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(readWriteAddToken, authorizationManager);
    ck_assert_ptr_nonnull(readWriteAdd);
    SOPC_UserWithAuthorization* readWriteExec =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(readWriteExecToken, authorizationManager);
    ck_assert_ptr_nonnull(readWriteExec);

    // x509 token
    SOPC_UserWithAuthorization* x509_untrustedIssued_defaultAccess = SOPC_UserWithAuthorization_CreateFromIdentityToken(
        x509_untrustedIssued_defaultAccessToken, authorizationManager);
    SOPC_UserWithAuthorization* x509_defaultAccess =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(x509_defaultAccessToken, authorizationManager);
    ck_assert_ptr_nonnull(x509_defaultAccess);
    SOPC_UserWithAuthorization* x509_readExecAdd =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(x509_readExecAddToken, authorizationManager);
    ck_assert_ptr_nonnull(x509_readExecAdd);
    SOPC_UserWithAuthorization* x509_writeExecAdd =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(x509_writeExecAddToken, authorizationManager);
    ck_assert_ptr_nonnull(x509_writeExecAdd);
    SOPC_UserWithAuthorization* x509_readWriteAdd =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(x509_readWriteAddToken, authorizationManager);
    ck_assert_ptr_nonnull(x509_readWriteAdd);
    SOPC_UserWithAuthorization* x509_readWriteExec =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(x509_readWriteExecToken, authorizationManager);
    ck_assert_ptr_nonnull(x509_readWriteExec);

    const SOPC_NodeId node = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 29};
    uint32_t attr = 13;
    bool authorized = false;
    // invalidNoAccesses
    status = SOPC_UserAuthorization_IsAuthorizedOperation(invalidNoAccesses, SOPC_USER_AUTHORIZATION_OPERATION_READ,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(invalidNoAccesses, SOPC_USER_AUTHORIZATION_OPERATION_WRITE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(
        invalidNoAccesses, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(invalidNoAccesses, SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);

    // anonNoAccesses
    status = SOPC_UserAuthorization_IsAuthorizedOperation(anonNoAccesses, SOPC_USER_AUTHORIZATION_OPERATION_READ, &node,
                                                          attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(anonNoAccesses, SOPC_USER_AUTHORIZATION_OPERATION_WRITE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(anonNoAccesses, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(anonNoAccesses, SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);

    // noAccess
    status = SOPC_UserAuthorization_IsAuthorizedOperation(noAccess, SOPC_USER_AUTHORIZATION_OPERATION_READ, &node, attr,
                                                          &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(noAccess, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &node,
                                                          attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(noAccess, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &node,
                                                          attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(noAccess, SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE, &node,
                                                          attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);

    // writeExecAdd
    status = SOPC_UserAuthorization_IsAuthorizedOperation(writeExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_READ, &node,
                                                          attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(writeExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &node,
                                                          attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(writeExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(writeExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);

    // readExecAdd
    status = SOPC_UserAuthorization_IsAuthorizedOperation(readExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_READ, &node,
                                                          attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(readExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &node,
                                                          attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(readExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(readExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE, &node,
                                                          attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);

    // readWriteAdd
    status = SOPC_UserAuthorization_IsAuthorizedOperation(readWriteAdd, SOPC_USER_AUTHORIZATION_OPERATION_READ, &node,
                                                          attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(readWriteAdd, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &node,
                                                          attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(readWriteAdd, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(readWriteAdd, SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);

    // readWriteExec
    status = SOPC_UserAuthorization_IsAuthorizedOperation(readWriteExec, SOPC_USER_AUTHORIZATION_OPERATION_READ, &node,
                                                          attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(readWriteExec, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &node,
                                                          attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(readWriteExec, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(readWriteExec, SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);

    // x509_untrustedIssued_defaultAccess --> dafault issuers rights is used (read only)
    status = SOPC_UserAuthorization_IsAuthorizedOperation(
        x509_untrustedIssued_defaultAccess, SOPC_USER_AUTHORIZATION_OPERATION_READ, &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(
        x509_untrustedIssued_defaultAccess, SOPC_USER_AUTHORIZATION_OPERATION_WRITE, &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(
        x509_untrustedIssued_defaultAccess, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(
        x509_untrustedIssued_defaultAccess, SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE, &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);

    // x509_defaultAccess --> Read, execute and addnode rights are set to the default value False because rights are
    // partialy defined
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_defaultAccess, SOPC_USER_AUTHORIZATION_OPERATION_READ,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_defaultAccess, SOPC_USER_AUTHORIZATION_OPERATION_WRITE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(
        x509_defaultAccess, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_defaultAccess, SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);

    // x509_writeExecAdd
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_writeExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_READ,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_writeExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_WRITE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(
        x509_writeExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_writeExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);

    // x509_readExecAdd
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_readExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_READ,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_readExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_WRITE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(
        x509_readExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_readExecAdd, SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);

    // x509_readWriteAdd
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_readWriteAdd, SOPC_USER_AUTHORIZATION_OPERATION_READ,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_readWriteAdd, SOPC_USER_AUTHORIZATION_OPERATION_WRITE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(
        x509_readWriteAdd, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_readWriteAdd, SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);

    // x509_readWriteExec
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_readWriteExec, SOPC_USER_AUTHORIZATION_OPERATION_READ,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_readWriteExec, SOPC_USER_AUTHORIZATION_OPERATION_WRITE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(
        x509_readWriteExec, SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE, &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(authorized);
    status = SOPC_UserAuthorization_IsAuthorizedOperation(x509_readWriteExec, SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE,
                                                          &node, attr, &authorized);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(!authorized);

    SOPC_UserWithAuthorization_Free(&invalidNoAccesses);
    SOPC_UserWithAuthorization_Free(&anonNoAccesses);
    SOPC_UserWithAuthorization_Free(&noAccess);
    SOPC_UserWithAuthorization_Free(&writeExecAdd);
    SOPC_UserWithAuthorization_Free(&readExecAdd);
    SOPC_UserWithAuthorization_Free(&readWriteAdd);
    SOPC_UserWithAuthorization_Free(&readWriteExec);
    SOPC_UserWithAuthorization_Free(&x509_untrustedIssued_defaultAccess);
    SOPC_UserWithAuthorization_Free(&x509_readWriteExec);
    SOPC_UserWithAuthorization_Free(&x509_readWriteAdd);
    SOPC_UserWithAuthorization_Free(&x509_writeExecAdd);
    SOPC_UserWithAuthorization_Free(&x509_readExecAdd);
    SOPC_UserWithAuthorization_Free(&x509_defaultAccess);

    SOPC_ExtensionObject_Clear(invalidUserNameToken);
    SOPC_ExtensionObject_Clear(invalidPasswordToken);
    SOPC_ExtensionObject_Clear(noAccessToken);
    SOPC_ExtensionObject_Clear(writeExecAddToken);
    SOPC_ExtensionObject_Clear(readExecAddToken);
    SOPC_ExtensionObject_Clear(readWriteAddToken);
    SOPC_ExtensionObject_Clear(readWriteExecToken);
    SOPC_ExtensionObject_Clear(x509_untrustedIssued_defaultAccessToken);
    SOPC_ExtensionObject_Clear(x509_readWriteExecToken);
    SOPC_ExtensionObject_Clear(x509_readWriteAddToken);
    SOPC_ExtensionObject_Clear(x509_writeExecAddToken);
    SOPC_ExtensionObject_Clear(x509_readExecAddToken);
    SOPC_ExtensionObject_Clear(x509_defaultAccessToken);

    SOPC_Free(invalidUserNameToken);
    SOPC_Free(invalidPasswordToken);
    SOPC_Free(noAccessToken);
    SOPC_Free(writeExecAddToken);
    SOPC_Free(readExecAddToken);
    SOPC_Free(readWriteAddToken);
    SOPC_Free(readWriteExecToken);
    SOPC_Free(x509_untrustedIssued_defaultAccessToken);
    SOPC_Free(x509_readWriteExecToken);
    SOPC_Free(x509_readWriteAddToken);
    SOPC_Free(x509_writeExecAddToken);
    SOPC_Free(x509_readExecAddToken);
    SOPC_Free(x509_defaultAccessToken);
}
#endif

START_TEST(test_XML_users_configuration)
{
// Without EXPAT test cannot be done
#ifdef WITH_EXPAT
    FILE* fd = fopen(XML_USERS_CONFIG_NAME, "r");

    ck_assert_ptr_nonnull(fd);

    SOPC_UserAuthentication_Manager* authenticationManager = NULL;
    SOPC_UserAuthorization_Manager* authorizationManager = NULL;

    bool result = SOPC_UsersConfig_Parse(fd, &authenticationManager, &authorizationManager);
    fclose(fd);
    ck_assert(result);

    check_parsed_users_config(authenticationManager, authorizationManager);

    SOPC_UserAuthentication_FreeManager(&authenticationManager);
    SOPC_UserAuthorization_FreeManager(&authorizationManager);

#else
    printf("Test test_XML_users_configuration ignored since EXPAT is not available\n");
#endif
}
END_TEST

static void setup(void)
{
    SOPC_Helper_EndiannessCfg_Initialize();
}

Suite* tests_make_suite_XML_parsers(void)
{
    Suite* s;
    TCase* tc_XML_parsers;

    s = suite_create("XML parsers tests");
    tc_XML_parsers = tcase_create("XML parsers");
    // Increase default timeout of 4s to 10s (useful when CK_FORK!=NO)
    tcase_set_timeout(tc_XML_parsers, 10);
    tcase_add_checked_fixture(tc_XML_parsers, setup, NULL);
    tcase_add_test(tc_XML_parsers, test_same_address_space_results);
    tcase_add_test(tc_XML_parsers, test_XML_config_configuration);
    tcase_add_test(tc_XML_parsers, test_XML_users_configuration);
    suite_add_tcase(s, tc_XML_parsers);

    return s;
}
