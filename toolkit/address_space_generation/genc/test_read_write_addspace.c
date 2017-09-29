
/*
 *  Copyright (C) 2017 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "add.h"

#include <stdio.h>
#include <stdbool.h>

#include "sopc_builtintypes.h"
#include "sopc_types.h"
#include "sopc_base_types.h"

#define NB_1 0    /* View */
#define NB_2 1    /* Object */
#define NB_3 6    /* Variable */
#define NB_4 0    /* VariableType */
#define NB_5 0    /* ObjectType */
#define NB_6 0    /* ReferenceType */
#define NB_7 0    /* DataType */
#define NB_8 0    /* Method */

#define NB (NB_1 + NB_2 + NB_3 + NB_4 + NB_5 + NB_6 + NB_7 + NB_8)

#define toSOPC_String(s) ((SOPC_Byte*)s)

#define DEFAULT_VARIANT  {SOPC_Null_Id, SOPC_VariantArrayType_SingleValue,{0}}



static SOPC_NodeId nodeid_1 = {IdentifierType_Numeric, 0, .Data.Numeric = 1000};
static SOPC_ExpandedNodeId ex_nodeid_1 = {{IdentifierType_Numeric, 0, .Data.Numeric = 1000}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_2 = {IdentifierType_Numeric, 0, .Data.Numeric = 35};
static SOPC_ExpandedNodeId ex_nodeid_2 = {{IdentifierType_Numeric, 0, .Data.Numeric = 35}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_3 = {IdentifierType_Numeric, 0, .Data.Numeric = 85};
static SOPC_ExpandedNodeId ex_nodeid_3 = {{IdentifierType_Numeric, 0, .Data.Numeric = 85}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_4 = {IdentifierType_Numeric, 0, .Data.Numeric = 40};
static SOPC_ExpandedNodeId ex_nodeid_4 = {{IdentifierType_Numeric, 0, .Data.Numeric = 40}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_5 = {IdentifierType_Numeric, 0, .Data.Numeric = 61};
static SOPC_ExpandedNodeId ex_nodeid_5 = {{IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_6 = {IdentifierType_Numeric, 0, .Data.Numeric = 47};
static SOPC_ExpandedNodeId ex_nodeid_6 = {{IdentifierType_Numeric, 0, .Data.Numeric = 47}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_7 = {IdentifierType_Numeric, 0, .Data.Numeric = 1001};
static SOPC_ExpandedNodeId ex_nodeid_7 = {{IdentifierType_Numeric, 0, .Data.Numeric = 1001}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_8 = {IdentifierType_Numeric, 0, .Data.Numeric = 1002};
static SOPC_ExpandedNodeId ex_nodeid_8 = {{IdentifierType_Numeric, 0, .Data.Numeric = 1002}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_9 = {IdentifierType_Numeric, 0, .Data.Numeric = 1003};
static SOPC_ExpandedNodeId ex_nodeid_9 = {{IdentifierType_Numeric, 0, .Data.Numeric = 1003}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_10 = {IdentifierType_Numeric, 0, .Data.Numeric = 1004};
static SOPC_ExpandedNodeId ex_nodeid_10 = {{IdentifierType_Numeric, 0, .Data.Numeric = 1004}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_11 = {IdentifierType_Numeric, 0, .Data.Numeric = 1005};
static SOPC_ExpandedNodeId ex_nodeid_11 = {{IdentifierType_Numeric, 0, .Data.Numeric = 1005}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_12 = {IdentifierType_Numeric, 0, .Data.Numeric = 1006};
static SOPC_ExpandedNodeId ex_nodeid_12 = {{IdentifierType_Numeric, 0, .Data.Numeric = 1006}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_13 = {IdentifierType_Numeric, 0, .Data.Numeric = 63};
static SOPC_ExpandedNodeId ex_nodeid_13 = {{IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0,0, NULL}, 0};


void* avoid_unused_nodes_var[] = {&nodeid_1,
&ex_nodeid_1,&nodeid_2,
&ex_nodeid_2,&nodeid_3,
&ex_nodeid_3,&nodeid_4,
&ex_nodeid_4,&nodeid_5,
&ex_nodeid_5,&nodeid_6,
&ex_nodeid_6,&nodeid_7,
&ex_nodeid_7,&nodeid_8,
&ex_nodeid_8,&nodeid_9,
&ex_nodeid_9,&nodeid_10,
&ex_nodeid_10,&nodeid_11,
&ex_nodeid_11,&nodeid_12,
&ex_nodeid_12,&nodeid_13,
&ex_nodeid_13,};


static SOPC_QualifiedName BrowseName[NB + 1] = {{0, {0, 0, NULL}}
,{0,{17,1,toSOPC_String("VariablesFolderBn")}}/* i=1000*/
,{0,{5,1,toSOPC_String("Int64")}}/* i=1001*/
,{0,{6,1,toSOPC_String("Uint32")}}/* i=1002*/
,{0,{6,1,toSOPC_String("Double")}}/* i=1003*/
,{0,{6,1,toSOPC_String("String")}}/* i=1004*/
,{0,{10,1,toSOPC_String("ByteString")}}/* i=1005*/
,{0,{10,1,toSOPC_String("XmlElement")}}/* i=1006*/

};


static SOPC_LocalizedText Description[] = {{{0, 0, NULL}, {0, 0, NULL}}
, {{0,1,toSOPC_String("")},{25,1,toSOPC_String("VariablesFolderDescObj1d2")}}
, {{0,1,toSOPC_String("")},{8,1,toSOPC_String("Int64_1d")}}
, {{0,1,toSOPC_String("")},{9,1,toSOPC_String("UInt32_1d")}}
, {{0,1,toSOPC_String("")},{9,1,toSOPC_String("Double_1d")}}
, {{0,1,toSOPC_String("")},{9,1,toSOPC_String("String_1d")}}
, {{0,1,toSOPC_String("")},{13,1,toSOPC_String("ByteString_1d")}}
, {{0,1,toSOPC_String("")},{13,1,toSOPC_String("XmlElement_1d")}}

};
static int Description_begin[] = {0, 1, 2, 3, 4, 5, 6, 7};
static int Description_end[] = {-1, 1, 2, 3, 4, 5, 6, 7};
static SOPC_LocalizedText DisplayName[] = {{{0, 0, NULL}, {0, 0, NULL}}
, {{0,1,toSOPC_String("")},{17,1,toSOPC_String("VariablesFolderDn")}}
, {{0,1,toSOPC_String("")},{9,1,toSOPC_String("Int64_1dn")}}
, {{0,1,toSOPC_String("")},{10,1,toSOPC_String("UInt32_1dn")}}
, {{0,1,toSOPC_String("")},{10,1,toSOPC_String("Double_1dn")}}
, {{0,1,toSOPC_String("")},{10,1,toSOPC_String("String_1dn")}}
, {{0,1,toSOPC_String("")},{14,1,toSOPC_String("ByteString_1dn")}}
, {{0,1,toSOPC_String("")},{14,1,toSOPC_String("XmlElement_1dn")}}

};
static int DisplayName_begin[] = {0, 1, 2, 3, 4, 5, 6, 7};
static int DisplayName_end[] = {-1, 1, 2, 3, 4, 5, 6, 7};


static int reference_begin[] = {0, 1, 9, 11, 13, 15, 17, 19};
static int reference_end[] = {-1,
0+8 /* i=1000 */,
8+2 /* i=1001 */,
10+2 /* i=1002 */,
12+2 /* i=1003 */,
14+2 /* i=1004 */,
16+2 /* i=1005 */,
18+2 /* i=1006 */};
static SOPC_NodeId* reference_type[] = {NULL,  &nodeid_2, &nodeid_4, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4};
static SOPC_ExpandedNodeId* reference_target[] = {NULL, &ex_nodeid_3, &ex_nodeid_5, &ex_nodeid_7, &ex_nodeid_8, &ex_nodeid_9, &ex_nodeid_10, &ex_nodeid_11, &ex_nodeid_12, &ex_nodeid_1, &ex_nodeid_13, &ex_nodeid_1, &ex_nodeid_13, &ex_nodeid_1, &ex_nodeid_13, &ex_nodeid_1, &ex_nodeid_13, &ex_nodeid_1, &ex_nodeid_13, &ex_nodeid_1, &ex_nodeid_13};
static bool reference_isForward[]={false, false, true, true, true, true, true, true, true, false, true, false, true, false, true, false, true, false, true, false, true};


static SOPC_NodeId* NodeId[NB+1] = {NULL,
&nodeid_1,
&nodeid_7,
&nodeid_8,
&nodeid_9,
&nodeid_10,
&nodeid_11,
&nodeid_12};



static OpcUa_NodeClass NodeClass[NB+1] = {OpcUa_NodeClass_Unspecified,
    OpcUa_NodeClass_Object, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable
};


SOPC_Variant Value[NB_3+NB_4+1] = {DEFAULT_VARIANT,{SOPC_Int64_Id, SOPC_VariantArrayType_SingleValue, {.Int64=-1000}},{SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue, {.Uint32=1000}},{SOPC_Double_Id, SOPC_VariantArrayType_SingleValue, {.Doublev=2.0}},{SOPC_String_Id, SOPC_VariantArrayType_SingleValue, {.String={14,1,toSOPC_String("String:INGOPCS")}}},{SOPC_ByteString_Id, SOPC_VariantArrayType_SingleValue, {.Bstring={18,1,toSOPC_String("ByteString:INGOPCS")}}},{SOPC_XmlElement_Id, SOPC_VariantArrayType_SingleValue, {.XmlElt={18,1,toSOPC_String("XmlElement:INGOPCS")}}}};


static SOPC_StatusCode status_code[] = {STATUS_NOK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK};


static SOPC_SByte AccessLevel[] = {0, 1, 1, 1, 1, 1, 1};

SOPC_AddressSpace addressSpace = {
    .nbVariables = NB_3,
    .nbVariableTypes = NB_4,
    .nbObjectTypes = NB_5,
    .nbReferenceTypes = NB_6,
    .nbDataTypes = NB_7,
    .nbMethods = NB_8,
    .nbObjects = NB_2,
    .nbViews = NB_1,
    .nbNodesTotal = NB,

    .browseNameArray = BrowseName,
    .descriptionIdxArray_begin = Description_begin,
    .descriptionIdxArray_end = Description_end,
    .descriptionArray = Description,
    .displayNameIdxArray_begin = DisplayName_begin,
    .displayNameIdxArray_end = DisplayName_end,
    .displayNameArray = DisplayName,
    .nodeClassArray = NodeClass,
    .nodeIdArray = NodeId,
    .referenceIdxArray_begin = reference_begin,
    .referenceIdxArray_end = reference_end,
    .referenceTypeArray = reference_type,
    .referenceTargetArray = reference_target,
    .referenceIsForwardArray = reference_isForward,
    .valueArray = Value,
    .valueStatusArray = status_code,
    .accessLevelArray = AccessLevel,
};
