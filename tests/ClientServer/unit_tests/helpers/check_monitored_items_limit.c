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
 * \brief Tests of the limits on the number of MonitoredItems (server global and per subscription).
 */

#include <check.h>
#include <stdbool.h>

#include "check_helpers.h"

#include "constants.h"
#include "monitored_item_event_filter_treatment_bs.h"
#include "monitored_item_pointer_bs.h"
#include "monitored_item_pointer_impl.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_string.h"
#include "sopc_mem_alloc.h"
#include "sopc_numeric_range.h"

static const constants__t_subscription_i SUB_A = 1;
static const constants__t_subscription_i SUB_B = 2;

/* Creates a MonitoredItem on the given attribute of ns=1;i=1003, returns its status code */
static constants_statuscodes_bs__t_StatusCode_i create_mi_on_attribute(constants__t_subscription_i sub,
                                                                       constants__t_AttributeId_i aid,
                                                                       constants__t_monitoringFilterCtx_i filterCtx,
                                                                       constants__t_monitoredItemPointer_i* mi)
{
    SOPC_NodeId nid;
    SOPC_NodeId_Initialize(&nid);
    nid.Namespace = 1;
    nid.IdentifierType = SOPC_IdentifierType_Numeric;
    nid.Data.Numeric = 1003;

    constants_statuscodes_bs__t_StatusCode_i sc = constants_statuscodes_bs__c_StatusCode_indet;
    constants__t_monitoredItemId_i miId = constants__c_monitoredItemId_indet;
    *mi = constants__c_monitoredItemPointer_indet;
    monitored_item_pointer_bs__create_monitored_item_pointer(sub, &nid, aid, NULL, constants__e_ttr_both,
                                                             constants__e_monitoringMode_reporting, 1, filterCtx, true,
                                                             1, &sc, mi, &miId);
    return sc;
}

/* Creates a MonitoredItem on the Value of ns=1;i=1003, returns its status code */
static constants_statuscodes_bs__t_StatusCode_i create_mi(constants__t_subscription_i sub,
                                                          constants__t_monitoringFilterCtx_i filterCtx,
                                                          constants__t_monitoredItemPointer_i* mi)
{
    return create_mi_on_attribute(sub, constants__e_aid_Value, filterCtx, mi);
}

/* Creates an event filter context with one select clause (browse path and index range set) */
static SOPC_InternalMonitoredItemFilterCtx* create_event_filter_ctx(void)
{
    SOPC_InternalMonitoredItemFilterCtx* filterCtx = SOPC_Calloc(1, sizeof(*filterCtx));
    ck_assert_ptr_nonnull(filterCtx);
    filterCtx->isDataFilter = false;
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_EncodeableObject_Create(&OpcUa_EventFilter_EncodeableType,
                                                                  (void**) &filterCtx->Filter.Event.eventFilter));
    OpcUa_EventFilter* eventFilter = filterCtx->Filter.Event.eventFilter;
    eventFilter->SelectClauses = SOPC_Calloc(1, sizeof(*eventFilter->SelectClauses));
    ck_assert_ptr_nonnull(eventFilter->SelectClauses);
    OpcUa_SimpleAttributeOperand_Initialize(&eventFilter->SelectClauses[0]);
    eventFilter->NoOfSelectClauses = 1;

    filterCtx->Filter.Event.qnPathStrSelectClauses = SOPC_Calloc(1, sizeof(char*));
    ck_assert_ptr_nonnull(filterCtx->Filter.Event.qnPathStrSelectClauses);
    filterCtx->Filter.Event.qnPathStrSelectClauses[0] = SOPC_strdup("0:Message");
    ck_assert_ptr_nonnull(filterCtx->Filter.Event.qnPathStrSelectClauses[0]);
    filterCtx->Filter.Event.indexRangeSelectClauses = SOPC_Calloc(1, sizeof(SOPC_NumericRange*));
    ck_assert_ptr_nonnull(filterCtx->Filter.Event.indexRangeSelectClauses);
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_NumericRange_Parse("1", &filterCtx->Filter.Event.indexRangeSelectClauses[0]));
    SOPC_NodeId_Initialize(&filterCtx->Filter.Event.whereClauseTypeId);
    return filterCtx;
}

