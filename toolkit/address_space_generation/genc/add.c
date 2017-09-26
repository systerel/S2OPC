
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
#define NB_2 23    /* Object */
#define NB_3 37    /* Variable */
#define NB_4 0    /* VariableType */
#define NB_5 0    /* ObjectType */
#define NB_6 0    /* ReferenceType */
#define NB_7 0    /* DataType */
#define NB_8 0    /* Method */

#define NB (NB_1 + NB_2 + NB_3 + NB_4 + NB_5 + NB_6 + NB_7 + NB_8)

#define toSOPC_String(s) ((SOPC_Byte*)s)



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
static SOPC_NodeId nodeid_11 = {IdentifierType_Numeric, 261, .Data.Numeric = 15361};
static SOPC_ExpandedNodeId ex_nodeid_11 = {{IdentifierType_Numeric, 261, .Data.Numeric = 15361}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_12 = {IdentifierType_String, 261, .Data.String = {21,0,toSOPC_String("Objects.15361.SIGNALs")}};
static SOPC_ExpandedNodeId ex_nodeid_12 = {{IdentifierType_String, 261, .Data.String = {21,0,toSOPC_String("Objects.15361.SIGNALs")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_13 = {IdentifierType_String, 261, .Data.String = {21,0,toSOPC_String("Objects.15361.SWITCHs")}};
static SOPC_ExpandedNodeId ex_nodeid_13 = {{IdentifierType_String, 261, .Data.String = {21,0,toSOPC_String("Objects.15361.SWITCHs")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_14 = {IdentifierType_String, 261, .Data.String = {20,0,toSOPC_String("Objects.15361.TRACKs")}};
static SOPC_ExpandedNodeId ex_nodeid_14 = {{IdentifierType_String, 261, .Data.String = {20,0,toSOPC_String("Objects.15361.TRACKs")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_15 = {IdentifierType_String, 261, .Data.String = {36,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019")}};
static SOPC_ExpandedNodeId ex_nodeid_15 = {{IdentifierType_String, 261, .Data.String = {36,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_16 = {IdentifierType_String, 261, .Data.String = {36,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025")}};
static SOPC_ExpandedNodeId ex_nodeid_16 = {{IdentifierType_String, 261, .Data.String = {36,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_17 = {IdentifierType_String, 261, .Data.String = {36,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G026")}};
static SOPC_ExpandedNodeId ex_nodeid_17 = {{IdentifierType_String, 261, .Data.String = {36,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G026")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_18 = {IdentifierType_String, 261, .Data.String = {34,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1")}};
static SOPC_ExpandedNodeId ex_nodeid_18 = {{IdentifierType_String, 261, .Data.String = {34,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_19 = {IdentifierType_String, 261, .Data.String = {36,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_026TK")}};
static SOPC_ExpandedNodeId ex_nodeid_19 = {{IdentifierType_String, 261, .Data.String = {36,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_026TK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_20 = {IdentifierType_String, 261, .Data.String = {35,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_OSTK")}};
static SOPC_ExpandedNodeId ex_nodeid_20 = {{IdentifierType_String, 261, .Data.String = {35,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_OSTK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_21 = {IdentifierType_String, 261, .Data.String = {35,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_P500")}};
static SOPC_ExpandedNodeId ex_nodeid_21 = {{IdentifierType_String, 261, .Data.String = {35,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_P500")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_22 = {IdentifierType_String, 261, .Data.String = {43,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK")}};
static SOPC_ExpandedNodeId ex_nodeid_22 = {{IdentifierType_String, 261, .Data.String = {43,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_23 = {IdentifierType_String, 261, .Data.String = {39,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_23 = {{IdentifierType_String, 261, .Data.String = {39,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_24 = {IdentifierType_String, 261, .Data.String = {39,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC")}};
static SOPC_ExpandedNodeId ex_nodeid_24 = {{IdentifierType_String, 261, .Data.String = {39,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_25 = {IdentifierType_String, 261, .Data.String = {48,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand")}};
static SOPC_ExpandedNodeId ex_nodeid_25 = {{IdentifierType_String, 261, .Data.String = {48,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_26 = {IdentifierType_String, 261, .Data.String = {50,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K")}};
static SOPC_ExpandedNodeId ex_nodeid_26 = {{IdentifierType_String, 261, .Data.String = {50,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_27 = {IdentifierType_String, 261, .Data.String = {51,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC")}};
static SOPC_ExpandedNodeId ex_nodeid_27 = {{IdentifierType_String, 261, .Data.String = {51,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_28 = {IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK")}};
static SOPC_ExpandedNodeId ex_nodeid_28 = {{IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_29 = {IdentifierType_String, 261, .Data.String = {43,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK")}};
static SOPC_ExpandedNodeId ex_nodeid_29 = {{IdentifierType_String, 261, .Data.String = {43,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_30 = {IdentifierType_String, 261, .Data.String = {44,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK")}};
static SOPC_ExpandedNodeId ex_nodeid_30 = {{IdentifierType_String, 261, .Data.String = {44,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_31 = {IdentifierType_String, 261, .Data.String = {46,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ")}};
static SOPC_ExpandedNodeId ex_nodeid_31 = {{IdentifierType_String, 261, .Data.String = {46,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_32 = {IdentifierType_String, 261, .Data.String = {46,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK")}};
static SOPC_ExpandedNodeId ex_nodeid_32 = {{IdentifierType_String, 261, .Data.String = {46,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_33 = {IdentifierType_String, 261, .Data.String = {49,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK")}};
static SOPC_ExpandedNodeId ex_nodeid_33 = {{IdentifierType_String, 261, .Data.String = {49,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_34 = {IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ")}};
static SOPC_ExpandedNodeId ex_nodeid_34 = {{IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_35 = {IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ")}};
static SOPC_ExpandedNodeId ex_nodeid_35 = {{IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_36 = {IdentifierType_String, 261, .Data.String = {46,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC")}};
static SOPC_ExpandedNodeId ex_nodeid_36 = {{IdentifierType_String, 261, .Data.String = {46,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_37 = {IdentifierType_String, 261, .Data.String = {49,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC")}};
static SOPC_ExpandedNodeId ex_nodeid_37 = {{IdentifierType_String, 261, .Data.String = {49,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_38 = {IdentifierType_String, 261, .Data.String = {47,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC")}};
static SOPC_ExpandedNodeId ex_nodeid_38 = {{IdentifierType_String, 261, .Data.String = {47,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_39 = {IdentifierType_String, 261, .Data.String = {39,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_39 = {{IdentifierType_String, 261, .Data.String = {39,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RM")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_40 = {IdentifierType_String, 261, .Data.String = {39,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RC")}};
static SOPC_ExpandedNodeId ex_nodeid_40 = {{IdentifierType_String, 261, .Data.String = {39,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RC")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_41 = {IdentifierType_String, 261, .Data.String = {48,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand")}};
static SOPC_ExpandedNodeId ex_nodeid_41 = {{IdentifierType_String, 261, .Data.String = {48,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_42 = {IdentifierType_String, 261, .Data.String = {50,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K")}};
static SOPC_ExpandedNodeId ex_nodeid_42 = {{IdentifierType_String, 261, .Data.String = {50,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_43 = {IdentifierType_String, 261, .Data.String = {51,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC")}};
static SOPC_ExpandedNodeId ex_nodeid_43 = {{IdentifierType_String, 261, .Data.String = {51,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_44 = {IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK")}};
static SOPC_ExpandedNodeId ex_nodeid_44 = {{IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_45 = {IdentifierType_String, 261, .Data.String = {43,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK")}};
static SOPC_ExpandedNodeId ex_nodeid_45 = {{IdentifierType_String, 261, .Data.String = {43,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_46 = {IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ")}};
static SOPC_ExpandedNodeId ex_nodeid_46 = {{IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_47 = {IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ")}};
static SOPC_ExpandedNodeId ex_nodeid_47 = {{IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_48 = {IdentifierType_String, 261, .Data.String = {39,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G026.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_48 = {{IdentifierType_String, 261, .Data.String = {39,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G026.RM")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_49 = {IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK")}};
static SOPC_ExpandedNodeId ex_nodeid_49 = {{IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_50 = {IdentifierType_String, 261, .Data.String = {37,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_50 = {{IdentifierType_String, 261, .Data.String = {37,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_51 = {IdentifierType_String, 261, .Data.String = {37,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RC")}};
static SOPC_ExpandedNodeId ex_nodeid_51 = {{IdentifierType_String, 261, .Data.String = {37,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RC")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_52 = {IdentifierType_String, 261, .Data.String = {46,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand")}};
static SOPC_ExpandedNodeId ex_nodeid_52 = {{IdentifierType_String, 261, .Data.String = {46,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_53 = {IdentifierType_String, 261, .Data.String = {48,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K")}};
static SOPC_ExpandedNodeId ex_nodeid_53 = {{IdentifierType_String, 261, .Data.String = {48,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_54 = {IdentifierType_String, 261, .Data.String = {49,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC")}};
static SOPC_ExpandedNodeId ex_nodeid_54 = {{IdentifierType_String, 261, .Data.String = {49,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_55 = {IdentifierType_String, 261, .Data.String = {41,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK")}};
static SOPC_ExpandedNodeId ex_nodeid_55 = {{IdentifierType_String, 261, .Data.String = {41,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_56 = {IdentifierType_String, 261, .Data.String = {41,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK")}};
static SOPC_ExpandedNodeId ex_nodeid_56 = {{IdentifierType_String, 261, .Data.String = {41,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_57 = {IdentifierType_String, 261, .Data.String = {40,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK")}};
static SOPC_ExpandedNodeId ex_nodeid_57 = {{IdentifierType_String, 261, .Data.String = {40,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_58 = {IdentifierType_String, 261, .Data.String = {41,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ")}};
static SOPC_ExpandedNodeId ex_nodeid_58 = {{IdentifierType_String, 261, .Data.String = {41,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_59 = {IdentifierType_String, 261, .Data.String = {41,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ")}};
static SOPC_ExpandedNodeId ex_nodeid_59 = {{IdentifierType_String, 261, .Data.String = {41,0,toSOPC_String("Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_60 = {IdentifierType_String, 261, .Data.String = {39,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_026TK.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_60 = {{IdentifierType_String, 261, .Data.String = {39,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_026TK.RM")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_61 = {IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK")}};
static SOPC_ExpandedNodeId ex_nodeid_61 = {{IdentifierType_String, 261, .Data.String = {42,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_62 = {IdentifierType_String, 261, .Data.String = {38,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_OSTK.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_62 = {{IdentifierType_String, 261, .Data.String = {38,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_OSTK.RM")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_63 = {IdentifierType_String, 261, .Data.String = {41,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK")}};
static SOPC_ExpandedNodeId ex_nodeid_63 = {{IdentifierType_String, 261, .Data.String = {41,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_64 = {IdentifierType_String, 261, .Data.String = {46,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM")}};
static SOPC_ExpandedNodeId ex_nodeid_64 = {{IdentifierType_String, 261, .Data.String = {46,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_65 = {IdentifierType_String, 261, .Data.String = {49,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK")}};
static SOPC_ExpandedNodeId ex_nodeid_65 = {{IdentifierType_String, 261, .Data.String = {49,0,toSOPC_String("Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK")}}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_66 = {IdentifierType_Numeric, 0, .Data.Numeric = 63};
static SOPC_ExpandedNodeId ex_nodeid_66 = {{IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0,0, NULL}, 0};
static SOPC_NodeId nodeid_67 = {IdentifierType_String, 0, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK")}};
static SOPC_ExpandedNodeId ex_nodeid_67 = {{IdentifierType_String, 0, .Data.String = {42,0,toSOPC_String("Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK")}}, {0,0, NULL}, 0};


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
&ex_nodeid_13,&nodeid_14,
&ex_nodeid_14,&nodeid_15,
&ex_nodeid_15,&nodeid_16,
&ex_nodeid_16,&nodeid_17,
&ex_nodeid_17,&nodeid_18,
&ex_nodeid_18,&nodeid_19,
&ex_nodeid_19,&nodeid_20,
&ex_nodeid_20,&nodeid_21,
&ex_nodeid_21,&nodeid_22,
&ex_nodeid_22,&nodeid_23,
&ex_nodeid_23,&nodeid_24,
&ex_nodeid_24,&nodeid_25,
&ex_nodeid_25,&nodeid_26,
&ex_nodeid_26,&nodeid_27,
&ex_nodeid_27,&nodeid_28,
&ex_nodeid_28,&nodeid_29,
&ex_nodeid_29,&nodeid_30,
&ex_nodeid_30,&nodeid_31,
&ex_nodeid_31,&nodeid_32,
&ex_nodeid_32,&nodeid_33,
&ex_nodeid_33,&nodeid_34,
&ex_nodeid_34,&nodeid_35,
&ex_nodeid_35,&nodeid_36,
&ex_nodeid_36,&nodeid_37,
&ex_nodeid_37,&nodeid_38,
&ex_nodeid_38,&nodeid_39,
&ex_nodeid_39,&nodeid_40,
&ex_nodeid_40,&nodeid_41,
&ex_nodeid_41,&nodeid_42,
&ex_nodeid_42,&nodeid_43,
&ex_nodeid_43,&nodeid_44,
&ex_nodeid_44,&nodeid_45,
&ex_nodeid_45,&nodeid_46,
&ex_nodeid_46,&nodeid_47,
&ex_nodeid_47,&nodeid_48,
&ex_nodeid_48,&nodeid_49,
&ex_nodeid_49,&nodeid_50,
&ex_nodeid_50,&nodeid_51,
&ex_nodeid_51,&nodeid_52,
&ex_nodeid_52,&nodeid_53,
&ex_nodeid_53,&nodeid_54,
&ex_nodeid_54,&nodeid_55,
&ex_nodeid_55,&nodeid_56,
&ex_nodeid_56,&nodeid_57,
&ex_nodeid_57,&nodeid_58,
&ex_nodeid_58,&nodeid_59,
&ex_nodeid_59,&nodeid_60,
&ex_nodeid_60,&nodeid_61,
&ex_nodeid_61,&nodeid_62,
&ex_nodeid_62,&nodeid_63,
&ex_nodeid_63,&nodeid_64,
&ex_nodeid_64,&nodeid_65,
&ex_nodeid_65,&nodeid_66,
&ex_nodeid_66,&nodeid_67,
&ex_nodeid_67,};


static SOPC_QualifiedName BrowseName[NB + 1] = {{0, {0, 0, NULL}}
,{0,{17,0,toSOPC_String("VariablesFolderBn")}}/* i=1000*/
,{261,{5,0,toSOPC_String("15361")}}/* ns=261;i=15361*/
,{0,{7,0,toSOPC_String("SIGNALs")}}/* ns=261;s=Objects.15361.SIGNALs*/
,{0,{7,0,toSOPC_String("SWITCHs")}}/* ns=261;s=Objects.15361.SWITCHs*/
,{0,{6,0,toSOPC_String("TRACKs")}}/* ns=261;s=Objects.15361.TRACKs*/
,{0,{14,0,toSOPC_String("BALA_RDLS_G019")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019*/
,{0,{2,0,toSOPC_String("RM")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM*/
,{0,{2,0,toSOPC_String("RC")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC*/
,{0,{14,0,toSOPC_String("BALA_RDLS_G025")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025*/
,{0,{2,0,toSOPC_String("RM")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM*/
,{0,{2,0,toSOPC_String("RC")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC*/
,{0,{14,0,toSOPC_String("BALA_RDLS_G026")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026*/
,{0,{2,0,toSOPC_String("RM")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM*/
,{0,{12,0,toSOPC_String("BALA_RDLS_W1")}}/* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1*/
,{0,{2,0,toSOPC_String("RM")}}/* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM*/
,{0,{2,0,toSOPC_String("RC")}}/* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC*/
,{0,{15,0,toSOPC_String("BALA_RDLS_026TK")}}/* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK*/
,{0,{2,0,toSOPC_String("RM")}}/* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM*/
,{0,{14,0,toSOPC_String("BALA_RDLS_OSTK")}}/* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK*/
,{0,{2,0,toSOPC_String("RM")}}/* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM*/
,{0,{14,0,toSOPC_String("BALA_RDLS_P500")}}/* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_P500*/
,{0,{22,0,toSOPC_String("BALA_RDLS_WBK_RDLN_EBK")}}/* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK*/
,{0,{2,0,toSOPC_String("RM")}}/* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM*/
,{0,{5,0,toSOPC_String("Int64")}}/* i=1001*/
,{0,{6,0,toSOPC_String("Uint32")}}/* i=1002*/
,{0,{6,0,toSOPC_String("Double")}}/* i=1003*/
,{0,{6,0,toSOPC_String("String")}}/* i=1004*/
,{0,{2,0,toSOPC_String("GK")}}/* s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK*/
,{0,{3,0,toSOPC_String("ASK")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK*/
,{0,{4,0,toSOPC_String("XBKK")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK*/
,{0,{6,0,toSOPC_String("XBZCRQ")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ*/
,{0,{6,0,toSOPC_String("XBZ-AK")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK*/
,{0,{9,0,toSOPC_String("XBZCRQ-AK")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK*/
,{0,{11,0,toSOPC_String("SendCommand")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand*/
,{0,{13,0,toSOPC_String("OffBlocking-K")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K*/
,{0,{14,0,toSOPC_String("OffBlocking-CC")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC*/
,{0,{2,0,toSOPC_String("GZ")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ*/
,{0,{2,0,toSOPC_String("SZ")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ*/
,{0,{6,0,toSOPC_String("XBZ-CC")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC*/
,{0,{9,0,toSOPC_String("XBZCRQ-CC")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC*/
,{0,{7,0,toSOPC_String("XBZC-CC")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC*/
,{0,{2,0,toSOPC_String("GK")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK*/
,{0,{3,0,toSOPC_String("ASK")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK*/
,{0,{11,0,toSOPC_String("SendCommand")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand*/
,{0,{13,0,toSOPC_String("OffBlocking-K")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K*/
,{0,{14,0,toSOPC_String("OffBlocking-CC")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC*/
,{0,{2,0,toSOPC_String("SZ")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ*/
,{0,{2,0,toSOPC_String("GZ")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ*/
,{0,{2,0,toSOPC_String("GK")}}/* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK*/
,{0,{3,0,toSOPC_String("NWK")}}/* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK*/
,{0,{3,0,toSOPC_String("RWK")}}/* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK*/
,{0,{2,0,toSOPC_String("LK")}}/* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK*/
,{0,{11,0,toSOPC_String("SendCommand")}}/* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand*/
,{0,{13,0,toSOPC_String("OffBlocking-K")}}/* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K*/
,{0,{14,0,toSOPC_String("OffBlocking-CC")}}/* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC*/
,{0,{3,0,toSOPC_String("NWZ")}}/* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ*/
,{0,{3,0,toSOPC_String("RWZ")}}/* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ*/
,{0,{2,0,toSOPC_String("TK")}}/* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK*/
,{0,{2,0,toSOPC_String("TK")}}/* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK*/
,{0,{2,0,toSOPC_String("TK")}}/* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK*/

};



static SOPC_LocalizedText Description[] = {{{0, 0, NULL}, {0, 0, NULL}}
, {{0,0,toSOPC_String("")},{25,0,toSOPC_String("VariablesFolderDescObj1d2")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{2,0,toSOPC_String("en")},{6,0,toSOPC_String("NoName")}}
, {{2,0,toSOPC_String("en")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("NoName")}}
, {{0,0,toSOPC_String("")},{8,0,toSOPC_String("Int64_1d")}}
, {{0,0,toSOPC_String("")},{9,0,toSOPC_String("UInt32_1d")}}
, {{0,0,toSOPC_String("")},{9,0,toSOPC_String("Double_1d")}}
, {{0,0,toSOPC_String("")},{9,0,toSOPC_String("String_1d")}}
, {{0,0,toSOPC_String("")},{32,0,toSOPC_String("Permissive Signal Status, ~é€bla")}}
, {{0,0,toSOPC_String("")},{23,0,toSOPC_String("Signal Approach Locking")}}
, {{0,0,toSOPC_String("")},{41,0,toSOPC_String("Exit blocking is in effect for the signal")}}
, {{0,0,toSOPC_String("")},{26,0,toSOPC_String("Exit Block Removal Request")}}
, {{0,0,toSOPC_String("")},{40,0,toSOPC_String("Exit blocking is rejected for the signal")}}
, {{0,0,toSOPC_String("")},{45,0,toSOPC_String("Exit block removal is rejected for the signal")}}
, {{0,0,toSOPC_String("")},{14,0,toSOPC_String("Signal request")}}
, {{0,0,toSOPC_String("")},{13,0,toSOPC_String("Signal cancel")}}
, {{0,0,toSOPC_String("")},{30,0,toSOPC_String("Exit Block Application Request")}}
, {{0,0,toSOPC_String("")},{26,0,toSOPC_String("Exit Block Removal Request")}}
, {{0,0,toSOPC_String("")},{32,0,toSOPC_String("Exit Block Removal - Acknowledge")}}
, {{0,0,toSOPC_String("")},{24,0,toSOPC_String("Permissive Signal Status")}}
, {{0,0,toSOPC_String("")},{23,0,toSOPC_String("Signal Approach Locking")}}
, {{0,0,toSOPC_String("")},{13,0,toSOPC_String("Signal cancel")}}
, {{0,0,toSOPC_String("")},{14,0,toSOPC_String("Signal request")}}
, {{0,0,toSOPC_String("")},{24,0,toSOPC_String("Permissive Signal Status")}}
, {{0,0,toSOPC_String("")},{22,0,toSOPC_String("Switch Detected Normal")}}
, {{0,0,toSOPC_String("")},{23,0,toSOPC_String("Switch Detected Reverse")}}
, {{0,0,toSOPC_String("")},{21,0,toSOPC_String("Switch Locally Locked")}}
, {{0,0,toSOPC_String("")},{34,0,toSOPC_String("Switch Calling in Normal Direction")}}
, {{0,0,toSOPC_String("")},{35,0,toSOPC_String("Switch Calling in Reverse Direction")}}
, {{0,0,toSOPC_String("")},{26,0,toSOPC_String("Secondary Detection Status")}}
, {{0,0,toSOPC_String("")},{26,0,toSOPC_String("Secondary Detection Status")}}
, {{0,0,toSOPC_String("")},{26,0,toSOPC_String("Secondary Detection Status")}}

};
static int Description_begin[] = {0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 35, 35, 35, 36, 37, 38, 39, 40, 41, 42, 42, 42, 42, 43, 44, 45, 46, 47, 48, 48, 48, 48, 49, 50, 51, 52};
static int Description_end[] = {-1, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 34, 34, 34, 35, 36, 37, 38, 39, 40, 41, 41, 41, 41, 42, 43, 44, 45, 46, 47, 47, 47, 47, 48, 49, 50, 51, 52};
static SOPC_LocalizedText DisplayName[] = {{{0, 0, NULL}, {0, 0, NULL}}
, {{0,0,toSOPC_String("")},{17,0,toSOPC_String("VariablesFolderDn")}}
, {{0,0,toSOPC_String("")},{5,0,toSOPC_String("15361")}}
, {{0,0,toSOPC_String("")},{7,0,toSOPC_String("SIGNALs")}}
, {{0,0,toSOPC_String("")},{7,0,toSOPC_String("SWITCHs")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("TRACKs")}}
, {{0,0,toSOPC_String("")},{14,0,toSOPC_String("BALA_RDLS_G019")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("RM")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("RC")}}
, {{0,0,toSOPC_String("")},{14,0,toSOPC_String("BALA_RDLS_G025")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("RM")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("RC")}}
, {{0,0,toSOPC_String("")},{14,0,toSOPC_String("BALA_RDLS_G026")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("RM")}}
, {{0,0,toSOPC_String("")},{12,0,toSOPC_String("BALA_RDLS_W1")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("RM")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("RC")}}
, {{0,0,toSOPC_String("")},{15,0,toSOPC_String("BALA_RDLS_026TK")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("RM")}}
, {{0,0,toSOPC_String("")},{14,0,toSOPC_String("BALA_RDLS_OSTK")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("RM")}}
, {{0,0,toSOPC_String("")},{14,0,toSOPC_String("BALA_RDLS_P500")}}
, {{0,0,toSOPC_String("")},{22,0,toSOPC_String("BALA_RDLS_WBK_RDLN_EBK")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("RM")}}
, {{0,0,toSOPC_String("")},{9,0,toSOPC_String("Int64_1dn")}}
, {{0,0,toSOPC_String("")},{10,0,toSOPC_String("UInt32_1dn")}}
, {{0,0,toSOPC_String("")},{10,0,toSOPC_String("Double_1dn")}}
, {{0,0,toSOPC_String("")},{10,0,toSOPC_String("String_1dn")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("GK")}}
, {{0,0,toSOPC_String("")},{3,0,toSOPC_String("ASK")}}
, {{0,0,toSOPC_String("")},{4,0,toSOPC_String("XBKK")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("XBZCRQ")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("XBZ-AK")}}
, {{0,0,toSOPC_String("")},{9,0,toSOPC_String("XBZCRQ-AK")}}
, {{0,0,toSOPC_String("")},{11,0,toSOPC_String("SendCommand")}}
, {{0,0,toSOPC_String("")},{13,0,toSOPC_String("OffBlocking-K")}}
, {{0,0,toSOPC_String("")},{14,0,toSOPC_String("OffBlocking-CC")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("GZ")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("SZ")}}
, {{0,0,toSOPC_String("")},{6,0,toSOPC_String("XBZ-CC")}}
, {{0,0,toSOPC_String("")},{9,0,toSOPC_String("XBZCRQ-CC")}}
, {{0,0,toSOPC_String("")},{7,0,toSOPC_String("XBZC-CC")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("GK")}}
, {{0,0,toSOPC_String("")},{3,0,toSOPC_String("ASK")}}
, {{0,0,toSOPC_String("")},{11,0,toSOPC_String("SendCommand")}}
, {{0,0,toSOPC_String("")},{13,0,toSOPC_String("OffBlocking-K")}}
, {{0,0,toSOPC_String("")},{14,0,toSOPC_String("OffBlocking-CC")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("SZ")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("GZ")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("GK")}}
, {{0,0,toSOPC_String("")},{3,0,toSOPC_String("NWK")}}
, {{0,0,toSOPC_String("")},{3,0,toSOPC_String("RWK")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("LK")}}
, {{0,0,toSOPC_String("")},{11,0,toSOPC_String("SendCommand")}}
, {{0,0,toSOPC_String("")},{13,0,toSOPC_String("OffBlocking-K")}}
, {{0,0,toSOPC_String("")},{14,0,toSOPC_String("OffBlocking-CC")}}
, {{0,0,toSOPC_String("")},{3,0,toSOPC_String("NWZ")}}
, {{0,0,toSOPC_String("")},{3,0,toSOPC_String("RWZ")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("TK")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("TK")}}
, {{0,0,toSOPC_String("")},{2,0,toSOPC_String("TK")}}

};
static int DisplayName_begin[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60};
static int DisplayName_end[] = {-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60};



static int reference_begin[] = {0, 1, 7, 12, 17, 20, 26, 33, 41, 48, 55, 59, 63, 66, 69, 76, 81, 85, 88, 91, 94, 97, 99, 102, 105, 107, 109, 111, 113, 115, 117, 119, 121, 123, 125, 127, 129, 131, 133, 135, 137, 139, 141, 143, 145, 147, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167, 169, 171, 173, 175, 177};
static int reference_end[] = {-1,
0+6 /* i=1000 */,
6+5 /* ns=261;i=15361 */,
11+5 /* ns=261;s=Objects.15361.SIGNALs */,
16+3 /* ns=261;s=Objects.15361.SWITCHs */,
19+6 /* ns=261;s=Objects.15361.TRACKs */,
25+7 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019 */,
32+8 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM */,
40+7 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC */,
47+7 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025 */,
54+4 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM */,
58+4 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC */,
62+3 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026 */,
65+3 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM */,
68+7 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1 */,
75+5 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM */,
80+4 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC */,
84+3 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK */,
87+3 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM */,
90+3 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK */,
93+3 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM */,
96+2 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_P500 */,
98+3 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK */,
101+3 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM */,
104+2 /* i=1001 */,
106+2 /* i=1002 */,
108+2 /* i=1003 */,
110+2 /* i=1004 */,
112+2 /* s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK */,
114+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK */,
116+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK */,
118+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ */,
120+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK */,
122+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK */,
124+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand */,
126+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K */,
128+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC */,
130+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ */,
132+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ */,
134+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC */,
136+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC */,
138+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC */,
140+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK */,
142+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK */,
144+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand */,
146+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K */,
148+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC */,
150+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ */,
152+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ */,
154+2 /* ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK */,
156+2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK */,
158+2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK */,
160+2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK */,
162+2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand */,
164+2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K */,
166+2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC */,
168+2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ */,
170+2 /* ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ */,
172+2 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK */,
174+2 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK */,
176+2 /* ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK */};
static SOPC_NodeId* reference_type[] = {NULL,  &nodeid_2, &nodeid_4, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_2, &nodeid_4, &nodeid_2, &nodeid_2, &nodeid_2, &nodeid_2, &nodeid_4, &nodeid_2, &nodeid_2, &nodeid_2, &nodeid_2, &nodeid_4, &nodeid_2, &nodeid_2, &nodeid_4, &nodeid_2, &nodeid_2, &nodeid_2, &nodeid_2, &nodeid_2, &nodeid_4, &nodeid_2, &nodeid_2, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_2, &nodeid_4, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_2, &nodeid_4, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_2, &nodeid_4, &nodeid_2, &nodeid_2, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_2, &nodeid_4, &nodeid_6, &nodeid_6, &nodeid_2, &nodeid_4, &nodeid_6, &nodeid_6, &nodeid_2, &nodeid_4, &nodeid_2, &nodeid_2, &nodeid_4, &nodeid_6, &nodeid_2, &nodeid_4, &nodeid_2, &nodeid_2, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_2, &nodeid_4, &nodeid_6, &nodeid_6, &nodeid_6, &nodeid_2, &nodeid_4, &nodeid_6, &nodeid_6, &nodeid_2, &nodeid_4, &nodeid_2, &nodeid_2, &nodeid_4, &nodeid_6, &nodeid_2, &nodeid_4, &nodeid_2, &nodeid_2, &nodeid_4, &nodeid_6, &nodeid_2, &nodeid_4, &nodeid_2, &nodeid_4, &nodeid_2, &nodeid_2, &nodeid_4, &nodeid_6, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4, &nodeid_6, &nodeid_4};
static SOPC_ExpandedNodeId* reference_target[] = {NULL, &ex_nodeid_3, &ex_nodeid_5, &ex_nodeid_7, &ex_nodeid_8, &ex_nodeid_9, &ex_nodeid_10, &ex_nodeid_3, &ex_nodeid_5, &ex_nodeid_12, &ex_nodeid_13, &ex_nodeid_14, &ex_nodeid_11, &ex_nodeid_5, &ex_nodeid_15, &ex_nodeid_16, &ex_nodeid_17, &ex_nodeid_11, &ex_nodeid_5, &ex_nodeid_18, &ex_nodeid_11, &ex_nodeid_5, &ex_nodeid_19, &ex_nodeid_20, &ex_nodeid_21, &ex_nodeid_22, &ex_nodeid_12, &ex_nodeid_5, &ex_nodeid_23, &ex_nodeid_24, &ex_nodeid_25, &ex_nodeid_26, &ex_nodeid_27, &ex_nodeid_15, &ex_nodeid_5, &ex_nodeid_28, &ex_nodeid_29, &ex_nodeid_30, &ex_nodeid_31, &ex_nodeid_32, &ex_nodeid_33, &ex_nodeid_15, &ex_nodeid_5, &ex_nodeid_34, &ex_nodeid_35, &ex_nodeid_36, &ex_nodeid_37, &ex_nodeid_38, &ex_nodeid_12, &ex_nodeid_5, &ex_nodeid_39, &ex_nodeid_40, &ex_nodeid_41, &ex_nodeid_42, &ex_nodeid_43, &ex_nodeid_16, &ex_nodeid_5, &ex_nodeid_44, &ex_nodeid_45, &ex_nodeid_16, &ex_nodeid_5, &ex_nodeid_46, &ex_nodeid_47, &ex_nodeid_12, &ex_nodeid_5, &ex_nodeid_48, &ex_nodeid_17, &ex_nodeid_5, &ex_nodeid_49, &ex_nodeid_13, &ex_nodeid_5, &ex_nodeid_50, &ex_nodeid_51, &ex_nodeid_52, &ex_nodeid_53, &ex_nodeid_54, &ex_nodeid_18, &ex_nodeid_5, &ex_nodeid_55, &ex_nodeid_56, &ex_nodeid_57, &ex_nodeid_18, &ex_nodeid_5, &ex_nodeid_58, &ex_nodeid_59, &ex_nodeid_14, &ex_nodeid_5, &ex_nodeid_60, &ex_nodeid_19, &ex_nodeid_5, &ex_nodeid_61, &ex_nodeid_14, &ex_nodeid_5, &ex_nodeid_62, &ex_nodeid_20, &ex_nodeid_5, &ex_nodeid_63, &ex_nodeid_14, &ex_nodeid_5, &ex_nodeid_14, &ex_nodeid_5, &ex_nodeid_64, &ex_nodeid_22, &ex_nodeid_5, &ex_nodeid_65, &ex_nodeid_1, &ex_nodeid_66, &ex_nodeid_1, &ex_nodeid_66, &ex_nodeid_1, &ex_nodeid_66, &ex_nodeid_1, &ex_nodeid_66, &ex_nodeid_23, &ex_nodeid_66, &ex_nodeid_23, &ex_nodeid_66, &ex_nodeid_23, &ex_nodeid_66, &ex_nodeid_23, &ex_nodeid_66, &ex_nodeid_23, &ex_nodeid_66, &ex_nodeid_23, &ex_nodeid_66, &ex_nodeid_15, &ex_nodeid_66, &ex_nodeid_15, &ex_nodeid_66, &ex_nodeid_15, &ex_nodeid_66, &ex_nodeid_24, &ex_nodeid_66, &ex_nodeid_24, &ex_nodeid_66, &ex_nodeid_24, &ex_nodeid_66, &ex_nodeid_24, &ex_nodeid_66, &ex_nodeid_24, &ex_nodeid_66, &ex_nodeid_39, &ex_nodeid_66, &ex_nodeid_39, &ex_nodeid_66, &ex_nodeid_16, &ex_nodeid_66, &ex_nodeid_16, &ex_nodeid_66, &ex_nodeid_16, &ex_nodeid_66, &ex_nodeid_40, &ex_nodeid_66, &ex_nodeid_40, &ex_nodeid_66, &ex_nodeid_48, &ex_nodeid_66, &ex_nodeid_50, &ex_nodeid_66, &ex_nodeid_50, &ex_nodeid_66, &ex_nodeid_50, &ex_nodeid_66, &ex_nodeid_18, &ex_nodeid_66, &ex_nodeid_18, &ex_nodeid_66, &ex_nodeid_18, &ex_nodeid_66, &ex_nodeid_51, &ex_nodeid_66, &ex_nodeid_51, &ex_nodeid_66, &ex_nodeid_60, &ex_nodeid_66, &ex_nodeid_62, &ex_nodeid_66, &ex_nodeid_64, &ex_nodeid_66};
static bool reference_isForward[]={false, false, true, true, true, true, true, false, true, true, true, true, false, true, true, true, true, false, true, true, false, true, true, true, true, true, false, true, true, true, true, true, true, false, true, true, true, true, true, true, true, false, true, true, true, true, true, true, false, true, true, true, true, true, true, false, true, true, true, false, true, true, true, false, true, true, false, true, true, false, true, true, true, true, true, true, false, true, true, true, true, false, true, true, true, false, true, true, false, true, true, false, true, true, false, true, true, false, true, false, true, true, false, true, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true};


static SOPC_NodeId* NodeId[NB+1] = {NULL,
&nodeid_1,
&nodeid_11,
&nodeid_12,
&nodeid_13,
&nodeid_14,
&nodeid_15,
&nodeid_23,
&nodeid_24,
&nodeid_16,
&nodeid_39,
&nodeid_40,
&nodeid_17,
&nodeid_48,
&nodeid_18,
&nodeid_50,
&nodeid_51,
&nodeid_19,
&nodeid_60,
&nodeid_20,
&nodeid_62,
&nodeid_21,
&nodeid_22,
&nodeid_64,
&nodeid_7,
&nodeid_8,
&nodeid_9,
&nodeid_10,
&nodeid_67,
&nodeid_29,
&nodeid_30,
&nodeid_31,
&nodeid_32,
&nodeid_33,
&nodeid_25,
&nodeid_26,
&nodeid_27,
&nodeid_34,
&nodeid_35,
&nodeid_36,
&nodeid_37,
&nodeid_38,
&nodeid_44,
&nodeid_45,
&nodeid_41,
&nodeid_42,
&nodeid_43,
&nodeid_46,
&nodeid_47,
&nodeid_49,
&nodeid_55,
&nodeid_56,
&nodeid_57,
&nodeid_52,
&nodeid_53,
&nodeid_54,
&nodeid_58,
&nodeid_59,
&nodeid_61,
&nodeid_63,
&nodeid_65};



static OpcUa_NodeClass NodeClass[NB+1] = {OpcUa_NodeClass_Unspecified,
    OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Object, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable, OpcUa_NodeClass_Variable
};


static SOPC_ByteString *Value[NB_3 + NB_4 +1] = {NULL};


static SOPC_StatusCode status_code[] = {STATUS_NOK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_NOK, STATUS_NOK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_NOK, STATUS_NOK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_NOK, STATUS_NOK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK, STATUS_OK};


static SOPC_SByte AccessLevel[] = {0, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

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
    .valueArray = (SOPC_Variant*) Value,
    .valueStatusArray = status_code,
    .accessLevelArray = AccessLevel,
};

