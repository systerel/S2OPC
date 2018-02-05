/*
 *  Copyright (C) 2018 Systerel and others.
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

#include "sopc_addspace.h"

#include <stdbool.h>
#include <stdio.h>

#include "opcua_statuscodes.h"

#include "sopc_builtintypes.h"
#include "sopc_toolkit_constants.h"
#include "sopc_types.h"

#define NB_1 0  /* View */
#define NB_2 50 /* Object */
#define NB_3 48 /* Variable */
#define NB_4 2  /* VariableType */
#define NB_5 2  /* ObjectType */
#define NB_6 1  /* ReferenceType */
#define NB_7 31 /* DataType */
#define NB_8 0  /* Method */

#define NB (NB_1 + NB_2 + NB_3 + NB_4 + NB_5 + NB_6 + NB_7 + NB_8)

#define toSOPC_String(s) ((SOPC_Byte*) s)

#define DEFAULT_VARIANT                                        \
    {                                                          \
        SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, { 0 } \
    }

static SOPC_NodeId nodeid_1 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 84};
static SOPC_ExpandedNodeId ex_nodeid_1 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 84}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_2 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35};
static SOPC_ExpandedNodeId ex_nodeid_2 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_3 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 85};
static SOPC_ExpandedNodeId ex_nodeid_3 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 85}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_4 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 86};
static SOPC_ExpandedNodeId ex_nodeid_4 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 86}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_5 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40};
static SOPC_ExpandedNodeId ex_nodeid_5 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_6 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61};
static SOPC_ExpandedNodeId ex_nodeid_6 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_7 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1000};
static SOPC_ExpandedNodeId ex_nodeid_7 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1000}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_8 = {SOPC_IdentifierType_Numeric, 261, .Data.Numeric = 15361};
static SOPC_ExpandedNodeId ex_nodeid_8 = {{SOPC_IdentifierType_Numeric, 261, .Data.Numeric = 15361}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_9 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 88};
static SOPC_ExpandedNodeId ex_nodeid_9 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 88}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_10 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 89};
static SOPC_ExpandedNodeId ex_nodeid_10 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 89}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_11 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 90};
static SOPC_ExpandedNodeId ex_nodeid_11 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 90}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_12 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 91};
static SOPC_ExpandedNodeId ex_nodeid_12 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 91}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_13 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 58};
static SOPC_ExpandedNodeId ex_nodeid_13 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 58}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_14 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 62};
static SOPC_ExpandedNodeId ex_nodeid_14 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 62}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_15 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 24};
static SOPC_ExpandedNodeId ex_nodeid_15 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 24}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_16 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 26};
static SOPC_ExpandedNodeId ex_nodeid_16 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 26}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_17 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 27};
static SOPC_ExpandedNodeId ex_nodeid_17 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 27}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_18 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 28};
static SOPC_ExpandedNodeId ex_nodeid_18 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 28}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_19 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 29};
static SOPC_ExpandedNodeId ex_nodeid_19 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 29}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_20 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1};
static SOPC_ExpandedNodeId ex_nodeid_20 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_21 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2};
static SOPC_ExpandedNodeId ex_nodeid_21 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_22 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3};
static SOPC_ExpandedNodeId ex_nodeid_22 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_23 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 4};
static SOPC_ExpandedNodeId ex_nodeid_23 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 4}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_24 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 5};
static SOPC_ExpandedNodeId ex_nodeid_24 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 5}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_25 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 6};
static SOPC_ExpandedNodeId ex_nodeid_25 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 6}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_26 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7};
static SOPC_ExpandedNodeId ex_nodeid_26 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_27 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 8};
static SOPC_ExpandedNodeId ex_nodeid_27 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 8}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_28 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 9};
static SOPC_ExpandedNodeId ex_nodeid_28 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 9}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_29 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10};
static SOPC_ExpandedNodeId ex_nodeid_29 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_30 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11};
static SOPC_ExpandedNodeId ex_nodeid_30 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_31 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12};
static SOPC_ExpandedNodeId ex_nodeid_31 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_32 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 13};
static SOPC_ExpandedNodeId ex_nodeid_32 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 13}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_33 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 14};
static SOPC_ExpandedNodeId ex_nodeid_33 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 14}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_34 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 15};
static SOPC_ExpandedNodeId ex_nodeid_34 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 15}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_35 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 16};
static SOPC_ExpandedNodeId ex_nodeid_35 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 16}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_36 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 17};
static SOPC_ExpandedNodeId ex_nodeid_36 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 17}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_37 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 18};
static SOPC_ExpandedNodeId ex_nodeid_37 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 18}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_38 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 19};
static SOPC_ExpandedNodeId ex_nodeid_38 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 19}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_39 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 20};
static SOPC_ExpandedNodeId ex_nodeid_39 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 20}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_40 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 21};
static SOPC_ExpandedNodeId ex_nodeid_40 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 21}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_41 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 22};
static SOPC_ExpandedNodeId ex_nodeid_41 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 22}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_42 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 23};
static SOPC_ExpandedNodeId ex_nodeid_42 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 23}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_43 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 31};
static SOPC_ExpandedNodeId ex_nodeid_43 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 31}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_44 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10001};
static SOPC_ExpandedNodeId ex_nodeid_44 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10001}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_45 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10002};
static SOPC_ExpandedNodeId ex_nodeid_45 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10002}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_46 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10003};
static SOPC_ExpandedNodeId ex_nodeid_46 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10003}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_47 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10004};
static SOPC_ExpandedNodeId ex_nodeid_47 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10004}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_48 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10005};
static SOPC_ExpandedNodeId ex_nodeid_48 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10005}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_49 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10006};
static SOPC_ExpandedNodeId ex_nodeid_49 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10006}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_50 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10007};
static SOPC_ExpandedNodeId ex_nodeid_50 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10007}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_51 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10008};
static SOPC_ExpandedNodeId ex_nodeid_51 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10008}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_52 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10009};
static SOPC_ExpandedNodeId ex_nodeid_52 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10009}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_53 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10010};
static SOPC_ExpandedNodeId ex_nodeid_53 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10010}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_54 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10011};
static SOPC_ExpandedNodeId ex_nodeid_54 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10011}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_55 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10012};
static SOPC_ExpandedNodeId ex_nodeid_55 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10012}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_56 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10013};
static SOPC_ExpandedNodeId ex_nodeid_56 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10013}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_57 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10014};
static SOPC_ExpandedNodeId ex_nodeid_57 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10014}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_58 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10015};
static SOPC_ExpandedNodeId ex_nodeid_58 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10015}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_59 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10016};
static SOPC_ExpandedNodeId ex_nodeid_59 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10016}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_60 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10017};
static SOPC_ExpandedNodeId ex_nodeid_60 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10017}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_61 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10018};
static SOPC_ExpandedNodeId ex_nodeid_61 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10018}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_62 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10019};
static SOPC_ExpandedNodeId ex_nodeid_62 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10019}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_63 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10020};
static SOPC_ExpandedNodeId ex_nodeid_63 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 10020}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_64 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47};
static SOPC_ExpandedNodeId ex_nodeid_64 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_65 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1001};
static SOPC_ExpandedNodeId ex_nodeid_65 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1001}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_66 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1002};
static SOPC_ExpandedNodeId ex_nodeid_66 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1002}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_67 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1003};
static SOPC_ExpandedNodeId ex_nodeid_67 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1003}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_68 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1004};
static SOPC_ExpandedNodeId ex_nodeid_68 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1004}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_69 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1005};
static SOPC_ExpandedNodeId ex_nodeid_69 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1005}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_70 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1006};
static SOPC_ExpandedNodeId ex_nodeid_70 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1006}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_71 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1007};
static SOPC_ExpandedNodeId ex_nodeid_71 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1007}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_72 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1008};
static SOPC_ExpandedNodeId ex_nodeid_72 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1008}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_73 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1009};
static SOPC_ExpandedNodeId ex_nodeid_73 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1009}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_74 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1010};
static SOPC_ExpandedNodeId ex_nodeid_74 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1010}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_75 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1011};
static SOPC_ExpandedNodeId ex_nodeid_75 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1011}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_76 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1012};
static SOPC_ExpandedNodeId ex_nodeid_76 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1012}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_77 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1013};
static SOPC_ExpandedNodeId ex_nodeid_77 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1013}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_78 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {21, 1, toSOPC_String("Objects.15361.SIGNALs")}};
static SOPC_ExpandedNodeId ex_nodeid_78 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {21, 1, toSOPC_String("Objects.15361.SIGNALs")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_79 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {21, 1, toSOPC_String("Objects.15361.SWITCHs")}};
static SOPC_ExpandedNodeId ex_nodeid_79 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {21, 1, toSOPC_String("Objects.15361.SWITCHs")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_80 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {20, 1, toSOPC_String("Objects.15361.TRACKs")}};
static SOPC_ExpandedNodeId ex_nodeid_80 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {20, 1, toSOPC_String("Objects.15361.TRACKs")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_81 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {36, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019")}};
static SOPC_ExpandedNodeId ex_nodeid_81 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {36, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_82 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {36, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025")}};
static SOPC_ExpandedNodeId ex_nodeid_82 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {36, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_83 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {36, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G026")}};
static SOPC_ExpandedNodeId ex_nodeid_83 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {36, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G026")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_84 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {34, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1")}};
static SOPC_ExpandedNodeId ex_nodeid_84 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {34, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_85 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {36, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_026TK")}};
static SOPC_ExpandedNodeId ex_nodeid_85 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {36, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_026TK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_86 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {35, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_OSTK")}};
static SOPC_ExpandedNodeId ex_nodeid_86 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {35, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_OSTK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_87 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {35, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_P500")}};
static SOPC_ExpandedNodeId ex_nodeid_87 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {35, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_P500")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_88 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {43, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK")}};
static SOPC_ExpandedNodeId ex_nodeid_88 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {43, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_89 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {39, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_89 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {39, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_90 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {39, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC")}};
static SOPC_ExpandedNodeId ex_nodeid_90 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {39, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_91 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {48, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand")}};
static SOPC_ExpandedNodeId ex_nodeid_91 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {48, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_92 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {50, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K")}};
static SOPC_ExpandedNodeId ex_nodeid_92 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {50, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_93 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {51, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC")}};
static SOPC_ExpandedNodeId ex_nodeid_93 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {51, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_94 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK")}};
static SOPC_ExpandedNodeId ex_nodeid_94 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_95 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {43, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK")}};
static SOPC_ExpandedNodeId ex_nodeid_95 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {43, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_96 = {SOPC_IdentifierType_String, 261,
                                .Data.String = {44, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK")}};
static SOPC_ExpandedNodeId ex_nodeid_96 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {44, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_97 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {46, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ")}};
static SOPC_ExpandedNodeId ex_nodeid_97 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {46, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_98 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {46, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK")}};
static SOPC_ExpandedNodeId ex_nodeid_98 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {46, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_99 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {49, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK")}};
static SOPC_ExpandedNodeId ex_nodeid_99 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {49, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_100 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ")}};
static SOPC_ExpandedNodeId ex_nodeid_100 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_101 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ")}};
static SOPC_ExpandedNodeId ex_nodeid_101 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_102 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {46, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC")}};
static SOPC_ExpandedNodeId ex_nodeid_102 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {46, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_103 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {49, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC")}};
static SOPC_ExpandedNodeId ex_nodeid_103 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {49, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_104 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {47, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC")}};
static SOPC_ExpandedNodeId ex_nodeid_104 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {47, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_105 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {39, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_105 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {39, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RM")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_106 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {39, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RC")}};
static SOPC_ExpandedNodeId ex_nodeid_106 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {39, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RC")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_107 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {48, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand")}};
static SOPC_ExpandedNodeId ex_nodeid_107 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {48, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_108 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {50, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K")}};
static SOPC_ExpandedNodeId ex_nodeid_108 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {50, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_109 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {51, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC")}};
static SOPC_ExpandedNodeId ex_nodeid_109 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {51, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_110 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK")}};
static SOPC_ExpandedNodeId ex_nodeid_110 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_111 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {43, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK")}};
static SOPC_ExpandedNodeId ex_nodeid_111 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {43, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_112 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ")}};
static SOPC_ExpandedNodeId ex_nodeid_112 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_113 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ")}};
static SOPC_ExpandedNodeId ex_nodeid_113 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_114 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {39, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G026.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_114 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {39, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G026.RM")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_115 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK")}};
static SOPC_ExpandedNodeId ex_nodeid_115 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {42, 1, toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_116 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {37, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_116 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {37, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_117 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {37, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RC")}};
static SOPC_ExpandedNodeId ex_nodeid_117 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {37, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RC")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_118 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {46, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand")}};
static SOPC_ExpandedNodeId ex_nodeid_118 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {46, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_119 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {48, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K")}};
static SOPC_ExpandedNodeId ex_nodeid_119 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {48, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_120 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {49, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC")}};
static SOPC_ExpandedNodeId ex_nodeid_120 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {49, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_121 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {41, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK")}};
static SOPC_ExpandedNodeId ex_nodeid_121 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {41, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_122 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {41, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK")}};
static SOPC_ExpandedNodeId ex_nodeid_122 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {41, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_123 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {40, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK")}};
static SOPC_ExpandedNodeId ex_nodeid_123 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {40, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_124 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {41, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ")}};
static SOPC_ExpandedNodeId ex_nodeid_124 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {41, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_125 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {41, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ")}};
static SOPC_ExpandedNodeId ex_nodeid_125 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {41, 1, toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_126 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {39, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_026TK.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_126 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {39, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_026TK.RM")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_127 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {42, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK")}};
static SOPC_ExpandedNodeId ex_nodeid_127 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {42, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_128 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {38, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_OSTK.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_128 = {
    {SOPC_IdentifierType_String, 261, .Data.String = {38, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_OSTK.RM")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_129 = {SOPC_IdentifierType_String, 261,
                                 .Data.String = {41, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK")}};
static SOPC_ExpandedNodeId ex_nodeid_129 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {41, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_130 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {46, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_130 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {46, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_131 = {
    SOPC_IdentifierType_String, 261,
    .Data.String = {49, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK")}};
static SOPC_ExpandedNodeId ex_nodeid_131 = {
    {SOPC_IdentifierType_String, 261,
     .Data.String = {49, 1, toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK")}},
    {0, 0, NULL},
    0};
static SOPC_NodeId nodeid_132 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2259};
static SOPC_ExpandedNodeId ex_nodeid_132 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2259}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_133 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63};
static SOPC_ExpandedNodeId ex_nodeid_133 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_134 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3114};
static SOPC_ExpandedNodeId ex_nodeid_134 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3114}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_135 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45};
static SOPC_ExpandedNodeId ex_nodeid_135 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_136 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 25};
static SOPC_ExpandedNodeId ex_nodeid_136 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 25}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_137 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 30};
static SOPC_ExpandedNodeId ex_nodeid_137 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 30}, {0, 0, NULL}, 0};
static SOPC_NodeId nodeid_138 = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 121};
static SOPC_ExpandedNodeId ex_nodeid_138 = {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 121}, {0, 0, NULL}, 0};

void* avoid_unused_nodes_var[] = {
    &nodeid_1,   &ex_nodeid_1,   &nodeid_2,   &ex_nodeid_2,   &nodeid_3,   &ex_nodeid_3,   &nodeid_4,   &ex_nodeid_4,
    &nodeid_5,   &ex_nodeid_5,   &nodeid_6,   &ex_nodeid_6,   &nodeid_7,   &ex_nodeid_7,   &nodeid_8,   &ex_nodeid_8,
    &nodeid_9,   &ex_nodeid_9,   &nodeid_10,  &ex_nodeid_10,  &nodeid_11,  &ex_nodeid_11,  &nodeid_12,  &ex_nodeid_12,
    &nodeid_13,  &ex_nodeid_13,  &nodeid_14,  &ex_nodeid_14,  &nodeid_15,  &ex_nodeid_15,  &nodeid_16,  &ex_nodeid_16,
    &nodeid_17,  &ex_nodeid_17,  &nodeid_18,  &ex_nodeid_18,  &nodeid_19,  &ex_nodeid_19,  &nodeid_20,  &ex_nodeid_20,
    &nodeid_21,  &ex_nodeid_21,  &nodeid_22,  &ex_nodeid_22,  &nodeid_23,  &ex_nodeid_23,  &nodeid_24,  &ex_nodeid_24,
    &nodeid_25,  &ex_nodeid_25,  &nodeid_26,  &ex_nodeid_26,  &nodeid_27,  &ex_nodeid_27,  &nodeid_28,  &ex_nodeid_28,
    &nodeid_29,  &ex_nodeid_29,  &nodeid_30,  &ex_nodeid_30,  &nodeid_31,  &ex_nodeid_31,  &nodeid_32,  &ex_nodeid_32,
    &nodeid_33,  &ex_nodeid_33,  &nodeid_34,  &ex_nodeid_34,  &nodeid_35,  &ex_nodeid_35,  &nodeid_36,  &ex_nodeid_36,
    &nodeid_37,  &ex_nodeid_37,  &nodeid_38,  &ex_nodeid_38,  &nodeid_39,  &ex_nodeid_39,  &nodeid_40,  &ex_nodeid_40,
    &nodeid_41,  &ex_nodeid_41,  &nodeid_42,  &ex_nodeid_42,  &nodeid_43,  &ex_nodeid_43,  &nodeid_44,  &ex_nodeid_44,
    &nodeid_45,  &ex_nodeid_45,  &nodeid_46,  &ex_nodeid_46,  &nodeid_47,  &ex_nodeid_47,  &nodeid_48,  &ex_nodeid_48,
    &nodeid_49,  &ex_nodeid_49,  &nodeid_50,  &ex_nodeid_50,  &nodeid_51,  &ex_nodeid_51,  &nodeid_52,  &ex_nodeid_52,
    &nodeid_53,  &ex_nodeid_53,  &nodeid_54,  &ex_nodeid_54,  &nodeid_55,  &ex_nodeid_55,  &nodeid_56,  &ex_nodeid_56,
    &nodeid_57,  &ex_nodeid_57,  &nodeid_58,  &ex_nodeid_58,  &nodeid_59,  &ex_nodeid_59,  &nodeid_60,  &ex_nodeid_60,
    &nodeid_61,  &ex_nodeid_61,  &nodeid_62,  &ex_nodeid_62,  &nodeid_63,  &ex_nodeid_63,  &nodeid_64,  &ex_nodeid_64,
    &nodeid_65,  &ex_nodeid_65,  &nodeid_66,  &ex_nodeid_66,  &nodeid_67,  &ex_nodeid_67,  &nodeid_68,  &ex_nodeid_68,
    &nodeid_69,  &ex_nodeid_69,  &nodeid_70,  &ex_nodeid_70,  &nodeid_71,  &ex_nodeid_71,  &nodeid_72,  &ex_nodeid_72,
    &nodeid_73,  &ex_nodeid_73,  &nodeid_74,  &ex_nodeid_74,  &nodeid_75,  &ex_nodeid_75,  &nodeid_76,  &ex_nodeid_76,
    &nodeid_77,  &ex_nodeid_77,  &nodeid_78,  &ex_nodeid_78,  &nodeid_79,  &ex_nodeid_79,  &nodeid_80,  &ex_nodeid_80,
    &nodeid_81,  &ex_nodeid_81,  &nodeid_82,  &ex_nodeid_82,  &nodeid_83,  &ex_nodeid_83,  &nodeid_84,  &ex_nodeid_84,
    &nodeid_85,  &ex_nodeid_85,  &nodeid_86,  &ex_nodeid_86,  &nodeid_87,  &ex_nodeid_87,  &nodeid_88,  &ex_nodeid_88,
    &nodeid_89,  &ex_nodeid_89,  &nodeid_90,  &ex_nodeid_90,  &nodeid_91,  &ex_nodeid_91,  &nodeid_92,  &ex_nodeid_92,
    &nodeid_93,  &ex_nodeid_93,  &nodeid_94,  &ex_nodeid_94,  &nodeid_95,  &ex_nodeid_95,  &nodeid_96,  &ex_nodeid_96,
    &nodeid_97,  &ex_nodeid_97,  &nodeid_98,  &ex_nodeid_98,  &nodeid_99,  &ex_nodeid_99,  &nodeid_100, &ex_nodeid_100,
    &nodeid_101, &ex_nodeid_101, &nodeid_102, &ex_nodeid_102, &nodeid_103, &ex_nodeid_103, &nodeid_104, &ex_nodeid_104,
    &nodeid_105, &ex_nodeid_105, &nodeid_106, &ex_nodeid_106, &nodeid_107, &ex_nodeid_107, &nodeid_108, &ex_nodeid_108,
    &nodeid_109, &ex_nodeid_109, &nodeid_110, &ex_nodeid_110, &nodeid_111, &ex_nodeid_111, &nodeid_112, &ex_nodeid_112,
    &nodeid_113, &ex_nodeid_113, &nodeid_114, &ex_nodeid_114, &nodeid_115, &ex_nodeid_115, &nodeid_116, &ex_nodeid_116,
    &nodeid_117, &ex_nodeid_117, &nodeid_118, &ex_nodeid_118, &nodeid_119, &ex_nodeid_119, &nodeid_120, &ex_nodeid_120,
    &nodeid_121, &ex_nodeid_121, &nodeid_122, &ex_nodeid_122, &nodeid_123, &ex_nodeid_123, &nodeid_124, &ex_nodeid_124,
    &nodeid_125, &ex_nodeid_125, &nodeid_126, &ex_nodeid_126, &nodeid_127, &ex_nodeid_127, &nodeid_128, &ex_nodeid_128,
    &nodeid_129, &ex_nodeid_129, &nodeid_130, &ex_nodeid_130, &nodeid_131, &ex_nodeid_131, &nodeid_132, &ex_nodeid_132,
    &nodeid_133, &ex_nodeid_133, &nodeid_134, &ex_nodeid_134, &nodeid_135, &ex_nodeid_135, &nodeid_136, &ex_nodeid_136,
    &nodeid_137, &ex_nodeid_137, &nodeid_138, &ex_nodeid_138,
};

static SOPC_QualifiedName BrowseName[NB + 1] = {
    {0, {0, 0, NULL}},
    {0, {4, 1, toSOPC_String("Root")}} /* i=84*/
    ,
    {0, {7, 1, toSOPC_String("Objects")}} /* i=85*/
    ,
    {0, {5, 1, toSOPC_String("Types")}} /* i=86*/
    ,
    {0, {11, 1, toSOPC_String("ObjectTypes")}} /* i=88*/
    ,
    {0, {13, 1, toSOPC_String("VariableTypes")}} /* i=89*/
    ,
    {0, {9, 1, toSOPC_String("DataTypes")}} /* i=90*/
    ,
    {0, {14, 1, toSOPC_String("ReferenceTypes")}} /* i=91*/
    ,
    {0, {19, 1, toSOPC_String("SessionPlaceHolder1")}} /* i=10001*/
    ,
    {0, {19, 1, toSOPC_String("SessionPlaceHolder2")}} /* i=10002*/
    ,
    {0, {19, 1, toSOPC_String("SessionPlaceHolder3")}} /* i=10003*/
    ,
    {0, {19, 1, toSOPC_String("SessionPlaceHolder4")}} /* i=10004*/
    ,
    {0, {19, 1, toSOPC_String("SessionPlaceHolder5")}} /* i=10005*/
    ,
    {0, {19, 1, toSOPC_String("SessionPlaceHolder6")}} /* i=10006*/
    ,
    {0, {19, 1, toSOPC_String("SessionPlaceHolder7")}} /* i=10007*/
    ,
    {0, {19, 1, toSOPC_String("SessionPlaceHolder8")}} /* i=10008*/
    ,
    {0, {19, 1, toSOPC_String("SessionPlaceHolder9")}} /* i=10009*/
    ,
    {0, {20, 1, toSOPC_String("SessionPlaceHolder10")}} /* i=10010*/
    ,
    {0, {20, 1, toSOPC_String("SessionPlaceHolder11")}} /* i=10011*/
    ,
    {0, {20, 1, toSOPC_String("SessionPlaceHolder12")}} /* i=10012*/
    ,
    {0, {20, 1, toSOPC_String("SessionPlaceHolder13")}} /* i=10013*/
    ,
    {0, {20, 1, toSOPC_String("SessionPlaceHolder14")}} /* i=10014*/
    ,
    {0, {20, 1, toSOPC_String("SessionPlaceHolder15")}} /* i=10015*/
    ,
    {0, {20, 1, toSOPC_String("SessionPlaceHolder16")}} /* i=10016*/
    ,
    {0, {20, 1, toSOPC_String("SessionPlaceHolder17")}} /* i=10017*/
    ,
    {0, {20, 1, toSOPC_String("SessionPlaceHolder18")}} /* i=10018*/
    ,
    {0, {20, 1, toSOPC_String("SessionPlaceHolder19")}} /* i=10019*/
    ,
    {0, {20, 1, toSOPC_String("SessionPlaceHolder20")}} /* i=10020*/
    ,
    {0, {17, 1, toSOPC_String("VariablesFolderBn")}} /* i=1000*/
    ,
    {261, {5, 1, toSOPC_String("15361")}} /* ns=261;i=15361*/
    ,
    {261, {7, 1, toSOPC_String("SIGNALs")}} /* ns=261;s=Objects.15361.SIGNALs*/
    ,
    {261, {7, 1, toSOPC_String("SWITCHs")}} /* ns=261;s=Objects.15361.SWITCHs*/
    ,
    {261, {6, 1, toSOPC_String("TRACKs")}} /* ns=261;s=Objects.15361.TRACKs*/
    ,
    {261, {14, 1, toSOPC_String("BALA_RDLS_G019")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019*/
    ,
    {261, {2, 1, toSOPC_String("RM")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM*/
    ,
    {261, {2, 1, toSOPC_String("RC")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC*/
    ,
    {261, {14, 1, toSOPC_String("BALA_RDLS_G025")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025*/
    ,
    {261, {2, 1, toSOPC_String("RM")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM*/
    ,
    {261, {2, 1, toSOPC_String("RC")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC*/
    ,
    {261, {14, 1, toSOPC_String("BALA_RDLS_G026")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026*/
    ,
    {261, {2, 1, toSOPC_String("RM")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM*/
    ,
    {261, {12, 1, toSOPC_String("BALA_RDLS_W1")}} /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1*/
    ,
    {261, {2, 1, toSOPC_String("RM")}} /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM*/
    ,
    {261, {2, 1, toSOPC_String("RC")}} /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC*/
    ,
    {261, {15, 1, toSOPC_String("BALA_RDLS_026TK")}} /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK*/
    ,
    {261, {2, 1, toSOPC_String("RM")}} /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM*/
    ,
    {261, {14, 1, toSOPC_String("BALA_RDLS_OSTK")}} /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK*/
    ,
    {261, {2, 1, toSOPC_String("RM")}} /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM*/
    ,
    {261, {14, 1, toSOPC_String("BALA_RDLS_P500")}} /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_P500*/
    ,
    {261, {22, 1, toSOPC_String("BALA_RDLS_WBK_RDLN_EBK")}} /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK*/
    ,
    {261, {2, 1, toSOPC_String("RM")}} /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM*/
    ,
    {0, {5, 1, toSOPC_String("State")}} /* i=2259*/
    ,
    {0, {11, 1, toSOPC_String("EnabledFlag")}} /* i=3114*/
    ,
    {0, {5, 1, toSOPC_String("Int64")}} /* i=1001*/
    ,
    {0, {6, 1, toSOPC_String("UInt32")}} /* i=1002*/
    ,
    {0, {6, 1, toSOPC_String("Double")}} /* i=1003*/
    ,
    {0, {6, 1, toSOPC_String("String")}} /* i=1004*/
    ,
    {0, {10, 1, toSOPC_String("ByteString")}} /* i=1005*/
    ,
    {0, {10, 1, toSOPC_String("XmlElement")}} /* i=1006*/
    ,
    {0, {5, 1, toSOPC_String("SByte")}} /* i=1007*/
    ,
    {0, {4, 1, toSOPC_String("Byte")}} /* i=1008*/
    ,
    {0, {5, 1, toSOPC_String("Int16")}} /* i=1009*/
    ,
    {0, {6, 1, toSOPC_String("UInt16")}} /* i=1010*/
    ,
    {0, {5, 1, toSOPC_String("Int32")}} /* i=1011*/
    ,
    {0, {6, 1, toSOPC_String("UInt64")}} /* i=1012*/
    ,
    {0, {5, 1, toSOPC_String("Float")}} /* i=1013*/
    ,
    {261, {2, 1, toSOPC_String("GK")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK*/
    ,
    {261, {3, 1, toSOPC_String("ASK")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK*/
    ,
    {261, {4, 1, toSOPC_String("XBKK")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK*/
    ,
    {261, {6, 1, toSOPC_String("XBZCRQ")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ*/
    ,
    {261, {6, 1, toSOPC_String("XBZ-AK")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK*/
    ,
    {261, {9, 1, toSOPC_String("XBZCRQ-AK")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK*/
    ,
    {261, {11, 1, toSOPC_String("SendCommand")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand*/
    ,
    {261, {13, 1, toSOPC_String("OffBlocking-K")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K*/
    ,
    {261, {14, 1, toSOPC_String("OffBlocking-CC")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC*/
    ,
    {261, {2, 1, toSOPC_String("GZ")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ*/
    ,
    {261, {2, 1, toSOPC_String("SZ")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ*/
    ,
    {261, {6, 1, toSOPC_String("XBZ-CC")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC*/
    ,
    {261, {9, 1, toSOPC_String("XBZCRQ-CC")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC*/
    ,
    {261, {7, 1, toSOPC_String("XBZC-CC")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC*/
    ,
    {261, {2, 1, toSOPC_String("GK")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK*/
    ,
    {261, {3, 1, toSOPC_String("ASK")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK*/
    ,
    {261, {11, 1, toSOPC_String("SendCommand")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand*/
    ,
    {261, {13, 1, toSOPC_String("OffBlocking-K")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K*/
    ,
    {261, {14, 1, toSOPC_String("OffBlocking-CC")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC*/
    ,
    {261, {2, 1, toSOPC_String("SZ")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ*/
    ,
    {261, {2, 1, toSOPC_String("GZ")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ*/
    ,
    {261, {2, 1, toSOPC_String("GK")}} /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK*/
    ,
    {261, {3, 1, toSOPC_String("NWK")}} /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK*/
    ,
    {261, {3, 1, toSOPC_String("RWK")}} /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK*/
    ,
    {261, {2, 1, toSOPC_String("LK")}} /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK*/
    ,
    {261, {11, 1, toSOPC_String("SendCommand")}} /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand*/
    ,
    {261, {13, 1, toSOPC_String("OffBlocking-K")}} /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K*/
    ,
    {261, {14, 1, toSOPC_String("OffBlocking-CC")}} /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC*/
    ,
    {261, {3, 1, toSOPC_String("NWZ")}} /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ*/
    ,
    {261, {3, 1, toSOPC_String("RWZ")}} /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ*/
    ,
    {261, {2, 1, toSOPC_String("TK")}} /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK*/
    ,
    {261, {2, 1, toSOPC_String("TK")}} /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK*/
    ,
    {261, {2, 1, toSOPC_String("TK")}} /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK*/
    ,
    {0, {16, 1, toSOPC_String("BaseVariableType")}} /* i=62*/
    ,
    {0, {20, 1, toSOPC_String("BaseDataVariableType")}} /* i=63*/
    ,
    {0, {14, 1, toSOPC_String("BaseObjectType")}} /* i=58*/
    ,
    {0, {10, 1, toSOPC_String("FolderType")}} /* i=61*/
    ,
    {0, {10, 1, toSOPC_String("References")}} /* i=31*/
    ,
    {0, {12, 1, toSOPC_String("BaseDataType")}} /* i=24*/
    ,
    {0, {6, 1, toSOPC_String("Number")}} /* i=26*/
    ,
    {0, {7, 1, toSOPC_String("Integer")}} /* i=27*/
    ,
    {0, {8, 1, toSOPC_String("UInteger")}} /* i=28*/
    ,
    {0, {11, 1, toSOPC_String("Enumeration")}} /* i=29*/
    ,
    {0, {7, 1, toSOPC_String("Boolean")}} /* i=1*/
    ,
    {0, {5, 1, toSOPC_String("SByte")}} /* i=2*/
    ,
    {0, {4, 1, toSOPC_String("Byte")}} /* i=3*/
    ,
    {0, {5, 1, toSOPC_String("Int16")}} /* i=4*/
    ,
    {0, {6, 1, toSOPC_String("UInt16")}} /* i=5*/
    ,
    {0, {5, 1, toSOPC_String("Int32")}} /* i=6*/
    ,
    {0, {6, 1, toSOPC_String("UInt32")}} /* i=7*/
    ,
    {0, {5, 1, toSOPC_String("Int64")}} /* i=8*/
    ,
    {0, {6, 1, toSOPC_String("UInt64")}} /* i=9*/
    ,
    {0, {5, 1, toSOPC_String("Float")}} /* i=10*/
    ,
    {0, {6, 1, toSOPC_String("Double")}} /* i=11*/
    ,
    {0, {6, 1, toSOPC_String("String")}} /* i=12*/
    ,
    {0, {8, 1, toSOPC_String("DateTime")}} /* i=13*/
    ,
    {0, {4, 1, toSOPC_String("Guid")}} /* i=14*/
    ,
    {0, {10, 1, toSOPC_String("ByteString")}} /* i=15*/
    ,
    {0, {10, 1, toSOPC_String("XmlElement")}} /* i=16*/
    ,
    {0, {6, 1, toSOPC_String("NodeId")}} /* i=17*/
    ,
    {0, {14, 1, toSOPC_String("ExpandedNodeId")}} /* i=18*/
    ,
    {0, {10, 1, toSOPC_String("StatusCode")}} /* i=19*/
    ,
    {0, {13, 1, toSOPC_String("QualifiedName")}} /* i=20*/
    ,
    {0, {13, 1, toSOPC_String("LocalizedText")}} /* i=21*/
    ,
    {0, {9, 1, toSOPC_String("Structure")}} /* i=22*/
    ,
    {0, {9, 1, toSOPC_String("DataValue")}} /* i=23*/
    ,
    {0, {14, 1, toSOPC_String("DiagnosticInfo")}} /* i=25*/
    ,
    {0, {5, 1, toSOPC_String("Image")}} /* i=30*/
    ,
    {0, {10, 1, toSOPC_String("Decimal128")}} /* i=121*/

};

static SOPC_LocalizedText Description[] = {
    {{0, 0, NULL}, {0, 0, NULL}},
    {{0, 1, toSOPC_String("")}, {37, 1, toSOPC_String("The root of the server address space.")}},
    {{0, 1, toSOPC_String("")},
     {76, 1, toSOPC_String("The browse entry point when looking for objects in the server address space.")}},
    {{0, 1, toSOPC_String("")},
     {74, 1, toSOPC_String("The browse entry point when looking for types in the server address space.")}},
    {{0, 1, toSOPC_String("")},
     {81, 1, toSOPC_String("The browse entry point when looking for object types in the server address space.")}},
    {{0, 1, toSOPC_String("")},
     {83, 1, toSOPC_String("The browse entry point when looking for variable types in the server address space.")}},
    {{0, 1, toSOPC_String("")},
     {79, 1, toSOPC_String("The browse entry point when looking for data types in the server address space.")}},
    {{0, 1, toSOPC_String("")},
     {84, 1, toSOPC_String("The browse entry point when looking for reference types in the server address space.")}},
    {{0, 1, toSOPC_String("")}, {25, 1, toSOPC_String("VariablesFolderDescObj1d2")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{2, 1, toSOPC_String("en")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NoName")}},
    {{0, 1, toSOPC_String("")}, {46, 1, toSOPC_String("If TRUE the diagnostics collection is enabled.")}},
    {{0, 1, toSOPC_String("")}, {8, 1, toSOPC_String("Int64_1d")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("UInt32_1d")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("Double_1d")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("String_1d")}},
    {{0, 1, toSOPC_String("")}, {13, 1, toSOPC_String("ByteString_1d")}},
    {{0, 1, toSOPC_String("")}, {13, 1, toSOPC_String("XmlElement_1d")}},
    {{0, 1, toSOPC_String("")}, {8, 1, toSOPC_String("SByte_1d")}},
    {{0, 1, toSOPC_String("")}, {7, 1, toSOPC_String("Byte_1d")}},
    {{0, 1, toSOPC_String("")}, {8, 1, toSOPC_String("Int16_1d")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("UInt16_1d")}},
    {{0, 1, toSOPC_String("")}, {8, 1, toSOPC_String("Int32_1d")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("UInt64_1d")}},
    {{0, 1, toSOPC_String("")}, {8, 1, toSOPC_String("Float_1d")}},
    {{0, 1, toSOPC_String("")}, {32, 1, toSOPC_String("Permissive Signal Status, ~é€bla")}},
    {{0, 1, toSOPC_String("")}, {23, 1, toSOPC_String("Signal Approach Locking")}},
    {{0, 1, toSOPC_String("")}, {41, 1, toSOPC_String("Exit blocking is in effect for the signal")}},
    {{0, 1, toSOPC_String("")}, {26, 1, toSOPC_String("Exit Block Removal Request")}},
    {{0, 1, toSOPC_String("")}, {40, 1, toSOPC_String("Exit blocking is rejected for the signal")}},
    {{0, 1, toSOPC_String("")}, {45, 1, toSOPC_String("Exit block removal is rejected for the signal")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("Signal request")}},
    {{0, 1, toSOPC_String("")}, {13, 1, toSOPC_String("Signal cancel")}},
    {{0, 1, toSOPC_String("")}, {30, 1, toSOPC_String("Exit Block Application Request")}},
    {{0, 1, toSOPC_String("")}, {26, 1, toSOPC_String("Exit Block Removal Request")}},
    {{0, 1, toSOPC_String("")}, {32, 1, toSOPC_String("Exit Block Removal - Acknowledge")}},
    {{0, 1, toSOPC_String("")}, {24, 1, toSOPC_String("Permissive Signal Status")}},
    {{0, 1, toSOPC_String("")}, {23, 1, toSOPC_String("Signal Approach Locking")}},
    {{0, 1, toSOPC_String("")}, {13, 1, toSOPC_String("Signal cancel")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("Signal request")}},
    {{0, 1, toSOPC_String("")}, {24, 1, toSOPC_String("Permissive Signal Status")}},
    {{0, 1, toSOPC_String("")}, {22, 1, toSOPC_String("Switch Detected Normal")}},
    {{0, 1, toSOPC_String("")}, {23, 1, toSOPC_String("Switch Detected Reverse")}},
    {{0, 1, toSOPC_String("")}, {21, 1, toSOPC_String("Switch Locally Locked")}},
    {{0, 1, toSOPC_String("")}, {34, 1, toSOPC_String("Switch Calling in Normal Direction")}},
    {{0, 1, toSOPC_String("")}, {35, 1, toSOPC_String("Switch Calling in Reverse Direction")}},
    {{0, 1, toSOPC_String("")}, {26, 1, toSOPC_String("Secondary Detection Status")}},
    {{0, 1, toSOPC_String("")}, {26, 1, toSOPC_String("Secondary Detection Status")}},
    {{0, 1, toSOPC_String("")}, {26, 1, toSOPC_String("Secondary Detection Status")}},
    {{0, 1, toSOPC_String("")}, {46, 1, toSOPC_String("The abstract base type for all variable nodes.")}},
    {{0, 1, toSOPC_String("")}, {54, 1, toSOPC_String("The type for variable that represents a process value.")}},
    {{0, 1, toSOPC_String("")}, {35, 1, toSOPC_String("The base type for all object nodes.")}},
    {{0, 1, toSOPC_String("")}, {47, 1, toSOPC_String("The type for objects that organize other nodes.")}},
    {{0, 1, toSOPC_String("")}, {42, 1, toSOPC_String("The abstract base type for all references.")}},
    {{0, 1, toSOPC_String("")}, {51, 1, toSOPC_String("Describes a value that can have any valid DataType.")}},
    {{0, 1, toSOPC_String("")}, {53, 1, toSOPC_String("Describes a value that can have any numeric DataType.")}},
    {{0, 1, toSOPC_String("")}, {53, 1, toSOPC_String("Describes a value that can have any integer DataType.")}},
    {{0, 1, toSOPC_String("")},
     {62, 1, toSOPC_String("Describes a value that can have any unsigned integer DataType.")}},
    {{0, 1, toSOPC_String("")}, {49, 1, toSOPC_String("Describes a value that is an enumerated DataType.")}},
    {{0, 1, toSOPC_String("")}, {47, 1, toSOPC_String("Describes a value that is either TRUE or FALSE.")}},
    {{0, 1, toSOPC_String("")}, {58, 1, toSOPC_String("Describes a value that is an integer between -128 and 127.")}},
    {{0, 1, toSOPC_String("")}, {55, 1, toSOPC_String("Describes a value that is an integer between 0 and 255.")}},
    {{0, 1, toSOPC_String("")},
     {64, 1, toSOPC_String("Describes a value that is an integer between −32,768 and 32,767.")}},
    {{0, 1, toSOPC_String("")}, {57, 1, toSOPC_String("Describes a value that is an integer between 0 and 65535.")}},
    {{0, 1, toSOPC_String("")},
     {79, 1, toSOPC_String("Describes a value that is an integer between −2,147,483,648  and 2,147,483,647.")}},
    {{0, 1, toSOPC_String("")},
     {65, 1, toSOPC_String("Describes a value that is an integer between 0 and 4,294,967,295.")}},
    {{0, 1, toSOPC_String("")},
     {102, 1,
      toSOPC_String(
          "Describes a value that is an integer between −9,223,372,036,854,775,808 and 9,223,372,036,854,775,807.")}},
    {{0, 1, toSOPC_String("")},
     {78, 1, toSOPC_String("Describes a value that is an integer between 0 and 18,446,744,073,709,551,615.")}},
    {{0, 1, toSOPC_String("")},
     {82, 1, toSOPC_String("Describes a value that is an IEEE 754-1985 single precision floating point number.")}},
    {{0, 1, toSOPC_String("")},
     {82, 1, toSOPC_String("Describes a value that is an IEEE 754-1985 double precision floating point number.")}},
    {{0, 1, toSOPC_String("")},
     {69, 1, toSOPC_String("Describes a value that is a sequence of printable Unicode characters.")}},
    {{0, 1, toSOPC_String("")},
     {61, 1, toSOPC_String("Describes a value that is a Gregorian calender date and time.")}},
    {{0, 1, toSOPC_String("")},
     {63, 1, toSOPC_String("Describes a value that is a 128-bit globally unique identifier.")}},
    {{0, 1, toSOPC_String("")}, {46, 1, toSOPC_String("Describes a value that is a sequence of bytes.")}},
    {{0, 1, toSOPC_String("")}, {41, 1, toSOPC_String("Describes a value that is an XML element.")}},
    {{0, 1, toSOPC_String("")},
     {81, 1, toSOPC_String("Describes a value that is an identifier for a node within a Server address space.")}},
    {{0, 1, toSOPC_String("")}, {60, 1, toSOPC_String("Describes a value that is an absolute identifier for a node.")}},
    {{0, 1, toSOPC_String("")},
     {86, 1, toSOPC_String("Describes a value that is a code representing the outcome of an operation by a Server.")}},
    {{0, 1, toSOPC_String("")}, {58, 1, toSOPC_String("Describes a value that is a name qualified by a namespace.")}},
    {{0, 1, toSOPC_String("")},
     {79, 1, toSOPC_String("Describes a value that is human readable Unicode text with a locale identifier.")}},
    {{0, 1, toSOPC_String("")},
     {91, 1,
      toSOPC_String("Describes a value that is any type of structure that can be described with a data encoding.")}},
    {{0, 1, toSOPC_String("")},
     {87, 1, toSOPC_String("Describes a value that is a structure containing a value, a status code and timestamps.")}},
    {{0, 1, toSOPC_String("")},
     {90, 1,
      toSOPC_String("Describes a value that is a structure containing diagnostics associated with a StatusCode.")}},
    {{0, 1, toSOPC_String("")},
     {64, 1, toSOPC_String("Describes a value that is an image encoded as a string of bytes.")}},
    {{0, 1, toSOPC_String("")}, {34, 1, toSOPC_String("Describes a 128-bit decimal value.")}}

};
static int Description_begin[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,   8,   8,   8,   8,   8,
    8,  8,  9,  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,  30,  31,  32,  32,  33,
    34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 52, 52, 52,  53,  54,  55,  56,  57,
    58, 59, 59, 59, 59, 60, 61, 62, 63, 64, 65, 65, 65, 65, 66, 67, 68, 69, 70, 71, 72, 73,  74,  75,  76,  77,  78,
    79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105};
static int Description_end[] = {
    -1, 1,  2,  3,  4,  5,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,   7,   7,   7,   7,   7,
    7,  8,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,  30,  31,  31,  32,  33,
    34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 51, 51, 51, 52,  53,  54,  55,  56,  57,
    58, 58, 58, 58, 59, 60, 61, 62, 63, 64, 64, 64, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73,  74,  75,  76,  77,  78,
    79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105};
static SOPC_LocalizedText DisplayName[] = {
    {{0, 0, NULL}, {0, 0, NULL}},
    {{0, 1, toSOPC_String("")}, {4, 1, toSOPC_String("Root")}},
    {{0, 1, toSOPC_String("")}, {7, 1, toSOPC_String("Objects")}},
    {{0, 1, toSOPC_String("")}, {5, 1, toSOPC_String("Types")}},
    {{0, 1, toSOPC_String("")}, {11, 1, toSOPC_String("ObjectTypes")}},
    {{0, 1, toSOPC_String("")}, {13, 1, toSOPC_String("VariableTypes")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("DataTypes")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("ReferenceTypes")}},
    {{0, 1, toSOPC_String("")}, {20, 1, toSOPC_String("SessionPlaceHolder 1")}},
    {{0, 1, toSOPC_String("")}, {20, 1, toSOPC_String("SessionPlaceHolder 2")}},
    {{0, 1, toSOPC_String("")}, {20, 1, toSOPC_String("SessionPlaceHolder 3")}},
    {{0, 1, toSOPC_String("")}, {20, 1, toSOPC_String("SessionPlaceHolder 4")}},
    {{0, 1, toSOPC_String("")}, {20, 1, toSOPC_String("SessionPlaceHolder 5")}},
    {{0, 1, toSOPC_String("")}, {20, 1, toSOPC_String("SessionPlaceHolder 6")}},
    {{0, 1, toSOPC_String("")}, {20, 1, toSOPC_String("SessionPlaceHolder 7")}},
    {{0, 1, toSOPC_String("")}, {20, 1, toSOPC_String("SessionPlaceHolder 8")}},
    {{0, 1, toSOPC_String("")}, {20, 1, toSOPC_String("SessionPlaceHolder 9")}},
    {{0, 1, toSOPC_String("")}, {21, 1, toSOPC_String("SessionPlaceHolder 10")}},
    {{0, 1, toSOPC_String("")}, {21, 1, toSOPC_String("SessionPlaceHolder 11")}},
    {{0, 1, toSOPC_String("")}, {21, 1, toSOPC_String("SessionPlaceHolder 12")}},
    {{0, 1, toSOPC_String("")}, {21, 1, toSOPC_String("SessionPlaceHolder 13")}},
    {{0, 1, toSOPC_String("")}, {21, 1, toSOPC_String("SessionPlaceHolder 14")}},
    {{0, 1, toSOPC_String("")}, {21, 1, toSOPC_String("SessionPlaceHolder 15")}},
    {{0, 1, toSOPC_String("")}, {21, 1, toSOPC_String("SessionPlaceHolder 16")}},
    {{0, 1, toSOPC_String("")}, {21, 1, toSOPC_String("SessionPlaceHolder 17")}},
    {{0, 1, toSOPC_String("")}, {21, 1, toSOPC_String("SessionPlaceHolder 18")}},
    {{0, 1, toSOPC_String("")}, {21, 1, toSOPC_String("SessionPlaceHolder 19")}},
    {{0, 1, toSOPC_String("")}, {21, 1, toSOPC_String("SessionPlaceHolder 20")}},
    {{0, 1, toSOPC_String("")}, {17, 1, toSOPC_String("VariablesFolderDn")}},
    {{0, 1, toSOPC_String("")}, {5, 1, toSOPC_String("15361")}},
    {{0, 1, toSOPC_String("")}, {7, 1, toSOPC_String("SIGNALs")}},
    {{0, 1, toSOPC_String("")}, {7, 1, toSOPC_String("SWITCHs")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("TRACKs")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("BALA_RDLS_G019")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("RM")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("RC")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("BALA_RDLS_G025")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("RM")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("RC")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("BALA_RDLS_G026")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("RM")}},
    {{0, 1, toSOPC_String("")}, {12, 1, toSOPC_String("BALA_RDLS_W1")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("RM")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("RC")}},
    {{0, 1, toSOPC_String("")}, {15, 1, toSOPC_String("BALA_RDLS_026TK")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("RM")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("BALA_RDLS_OSTK")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("RM")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("BALA_RDLS_P500")}},
    {{0, 1, toSOPC_String("")}, {22, 1, toSOPC_String("BALA_RDLS_WBK_RDLN_EBK")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("RM")}},
    {{0, 1, toSOPC_String("")}, {5, 1, toSOPC_String("State")}},
    {{0, 1, toSOPC_String("")}, {11, 1, toSOPC_String("EnabledFlag")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("Int64_1dn")}},
    {{0, 1, toSOPC_String("")}, {10, 1, toSOPC_String("UInt32_1dn")}},
    {{0, 1, toSOPC_String("")}, {10, 1, toSOPC_String("Double_1dn")}},
    {{0, 1, toSOPC_String("")}, {10, 1, toSOPC_String("String_1dn")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("ByteString_1dn")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("XmlElement_1dn")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("SByte_1dn")}},
    {{0, 1, toSOPC_String("")}, {8, 1, toSOPC_String("Byte_1dn")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("Int16_1dn")}},
    {{0, 1, toSOPC_String("")}, {10, 1, toSOPC_String("UInt16_1dn")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("Int32_1dn")}},
    {{0, 1, toSOPC_String("")}, {10, 1, toSOPC_String("UInt64_1dn")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("Float_1dn")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("GK")}},
    {{0, 1, toSOPC_String("")}, {3, 1, toSOPC_String("ASK")}},
    {{0, 1, toSOPC_String("")}, {4, 1, toSOPC_String("XBKK")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("XBZCRQ")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("XBZ-AK")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("XBZCRQ-AK")}},
    {{0, 1, toSOPC_String("")}, {11, 1, toSOPC_String("SendCommand")}},
    {{0, 1, toSOPC_String("")}, {13, 1, toSOPC_String("OffBlocking-K")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("OffBlocking-CC")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("GZ")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("SZ")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("XBZ-CC")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("XBZCRQ-CC")}},
    {{0, 1, toSOPC_String("")}, {7, 1, toSOPC_String("XBZC-CC")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("GK")}},
    {{0, 1, toSOPC_String("")}, {3, 1, toSOPC_String("ASK")}},
    {{0, 1, toSOPC_String("")}, {11, 1, toSOPC_String("SendCommand")}},
    {{0, 1, toSOPC_String("")}, {13, 1, toSOPC_String("OffBlocking-K")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("OffBlocking-CC")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("SZ")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("GZ")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("GK")}},
    {{0, 1, toSOPC_String("")}, {3, 1, toSOPC_String("NWK")}},
    {{0, 1, toSOPC_String("")}, {3, 1, toSOPC_String("RWK")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("LK")}},
    {{0, 1, toSOPC_String("")}, {11, 1, toSOPC_String("SendCommand")}},
    {{0, 1, toSOPC_String("")}, {13, 1, toSOPC_String("OffBlocking-K")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("OffBlocking-CC")}},
    {{0, 1, toSOPC_String("")}, {3, 1, toSOPC_String("NWZ")}},
    {{0, 1, toSOPC_String("")}, {3, 1, toSOPC_String("RWZ")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("TK")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("TK")}},
    {{0, 1, toSOPC_String("")}, {2, 1, toSOPC_String("TK")}},
    {{0, 1, toSOPC_String("")}, {16, 1, toSOPC_String("BaseVariableType")}},
    {{0, 1, toSOPC_String("")}, {20, 1, toSOPC_String("BaseDataVariableType")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("BaseObjectType")}},
    {{0, 1, toSOPC_String("")}, {10, 1, toSOPC_String("FolderType")}},
    {{0, 1, toSOPC_String("")}, {10, 1, toSOPC_String("References")}},
    {{0, 1, toSOPC_String("")}, {12, 1, toSOPC_String("BaseDataType")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("Number")}},
    {{0, 1, toSOPC_String("")}, {7, 1, toSOPC_String("Integer")}},
    {{0, 1, toSOPC_String("")}, {8, 1, toSOPC_String("UInteger")}},
    {{0, 1, toSOPC_String("")}, {11, 1, toSOPC_String("Enumeration")}},
    {{0, 1, toSOPC_String("")}, {7, 1, toSOPC_String("Boolean")}},
    {{0, 1, toSOPC_String("")}, {5, 1, toSOPC_String("SByte")}},
    {{0, 1, toSOPC_String("")}, {4, 1, toSOPC_String("Byte")}},
    {{0, 1, toSOPC_String("")}, {5, 1, toSOPC_String("Int16")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("UInt16")}},
    {{0, 1, toSOPC_String("")}, {5, 1, toSOPC_String("Int32")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("UInt32")}},
    {{0, 1, toSOPC_String("")}, {5, 1, toSOPC_String("Int64")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("UInt64")}},
    {{0, 1, toSOPC_String("")}, {5, 1, toSOPC_String("Float")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("Double")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("String")}},
    {{0, 1, toSOPC_String("")}, {8, 1, toSOPC_String("DateTime")}},
    {{0, 1, toSOPC_String("")}, {4, 1, toSOPC_String("Guid")}},
    {{0, 1, toSOPC_String("")}, {10, 1, toSOPC_String("ByteString")}},
    {{0, 1, toSOPC_String("")}, {10, 1, toSOPC_String("XmlElement")}},
    {{0, 1, toSOPC_String("")}, {6, 1, toSOPC_String("NodeId")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("ExpandedNodeId")}},
    {{0, 1, toSOPC_String("")}, {10, 1, toSOPC_String("StatusCode")}},
    {{0, 1, toSOPC_String("")}, {13, 1, toSOPC_String("QualifiedName")}},
    {{0, 1, toSOPC_String("")}, {13, 1, toSOPC_String("LocalizedText")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("Structure")}},
    {{0, 1, toSOPC_String("")}, {9, 1, toSOPC_String("DataValue")}},
    {{0, 1, toSOPC_String("")}, {14, 1, toSOPC_String("DiagnosticInfo")}},
    {{0, 1, toSOPC_String("")}, {5, 1, toSOPC_String("Image")}},
    {{0, 1, toSOPC_String("")}, {10, 1, toSOPC_String("Decimal128")}}

};
static int DisplayName_begin[] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
    23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,
    46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,
    69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,
    92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
    115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134};
static int DisplayName_end[] = {
    -1,  1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
    23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,
    46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,
    69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,
    92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
    115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134};

static int reference_begin[] = {
    0,   1,   4,   8,   14,  17,  20,  50,  53,  53,  53,  53,  53,  53,  53,  53,  53,  53,  53,  53,  53,  53,  53,
    53,  53,  53,  53,  53,  53,  68,  72,  77,  80,  86,  93,  101, 108, 115, 119, 123, 126, 129, 136, 141, 145, 148,
    151, 154, 157, 159, 162, 165, 167, 167, 169, 171, 173, 175, 177, 179, 181, 183, 185, 187, 189, 191, 193, 195, 197,
    199, 201, 203, 205, 207, 209, 211, 213, 215, 217, 219, 221, 223, 225, 227, 229, 231, 233, 235, 237, 239, 241, 243,
    245, 247, 249, 251, 253, 255, 257, 259, 259, 260, 260, 261, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271,
    272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291};
static int reference_end[] = {-1,
                              0 + 3 /* i=84 */,
                              3 + 4 /* i=85 */,
                              7 + 6 /* i=86 */,
                              13 + 3 /* i=88 */,
                              16 + 3 /* i=89 */,
                              19 + 30 /* i=90 */,
                              49 + 3 /* i=91 */,
                              52 + 0 /* i=10001 */,
                              52 + 0 /* i=10002 */,
                              52 + 0 /* i=10003 */,
                              52 + 0 /* i=10004 */,
                              52 + 0 /* i=10005 */,
                              52 + 0 /* i=10006 */,
                              52 + 0 /* i=10007 */,
                              52 + 0 /* i=10008 */,
                              52 + 0 /* i=10009 */,
                              52 + 0 /* i=10010 */,
                              52 + 0 /* i=10011 */,
                              52 + 0 /* i=10012 */,
                              52 + 0 /* i=10013 */,
                              52 + 0 /* i=10014 */,
                              52 + 0 /* i=10015 */,
                              52 + 0 /* i=10016 */,
                              52 + 0 /* i=10017 */,
                              52 + 0 /* i=10018 */,
                              52 + 0 /* i=10019 */,
                              52 + 0 /* i=10020 */,
                              52 + 15 /* i=1000 */,
                              67 + 4 /* ns=261;i=15361 */,
                              71 + 5 /* ns=261;s=Objects.15361.SIGNALs */,
                              76 + 3 /* ns=261;s=Objects.15361.SWITCHs */,
                              79 + 6 /* ns=261;s=Objects.15361.TRACKs */,
                              85 + 7 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019 */,
                              92 + 8 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM */,
                              100 + 7 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC */,
                              107 + 7 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025 */,
                              114 + 4 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM */,
                              118 + 4 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC */,
                              122 + 3 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026 */,
                              125 + 3 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM */,
                              128 + 7 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1 */,
                              135 + 5 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM */,
                              140 + 4 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC */,
                              144 + 3 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK */,
                              147 + 3 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM */,
                              150 + 3 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK */,
                              153 + 3 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM */,
                              156 + 2 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_P500 */,
                              158 + 3 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK */,
                              161 + 3 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM */,
                              164 + 2 /* i=2259 */,
                              166 + 0 /* i=3114 */,
                              166 + 2 /* i=1001 */,
                              168 + 2 /* i=1002 */,
                              170 + 2 /* i=1003 */,
                              172 + 2 /* i=1004 */,
                              174 + 2 /* i=1005 */,
                              176 + 2 /* i=1006 */,
                              178 + 2 /* i=1007 */,
                              180 + 2 /* i=1008 */,
                              182 + 2 /* i=1009 */,
                              184 + 2 /* i=1010 */,
                              186 + 2 /* i=1011 */,
                              188 + 2 /* i=1012 */,
                              190 + 2 /* i=1013 */,
                              192 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK */,
                              194 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK */,
                              196 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK */,
                              198 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ */,
                              200 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK */,
                              202 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK */,
                              204 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand */,
                              206 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K */,
                              208 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC */,
                              210 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ */,
                              212 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ */,
                              214 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC */,
                              216 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC */,
                              218 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC */,
                              220 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK */,
                              222 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK */,
                              224 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand */,
                              226 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K */,
                              228 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC */,
                              230 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ */,
                              232 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ */,
                              234 + 2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK */,
                              236 + 2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK */,
                              238 + 2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK */,
                              240 + 2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK */,
                              242 + 2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand */,
                              244 + 2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K */,
                              246 + 2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC */,
                              248 + 2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ */,
                              250 + 2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ */,
                              252 + 2 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK */,
                              254 + 2 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK */,
                              256 + 2 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK */,
                              258 + 0 /* i=62 */,
                              258 + 1 /* i=63 */,
                              259 + 0 /* i=58 */,
                              259 + 1 /* i=61 */,
                              260 + 0 /* i=31 */,
                              260 + 1 /* i=24 */,
                              261 + 1 /* i=26 */,
                              262 + 1 /* i=27 */,
                              263 + 1 /* i=28 */,
                              264 + 1 /* i=29 */,
                              265 + 1 /* i=1 */,
                              266 + 1 /* i=2 */,
                              267 + 1 /* i=3 */,
                              268 + 1 /* i=4 */,
                              269 + 1 /* i=5 */,
                              270 + 1 /* i=6 */,
                              271 + 1 /* i=7 */,
                              272 + 1 /* i=8 */,
                              273 + 1 /* i=9 */,
                              274 + 1 /* i=10 */,
                              275 + 1 /* i=11 */,
                              276 + 1 /* i=12 */,
                              277 + 1 /* i=13 */,
                              278 + 1 /* i=14 */,
                              279 + 1 /* i=15 */,
                              280 + 1 /* i=16 */,
                              281 + 1 /* i=17 */,
                              282 + 1 /* i=18 */,
                              283 + 1 /* i=19 */,
                              284 + 1 /* i=20 */,
                              285 + 1 /* i=21 */,
                              286 + 1 /* i=22 */,
                              287 + 1 /* i=23 */,
                              288 + 1 /* i=25 */,
                              289 + 1 /* i=30 */,
                              290 + 1 /* i=121 */};
static SOPC_NodeId* reference_type[] = {
    NULL,        &nodeid_2,   &nodeid_2,   &nodeid_5,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_5,   &nodeid_2,
    &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_5,   &nodeid_2,   &nodeid_2,   &nodeid_5,   &nodeid_2,
    &nodeid_2,   &nodeid_5,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,
    &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,
    &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,
    &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_5,   &nodeid_2,   &nodeid_2,   &nodeid_5,   &nodeid_2,
    &nodeid_5,   &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_64,
    &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_5,   &nodeid_2,   &nodeid_2,   &nodeid_2,
    &nodeid_2,   &nodeid_5,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_5,   &nodeid_2,   &nodeid_2,
    &nodeid_5,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_2,   &nodeid_5,   &nodeid_2,   &nodeid_2,
    &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_2,   &nodeid_5,   &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_64,
    &nodeid_64,  &nodeid_64,  &nodeid_2,   &nodeid_5,   &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_64,
    &nodeid_2,   &nodeid_5,   &nodeid_2,   &nodeid_2,   &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_2,   &nodeid_5,
    &nodeid_64,  &nodeid_64,  &nodeid_2,   &nodeid_5,   &nodeid_64,  &nodeid_64,  &nodeid_2,   &nodeid_5,   &nodeid_2,
    &nodeid_2,   &nodeid_5,   &nodeid_64,  &nodeid_2,   &nodeid_5,   &nodeid_2,   &nodeid_2,   &nodeid_64,  &nodeid_64,
    &nodeid_64,  &nodeid_2,   &nodeid_5,   &nodeid_64,  &nodeid_64,  &nodeid_64,  &nodeid_2,   &nodeid_5,   &nodeid_64,
    &nodeid_64,  &nodeid_2,   &nodeid_5,   &nodeid_2,   &nodeid_2,   &nodeid_5,   &nodeid_64,  &nodeid_2,   &nodeid_5,
    &nodeid_2,   &nodeid_2,   &nodeid_5,   &nodeid_64,  &nodeid_2,   &nodeid_5,   &nodeid_2,   &nodeid_5,   &nodeid_2,
    &nodeid_2,   &nodeid_5,   &nodeid_64,  &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,
    &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,
    &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,
    &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,
    &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,
    &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,
    &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,
    &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,
    &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,
    &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,
    &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_64,  &nodeid_5,   &nodeid_135, &nodeid_135,
    &nodeid_2,   &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135,
    &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135,
    &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135,
    &nodeid_135, &nodeid_135, &nodeid_135, &nodeid_135};
static SOPC_ExpandedNodeId* reference_target[] = {
    NULL,           &ex_nodeid_3,   &ex_nodeid_4,   &ex_nodeid_6,   &ex_nodeid_7,   &ex_nodeid_8,   &ex_nodeid_1,
    &ex_nodeid_6,   &ex_nodeid_1,   &ex_nodeid_9,   &ex_nodeid_10,  &ex_nodeid_11,  &ex_nodeid_12,  &ex_nodeid_6,
    &ex_nodeid_4,   &ex_nodeid_13,  &ex_nodeid_6,   &ex_nodeid_4,   &ex_nodeid_14,  &ex_nodeid_6,   &ex_nodeid_4,
    &ex_nodeid_15,  &ex_nodeid_16,  &ex_nodeid_17,  &ex_nodeid_18,  &ex_nodeid_19,  &ex_nodeid_20,  &ex_nodeid_21,
    &ex_nodeid_22,  &ex_nodeid_23,  &ex_nodeid_24,  &ex_nodeid_25,  &ex_nodeid_26,  &ex_nodeid_27,  &ex_nodeid_28,
    &ex_nodeid_29,  &ex_nodeid_30,  &ex_nodeid_31,  &ex_nodeid_32,  &ex_nodeid_33,  &ex_nodeid_34,  &ex_nodeid_35,
    &ex_nodeid_36,  &ex_nodeid_37,  &ex_nodeid_38,  &ex_nodeid_39,  &ex_nodeid_40,  &ex_nodeid_41,  &ex_nodeid_42,
    &ex_nodeid_6,   &ex_nodeid_4,   &ex_nodeid_43,  &ex_nodeid_6,   &ex_nodeid_3,   &ex_nodeid_6,   &ex_nodeid_65,
    &ex_nodeid_66,  &ex_nodeid_67,  &ex_nodeid_68,  &ex_nodeid_69,  &ex_nodeid_70,  &ex_nodeid_71,  &ex_nodeid_72,
    &ex_nodeid_73,  &ex_nodeid_74,  &ex_nodeid_75,  &ex_nodeid_76,  &ex_nodeid_77,  &ex_nodeid_6,   &ex_nodeid_78,
    &ex_nodeid_79,  &ex_nodeid_80,  &ex_nodeid_8,   &ex_nodeid_6,   &ex_nodeid_81,  &ex_nodeid_82,  &ex_nodeid_83,
    &ex_nodeid_8,   &ex_nodeid_6,   &ex_nodeid_84,  &ex_nodeid_8,   &ex_nodeid_6,   &ex_nodeid_85,  &ex_nodeid_86,
    &ex_nodeid_87,  &ex_nodeid_88,  &ex_nodeid_78,  &ex_nodeid_6,   &ex_nodeid_89,  &ex_nodeid_90,  &ex_nodeid_91,
    &ex_nodeid_92,  &ex_nodeid_93,  &ex_nodeid_81,  &ex_nodeid_6,   &ex_nodeid_94,  &ex_nodeid_95,  &ex_nodeid_96,
    &ex_nodeid_97,  &ex_nodeid_98,  &ex_nodeid_99,  &ex_nodeid_81,  &ex_nodeid_6,   &ex_nodeid_100, &ex_nodeid_101,
    &ex_nodeid_102, &ex_nodeid_103, &ex_nodeid_104, &ex_nodeid_78,  &ex_nodeid_6,   &ex_nodeid_105, &ex_nodeid_106,
    &ex_nodeid_107, &ex_nodeid_108, &ex_nodeid_109, &ex_nodeid_82,  &ex_nodeid_6,   &ex_nodeid_110, &ex_nodeid_111,
    &ex_nodeid_82,  &ex_nodeid_6,   &ex_nodeid_112, &ex_nodeid_113, &ex_nodeid_78,  &ex_nodeid_6,   &ex_nodeid_114,
    &ex_nodeid_83,  &ex_nodeid_6,   &ex_nodeid_115, &ex_nodeid_79,  &ex_nodeid_6,   &ex_nodeid_116, &ex_nodeid_117,
    &ex_nodeid_118, &ex_nodeid_119, &ex_nodeid_120, &ex_nodeid_84,  &ex_nodeid_6,   &ex_nodeid_121, &ex_nodeid_122,
    &ex_nodeid_123, &ex_nodeid_84,  &ex_nodeid_6,   &ex_nodeid_124, &ex_nodeid_125, &ex_nodeid_80,  &ex_nodeid_6,
    &ex_nodeid_126, &ex_nodeid_85,  &ex_nodeid_6,   &ex_nodeid_127, &ex_nodeid_80,  &ex_nodeid_6,   &ex_nodeid_128,
    &ex_nodeid_86,  &ex_nodeid_6,   &ex_nodeid_129, &ex_nodeid_80,  &ex_nodeid_6,   &ex_nodeid_80,  &ex_nodeid_6,
    &ex_nodeid_130, &ex_nodeid_88,  &ex_nodeid_6,   &ex_nodeid_131, &ex_nodeid_7,   &ex_nodeid_133, &ex_nodeid_7,
    &ex_nodeid_133, &ex_nodeid_7,   &ex_nodeid_133, &ex_nodeid_7,   &ex_nodeid_133, &ex_nodeid_7,   &ex_nodeid_133,
    &ex_nodeid_7,   &ex_nodeid_133, &ex_nodeid_7,   &ex_nodeid_133, &ex_nodeid_7,   &ex_nodeid_133, &ex_nodeid_7,
    &ex_nodeid_133, &ex_nodeid_7,   &ex_nodeid_133, &ex_nodeid_7,   &ex_nodeid_133, &ex_nodeid_7,   &ex_nodeid_133,
    &ex_nodeid_7,   &ex_nodeid_133, &ex_nodeid_7,   &ex_nodeid_133, &ex_nodeid_89,  &ex_nodeid_133, &ex_nodeid_89,
    &ex_nodeid_133, &ex_nodeid_89,  &ex_nodeid_133, &ex_nodeid_89,  &ex_nodeid_133, &ex_nodeid_89,  &ex_nodeid_133,
    &ex_nodeid_89,  &ex_nodeid_133, &ex_nodeid_81,  &ex_nodeid_133, &ex_nodeid_81,  &ex_nodeid_133, &ex_nodeid_81,
    &ex_nodeid_133, &ex_nodeid_90,  &ex_nodeid_133, &ex_nodeid_90,  &ex_nodeid_133, &ex_nodeid_90,  &ex_nodeid_133,
    &ex_nodeid_90,  &ex_nodeid_133, &ex_nodeid_90,  &ex_nodeid_133, &ex_nodeid_105, &ex_nodeid_133, &ex_nodeid_105,
    &ex_nodeid_133, &ex_nodeid_82,  &ex_nodeid_133, &ex_nodeid_82,  &ex_nodeid_133, &ex_nodeid_82,  &ex_nodeid_133,
    &ex_nodeid_106, &ex_nodeid_133, &ex_nodeid_106, &ex_nodeid_133, &ex_nodeid_114, &ex_nodeid_133, &ex_nodeid_116,
    &ex_nodeid_133, &ex_nodeid_116, &ex_nodeid_133, &ex_nodeid_116, &ex_nodeid_133, &ex_nodeid_84,  &ex_nodeid_133,
    &ex_nodeid_84,  &ex_nodeid_133, &ex_nodeid_84,  &ex_nodeid_133, &ex_nodeid_117, &ex_nodeid_133, &ex_nodeid_117,
    &ex_nodeid_133, &ex_nodeid_126, &ex_nodeid_133, &ex_nodeid_128, &ex_nodeid_133, &ex_nodeid_130, &ex_nodeid_133,
    &ex_nodeid_14,  &ex_nodeid_13,  &ex_nodeid_4,   &ex_nodeid_15,  &ex_nodeid_16,  &ex_nodeid_16,  &ex_nodeid_15,
    &ex_nodeid_15,  &ex_nodeid_17,  &ex_nodeid_18,  &ex_nodeid_17,  &ex_nodeid_18,  &ex_nodeid_17,  &ex_nodeid_18,
    &ex_nodeid_17,  &ex_nodeid_18,  &ex_nodeid_16,  &ex_nodeid_16,  &ex_nodeid_15,  &ex_nodeid_15,  &ex_nodeid_15,
    &ex_nodeid_15,  &ex_nodeid_15,  &ex_nodeid_15,  &ex_nodeid_15,  &ex_nodeid_15,  &ex_nodeid_15,  &ex_nodeid_15,
    &ex_nodeid_15,  &ex_nodeid_15,  &ex_nodeid_15,  &ex_nodeid_34,  &ex_nodeid_16};
static bool reference_isForward[] = {
    false, true,  true,  true,  true,  true,  false, true,  false, true,  true,  true,  true,  true,  false, true,
    true,  false, true,  true,  false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
    true,  true,  false, true,  true,  false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
    true,  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  true,  false, true,  true,
    false, true,  true,  true,  true,  true,  false, true,  true,  true,  true,  true,  true,  false, true,  true,
    true,  true,  true,  true,  true,  false, true,  true,  true,  true,  true,  true,  false, true,  true,  true,
    true,  true,  true,  false, true,  true,  true,  false, true,  true,  true,  false, true,  true,  false, true,
    true,  false, true,  true,  true,  true,  true,  true,  false, true,  true,  true,  true,  false, true,  true,
    true,  false, true,  true,  false, true,  true,  false, true,  true,  false, true,  true,  false, true,  false,
    true,  true,  false, true,  true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false};

static SOPC_NodeId* NodeId[NB + 1] = {
    NULL,        &nodeid_1,   &nodeid_3,   &nodeid_4,   &nodeid_9,   &nodeid_10,  &nodeid_11,  &nodeid_12,
    &nodeid_44,  &nodeid_45,  &nodeid_46,  &nodeid_47,  &nodeid_48,  &nodeid_49,  &nodeid_50,  &nodeid_51,
    &nodeid_52,  &nodeid_53,  &nodeid_54,  &nodeid_55,  &nodeid_56,  &nodeid_57,  &nodeid_58,  &nodeid_59,
    &nodeid_60,  &nodeid_61,  &nodeid_62,  &nodeid_63,  &nodeid_7,   &nodeid_8,   &nodeid_78,  &nodeid_79,
    &nodeid_80,  &nodeid_81,  &nodeid_89,  &nodeid_90,  &nodeid_82,  &nodeid_105, &nodeid_106, &nodeid_83,
    &nodeid_114, &nodeid_84,  &nodeid_116, &nodeid_117, &nodeid_85,  &nodeid_126, &nodeid_86,  &nodeid_128,
    &nodeid_87,  &nodeid_88,  &nodeid_130, &nodeid_132, &nodeid_134, &nodeid_65,  &nodeid_66,  &nodeid_67,
    &nodeid_68,  &nodeid_69,  &nodeid_70,  &nodeid_71,  &nodeid_72,  &nodeid_73,  &nodeid_74,  &nodeid_75,
    &nodeid_76,  &nodeid_77,  &nodeid_94,  &nodeid_95,  &nodeid_96,  &nodeid_97,  &nodeid_98,  &nodeid_99,
    &nodeid_91,  &nodeid_92,  &nodeid_93,  &nodeid_100, &nodeid_101, &nodeid_102, &nodeid_103, &nodeid_104,
    &nodeid_110, &nodeid_111, &nodeid_107, &nodeid_108, &nodeid_109, &nodeid_112, &nodeid_113, &nodeid_115,
    &nodeid_121, &nodeid_122, &nodeid_123, &nodeid_118, &nodeid_119, &nodeid_120, &nodeid_124, &nodeid_125,
    &nodeid_127, &nodeid_129, &nodeid_131, &nodeid_14,  &nodeid_133, &nodeid_13,  &nodeid_6,   &nodeid_43,
    &nodeid_15,  &nodeid_16,  &nodeid_17,  &nodeid_18,  &nodeid_19,  &nodeid_20,  &nodeid_21,  &nodeid_22,
    &nodeid_23,  &nodeid_24,  &nodeid_25,  &nodeid_26,  &nodeid_27,  &nodeid_28,  &nodeid_29,  &nodeid_30,
    &nodeid_31,  &nodeid_32,  &nodeid_33,  &nodeid_34,  &nodeid_35,  &nodeid_36,  &nodeid_37,  &nodeid_38,
    &nodeid_39,  &nodeid_40,  &nodeid_41,  &nodeid_42,  &nodeid_136, &nodeid_137, &nodeid_138};

static OpcUa_NodeClass NodeClass[NB + 1] = {
    OpcUa_NodeClass_Unspecified,  OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,
    OpcUa_NodeClass_Object,       OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,
    OpcUa_NodeClass_Object,       OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,
    OpcUa_NodeClass_Object,       OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,
    OpcUa_NodeClass_Object,       OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,
    OpcUa_NodeClass_Object,       OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,
    OpcUa_NodeClass_Object,       OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,
    OpcUa_NodeClass_Object,       OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,
    OpcUa_NodeClass_Object,       OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,
    OpcUa_NodeClass_Object,       OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,
    OpcUa_NodeClass_Object,       OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,
    OpcUa_NodeClass_Object,       OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,
    OpcUa_NodeClass_Object,       OpcUa_NodeClass_Object,     OpcUa_NodeClass_Object,     OpcUa_NodeClass_Variable,
    OpcUa_NodeClass_Variable,     OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,
    OpcUa_NodeClass_Variable,     OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,
    OpcUa_NodeClass_Variable,     OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,
    OpcUa_NodeClass_Variable,     OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,
    OpcUa_NodeClass_Variable,     OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,
    OpcUa_NodeClass_Variable,     OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,
    OpcUa_NodeClass_Variable,     OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,
    OpcUa_NodeClass_Variable,     OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,
    OpcUa_NodeClass_Variable,     OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,
    OpcUa_NodeClass_Variable,     OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,
    OpcUa_NodeClass_Variable,     OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,
    OpcUa_NodeClass_Variable,     OpcUa_NodeClass_Variable,   OpcUa_NodeClass_Variable,   OpcUa_NodeClass_VariableType,
    OpcUa_NodeClass_VariableType, OpcUa_NodeClass_ObjectType, OpcUa_NodeClass_ObjectType, OpcUa_NodeClass_ReferenceType,
    OpcUa_NodeClass_DataType,     OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,
    OpcUa_NodeClass_DataType,     OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,
    OpcUa_NodeClass_DataType,     OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,
    OpcUa_NodeClass_DataType,     OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,
    OpcUa_NodeClass_DataType,     OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,
    OpcUa_NodeClass_DataType,     OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,
    OpcUa_NodeClass_DataType,     OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType,
    OpcUa_NodeClass_DataType,     OpcUa_NodeClass_DataType,   OpcUa_NodeClass_DataType};

SOPC_Variant Value[NB_3 + NB_4 + 1] = {
    DEFAULT_VARIANT,
    {SOPC_Int32_Id, SOPC_VariantArrayType_SingleValue, {.Int32 = 0}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Int64_Id, SOPC_VariantArrayType_SingleValue, {.Int64 = -1000}},
    {SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue, {.Uint32 = 1000}},
    {SOPC_Double_Id, SOPC_VariantArrayType_SingleValue, {.Doublev = 2.0}},
    {SOPC_String_Id, SOPC_VariantArrayType_SingleValue, {.String = {14, 1, toSOPC_String("String:INGOPCS")}}},
    {SOPC_ByteString_Id, SOPC_VariantArrayType_SingleValue, {.Bstring = {18, 1, toSOPC_String("ByteString:INGOPCS")}}},
    {SOPC_XmlElement_Id, SOPC_VariantArrayType_SingleValue, {.XmlElt = {18, 1, toSOPC_String("XmlElement:INGOPCS")}}},
    {SOPC_SByte_Id, SOPC_VariantArrayType_SingleValue, {.Sbyte = -127}},
    {SOPC_Byte_Id, SOPC_VariantArrayType_SingleValue, {.Byte = 255}},
    {SOPC_Int16_Id, SOPC_VariantArrayType_SingleValue, {.Int16 = -32767}},
    {SOPC_UInt16_Id, SOPC_VariantArrayType_SingleValue, {.Uint16 = 65535}},
    {SOPC_Int32_Id, SOPC_VariantArrayType_SingleValue, {.Int32 = -2147483647}},
    {SOPC_UInt64_Id, SOPC_VariantArrayType_SingleValue, {.Uint64 = 1844674407370955}},
    {SOPC_Float_Id, SOPC_VariantArrayType_SingleValue, {.Floatv = 109517.875}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    DEFAULT_VARIANT,
    DEFAULT_VARIANT,
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    DEFAULT_VARIANT,
    DEFAULT_VARIANT,
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    DEFAULT_VARIANT,
    DEFAULT_VARIANT,
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = true}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = true}},
    {SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}},
    DEFAULT_VARIANT,
    DEFAULT_VARIANT};

static SOPC_StatusCode status_code[] = {OpcUa_BadDataUnavailable,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        OpcUa_BadDataUnavailable,
                                        OpcUa_BadDataUnavailable,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        OpcUa_BadDataUnavailable,
                                        OpcUa_BadDataUnavailable,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        OpcUa_BadDataUnavailable,
                                        OpcUa_BadDataUnavailable,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        0x00000000,
                                        OpcUa_BadDataUnavailable,
                                        OpcUa_BadDataUnavailable};

static SOPC_SByte AccessLevel[] = {0, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1,
                                   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

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