START_TEST(test_monitored_items_global_and_subscription_limits)
{
    constants__t_monitoredItemPointer_i mis[4];
    constants__t_monitoredItemPointer_i refused = constants__c_monitoredItemPointer_indet;

    monitored_item_pointer_bs__INITIALISATION();
    SOPC_MonitoredItemPointer_SetLimits(3, 2);

    /* Subscription A: 2 MonitoredItems accepted, the 3rd one refused by the per subscription limit */
    ck_assert_int_eq(constants_statuscodes_bs__e_sc_ok, create_mi(SUB_A, NULL, &mis[0]));
    ck_assert_int_eq(constants_statuscodes_bs__e_sc_ok, create_mi(SUB_A, NULL, &mis[1]));
    ck_assert_int_eq(constants_statuscodes_bs__e_sc_bad_too_many_monitored_items, create_mi(SUB_A, NULL, &refused));
    ck_assert_ptr_null(refused);
    ck_assert_uint_eq(2, SOPC_MonitoredItemPointer_GetNbMonitoredItemsInSubscription(SUB_A));

    /* Subscription B: 1 MonitoredItem accepted, the next one refused by the server global limit (3) */
    ck_assert_int_eq(constants_statuscodes_bs__e_sc_ok, create_mi(SUB_B, NULL, &mis[2]));
    ck_assert_int_eq(constants_statuscodes_bs__e_sc_bad_too_many_monitored_items, create_mi(SUB_B, NULL, &refused));
    ck_assert_uint_eq(3, SOPC_MonitoredItemPointer_GetNbMonitoredItems());
    ck_assert_uint_eq(1, SOPC_MonitoredItemPointer_GetNbMonitoredItemsInSubscription(SUB_B));

    /* Deletion releases room in the server and in the subscription */
    monitored_item_pointer_bs__delete_monitored_item_pointer(mis[0]);
    ck_assert_uint_eq(2, SOPC_MonitoredItemPointer_GetNbMonitoredItems());
    ck_assert_uint_eq(1, SOPC_MonitoredItemPointer_GetNbMonitoredItemsInSubscription(SUB_A));
    ck_assert_int_eq(constants_statuscodes_bs__e_sc_ok, create_mi(SUB_B, NULL, &mis[3]));
    ck_assert_uint_eq(2, SOPC_MonitoredItemPointer_GetNbMonitoredItemsInSubscription(SUB_B));

    /* No limit (0): accepted again beyond previous limits */
    SOPC_MonitoredItemPointer_SetLimits(0, 0);
    ck_assert_int_eq(constants_statuscodes_bs__e_sc_ok, create_mi(SUB_B, NULL, &mis[0]));
    ck_assert_uint_eq(4, SOPC_MonitoredItemPointer_GetNbMonitoredItems());

    monitored_item_pointer_bs__monitored_item_pointer_bs_UNINITIALISATION();
    ck_assert_uint_eq(0, SOPC_MonitoredItemPointer_GetNbMonitoredItems());
    ck_assert_uint_eq(0, SOPC_MonitoredItemPointer_GetNbMonitoredItemsInSubscription(SUB_B));
}
END_TEST

/* A refused creation shall leave the filter context to the caller, which deletes it (subscription_core):
 * the context shall be freed exactly once (checked by AddressSanitizer) and according to its type
 * (data change filter or event filter) */
START_TEST(test_monitored_item_refused_filter_ownership)
{
    constants__t_monitoredItemPointer_i mi = constants__c_monitoredItemPointer_indet;

    monitored_item_pointer_bs__INITIALISATION();
    SOPC_MonitoredItemPointer_SetLimits(1, 0);
    ck_assert_int_eq(constants_statuscodes_bs__e_sc_ok, create_mi(SUB_A, NULL, &mi));

    SOPC_InternalMonitoredItemFilterCtx* filterCtx = SOPC_Calloc(1, sizeof(*filterCtx));
    ck_assert_ptr_nonnull(filterCtx);
    filterCtx->isDataFilter = true;
    OpcUa_DataChangeFilter_Initialize(&filterCtx->Filter.Data.dataFilter);
    filterCtx->Filter.Data.dataFilter.DeadbandType = OpcUa_DeadbandType_Absolute;
    filterCtx->Filter.Data.dataFilter.DeadbandValue = 1.0;
    filterCtx->Filter.Data.lastCachedValueForFilter = SOPC_Variant_Create();
    ck_assert_ptr_nonnull(filterCtx->Filter.Data.lastCachedValueForFilter);

    constants__t_monitoredItemPointer_i refused = constants__c_monitoredItemPointer_indet;
    ck_assert_int_eq(constants_statuscodes_bs__e_sc_bad_too_many_monitored_items,
                     create_mi(SUB_A, filterCtx, &refused));
    ck_assert_ptr_null(refused);
    /* Caller side deletion, as done by subscription_core on creation failure */
    monitored_item_event_filter_treatment_bs__delete_event_filter_context(filterCtx);

    /* Same with an event filter context on an event MonitoredItem */
    filterCtx = create_event_filter_ctx();
    refused = constants__c_monitoredItemPointer_indet;
    ck_assert_int_eq(constants_statuscodes_bs__e_sc_bad_too_many_monitored_items,
                     create_mi_on_attribute(SUB_A, constants__e_aid_EventNotifier, filterCtx, &refused));
    ck_assert_ptr_null(refused);
    monitored_item_event_filter_treatment_bs__delete_event_filter_context(filterCtx);

    monitored_item_pointer_bs__monitored_item_pointer_bs_UNINITIALISATION();
}
END_TEST

Suite* tests_make_suite_monitored_items_limit(void)
{
    Suite* s = suite_create("MonitoredItems limits");
    TCase* tc = tcase_create("MonitoredItems limits");
    tcase_add_test(tc, test_monitored_items_global_and_subscription_limits);
    tcase_add_test(tc, test_monitored_item_refused_filter_ownership);
    suite_add_tcase(s, tc);
    return s;
}
