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

#include "sopc_address_space.h"

#include <stdio.h>
#include <stdbool.h>

#include "opcua_statuscodes.h"

#include "sopc_builtintypes.h"
#include "sopc_enums.h"
#include "sopc_types.h"
#include "sopc_macros.h"

const bool sopc_embedded_is_const_addspace = true;

SOPC_GCC_DIAGNOSTIC_IGNORE_DISCARD_QUALIFIER
const SOPC_AddressSpace_Node SOPC_Embedded_AddressSpace_Nodes[] = {
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 24},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("BaseDataType")-1, 1, (SOPC_Byte*) "BaseDataType"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("BaseDataType")-1, 1, (SOPC_Byte*) "BaseDataType"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 8,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 26}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 29}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 13}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 19}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 21}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 22}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 26},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("Number")-1, 1, (SOPC_Byte*) "Number"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Number")-1, 1, (SOPC_Byte*) "Number"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 4,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 24}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 27}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 28}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 27},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("Integer")-1, 1, (SOPC_Byte*) "Integer"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Integer")-1, 1, (SOPC_Byte*) "Integer"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 26}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 4}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 28},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("UInteger")-1, 1, (SOPC_Byte*) "UInteger"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("UInteger")-1, 1, (SOPC_Byte*) "UInteger"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 4,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 26}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 5}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 29},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("Enumeration")-1, 1, (SOPC_Byte*) "Enumeration"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Enumeration")-1, 1, (SOPC_Byte*) "Enumeration"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 3,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 24}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 851}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 852}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("Boolean")-1, 1, (SOPC_Byte*) "Boolean"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Boolean")-1, 1, (SOPC_Byte*) "Boolean"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 24}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("Byte")-1, 1, (SOPC_Byte*) "Byte"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Byte")-1, 1, (SOPC_Byte*) "Byte"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 28}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 4},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("Int16")-1, 1, (SOPC_Byte*) "Int16"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Int16")-1, 1, (SOPC_Byte*) "Int16"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 27}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 5},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("UInt16")-1, 1, (SOPC_Byte*) "UInt16"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("UInt16")-1, 1, (SOPC_Byte*) "UInt16"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 28}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("UInt32")-1, 1, (SOPC_Byte*) "UInt32"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("UInt32")-1, 1, (SOPC_Byte*) "UInt32"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 28}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("Double")-1, 1, (SOPC_Byte*) "Double"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Double")-1, 1, (SOPC_Byte*) "Double"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 26}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 290}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("String")-1, 1, (SOPC_Byte*) "String"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("String")-1, 1, (SOPC_Byte*) "String"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 24}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 295}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 13},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("DateTime")-1, 1, (SOPC_Byte*) "DateTime"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("DateTime")-1, 1, (SOPC_Byte*) "DateTime"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 24}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 294}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 19},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("StatusCode")-1, 1, (SOPC_Byte*) "StatusCode"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("StatusCode")-1, 1, (SOPC_Byte*) "StatusCode"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 24}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 21},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("LocalizedText")-1, 1, (SOPC_Byte*) "LocalizedText"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("LocalizedText")-1, 1, (SOPC_Byte*) "LocalizedText"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 24}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 22},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("Structure")-1, 1, (SOPC_Byte*) "Structure"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Structure")-1, 1, (SOPC_Byte*) "Structure"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 4,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 24}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 344}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 338}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 862}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ReferenceType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.reference_type={
            .encodeableType = &OpcUa_ReferenceTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 31},
            .NodeClass = OpcUa_NodeClass_ReferenceType,
            .BrowseName = {0, {sizeof("References")-1, 1, (SOPC_Byte*) "References"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("References")-1, 1, (SOPC_Byte*) "References"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 32}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 33}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ReferenceType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.reference_type={
            .encodeableType = &OpcUa_ReferenceTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 32},
            .NodeClass = OpcUa_NodeClass_ReferenceType,
            .BrowseName = {0, {sizeof("NonHierarchicalReferences")-1, 1, (SOPC_Byte*) "NonHierarchicalReferences"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("NonHierarchicalReferences")-1, 1, (SOPC_Byte*) "NonHierarchicalReferences"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 4,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 31}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 37}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 38}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ReferenceType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.reference_type={
            .encodeableType = &OpcUa_ReferenceTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 33},
            .NodeClass = OpcUa_NodeClass_ReferenceType,
            .BrowseName = {0, {sizeof("HierarchicalReferences")-1, 1, (SOPC_Byte*) "HierarchicalReferences"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("HierarchicalReferences")-1, 1, (SOPC_Byte*) "HierarchicalReferences"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 3,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 31}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 34}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ReferenceType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.reference_type={
            .encodeableType = &OpcUa_ReferenceTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 34},
            .NodeClass = OpcUa_NodeClass_ReferenceType,
            .BrowseName = {0, {sizeof("HasChild")-1, 1, (SOPC_Byte*) "HasChild"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("HasChild")-1, 1, (SOPC_Byte*) "HasChild"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 3,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 33}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 44}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ReferenceType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.reference_type={
            .encodeableType = &OpcUa_ReferenceTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
            .NodeClass = OpcUa_NodeClass_ReferenceType,
            .BrowseName = {0, {sizeof("Organizes")-1, 1, (SOPC_Byte*) "Organizes"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Organizes")-1, 1, (SOPC_Byte*) "Organizes"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 33}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ReferenceType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.reference_type={
            .encodeableType = &OpcUa_ReferenceTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 37},
            .NodeClass = OpcUa_NodeClass_ReferenceType,
            .BrowseName = {0, {sizeof("HasModellingRule")-1, 1, (SOPC_Byte*) "HasModellingRule"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("HasModellingRule")-1, 1, (SOPC_Byte*) "HasModellingRule"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 32}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ReferenceType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.reference_type={
            .encodeableType = &OpcUa_ReferenceTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 38},
            .NodeClass = OpcUa_NodeClass_ReferenceType,
            .BrowseName = {0, {sizeof("HasEncoding")-1, 1, (SOPC_Byte*) "HasEncoding"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("HasEncoding")-1, 1, (SOPC_Byte*) "HasEncoding"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 32}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ReferenceType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.reference_type={
            .encodeableType = &OpcUa_ReferenceTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
            .NodeClass = OpcUa_NodeClass_ReferenceType,
            .BrowseName = {0, {sizeof("HasTypeDefinition")-1, 1, (SOPC_Byte*) "HasTypeDefinition"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("HasTypeDefinition")-1, 1, (SOPC_Byte*) "HasTypeDefinition"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 32}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ReferenceType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.reference_type={
            .encodeableType = &OpcUa_ReferenceTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 44},
            .NodeClass = OpcUa_NodeClass_ReferenceType,
            .BrowseName = {0, {sizeof("Aggregates")-1, 1, (SOPC_Byte*) "Aggregates"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Aggregates")-1, 1, (SOPC_Byte*) "Aggregates"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 3,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 34}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ReferenceType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.reference_type={
            .encodeableType = &OpcUa_ReferenceTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
            .NodeClass = OpcUa_NodeClass_ReferenceType,
            .BrowseName = {0, {sizeof("HasSubtype")-1, 1, (SOPC_Byte*) "HasSubtype"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("HasSubtype")-1, 1, (SOPC_Byte*) "HasSubtype"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 34}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ReferenceType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.reference_type={
            .encodeableType = &OpcUa_ReferenceTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
            .NodeClass = OpcUa_NodeClass_ReferenceType,
            .BrowseName = {0, {sizeof("HasProperty")-1, 1, (SOPC_Byte*) "HasProperty"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("HasProperty")-1, 1, (SOPC_Byte*) "HasProperty"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 44}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ReferenceType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.reference_type={
            .encodeableType = &OpcUa_ReferenceTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
            .NodeClass = OpcUa_NodeClass_ReferenceType,
            .BrowseName = {0, {sizeof("HasComponent")-1, 1, (SOPC_Byte*) "HasComponent"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("HasComponent")-1, 1, (SOPC_Byte*) "HasComponent"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 44}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ObjectType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object_type={
            .encodeableType = &OpcUa_ObjectTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 58},
            .NodeClass = OpcUa_NodeClass_ObjectType,
            .BrowseName = {0, {sizeof("BaseObjectType")-1, 1, (SOPC_Byte*) "BaseObjectType"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("BaseObjectType")-1, 1, (SOPC_Byte*) "BaseObjectType"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 3,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 76}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 77}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ObjectType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object_type={
            .encodeableType = &OpcUa_ObjectTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61},
            .NodeClass = OpcUa_NodeClass_ObjectType,
            .BrowseName = {0, {sizeof("FolderType")-1, 1, (SOPC_Byte*) "FolderType"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("FolderType")-1, 1, (SOPC_Byte*) "FolderType"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 58}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_VariableType,
        0x00,
        {0, 0},
        {.variable_type={
            .encodeableType = &OpcUa_VariableTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 62},
            .NodeClass = OpcUa_NodeClass_VariableType,
            .BrowseName = {0, {sizeof("BaseVariableType")-1, 1, (SOPC_Byte*) "BaseVariableType"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("BaseVariableType")-1, 1, (SOPC_Byte*) "BaseVariableType"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = {true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 0},
            .ValueRank = (-2),
        }}
    },
    {
        OpcUa_NodeClass_VariableType,
        0x00,
        {0, 0},
        {.variable_type={
            .encodeableType = &OpcUa_VariableTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63},
            .NodeClass = OpcUa_NodeClass_VariableType,
            .BrowseName = {0, {sizeof("BaseDataVariableType")-1, 1, (SOPC_Byte*) "BaseDataVariableType"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("BaseDataVariableType")-1, 1, (SOPC_Byte*) "BaseDataVariableType"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 62}, {0, 0, NULL}, 0},
                }
            },
            .Value = {true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 0},
            .ValueRank = (-2),
        }}
    },
    {
        OpcUa_NodeClass_VariableType,
        0x00,
        {0, 0},
        {.variable_type={
            .encodeableType = &OpcUa_VariableTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68},
            .NodeClass = OpcUa_NodeClass_VariableType,
            .BrowseName = {0, {sizeof("PropertyType")-1, 1, (SOPC_Byte*) "PropertyType"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("PropertyType")-1, 1, (SOPC_Byte*) "PropertyType"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 62}, {0, 0, NULL}, 0},
                }
            },
            .Value = {true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 0},
            .ValueRank = (-2),
        }}
    },
    {
        OpcUa_NodeClass_ObjectType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object_type={
            .encodeableType = &OpcUa_ObjectTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 76},
            .NodeClass = OpcUa_NodeClass_ObjectType,
            .BrowseName = {0, {sizeof("DataTypeEncodingType")-1, 1, (SOPC_Byte*) "DataTypeEncodingType"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("DataTypeEncodingType")-1, 1, (SOPC_Byte*) "DataTypeEncodingType"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 58}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_ObjectType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object_type={
            .encodeableType = &OpcUa_ObjectTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 77},
            .NodeClass = OpcUa_NodeClass_ObjectType,
            .BrowseName = {0, {sizeof("ModellingRuleType")-1, 1, (SOPC_Byte*) "ModellingRuleType"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ModellingRuleType")-1, 1, (SOPC_Byte*) "ModellingRuleType"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 58}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 111}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 111},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("NamingRule")-1, 1, (SOPC_Byte*) "NamingRule"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("NamingRule")-1, 1, (SOPC_Byte*) "NamingRule"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 37},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 78}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 0}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 120},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 78},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Mandatory")-1, 1, (SOPC_Byte*) "Mandatory"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Mandatory")-1, 1, (SOPC_Byte*) "Mandatory"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 77}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 112}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 112},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("NamingRule")-1, 1, (SOPC_Byte*) "NamingRule"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("NamingRule")-1, 1, (SOPC_Byte*) "NamingRule"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 1}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 120},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 80},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Optional")-1, 1, (SOPC_Byte*) "Optional"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Optional")-1, 1, (SOPC_Byte*) "Optional"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 77}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 113}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 113},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("NamingRule")-1, 1, (SOPC_Byte*) "NamingRule"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("NamingRule")-1, 1, (SOPC_Byte*) "NamingRule"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 2}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 120},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11508},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("OptionalPlaceholder")-1, 1, (SOPC_Byte*) "OptionalPlaceholder"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("OptionalPlaceholder")-1, 1, (SOPC_Byte*) "OptionalPlaceholder"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 77}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11509}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11509},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("NamingRule")-1, 1, (SOPC_Byte*) "NamingRule"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("NamingRule")-1, 1, (SOPC_Byte*) "NamingRule"}},
            .Description = {{0, 0, NULL}, {sizeof("Specified the significances of the BrowseName when a type is instantiated.")-1, 1, (SOPC_Byte*) "Specified the significances of the BrowseName when a type is instantiated."}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 3}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 120},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11510},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("MandatoryPlaceholder")-1, 1, (SOPC_Byte*) "MandatoryPlaceholder"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MandatoryPlaceholder")-1, 1, (SOPC_Byte*) "MandatoryPlaceholder"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 77}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11511}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11511},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("NamingRule")-1, 1, (SOPC_Byte*) "NamingRule"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("NamingRule")-1, 1, (SOPC_Byte*) "NamingRule"}},
            .Description = {{0, 0, NULL}, {sizeof("Specified the significances of the BrowseName when a type is instantiated.")-1, 1, (SOPC_Byte*) "Specified the significances of the BrowseName when a type is instantiated."}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 4}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 120},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 84},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Root")-1, 1, (SOPC_Byte*) "Root"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Root")-1, 1, (SOPC_Byte*) "Root"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 4,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 85}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 86}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 87}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 85},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Objects")-1, 1, (SOPC_Byte*) "Objects"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Objects")-1, 1, (SOPC_Byte*) "Objects"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 4,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2253}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubVars")-1, 1, (SOPC_Byte*) "PubVars"}}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubVars")-1, 1, (SOPC_Byte*) "SubVars"}}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 86},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Types")-1, 1, (SOPC_Byte*) "Types"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Types")-1, 1, (SOPC_Byte*) "Types"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 7,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 88}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 89}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 90}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 91}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3048}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 17708}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 87},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Views")-1, 1, (SOPC_Byte*) "Views"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Views")-1, 1, (SOPC_Byte*) "Views"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 88},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("ObjectTypes")-1, 1, (SOPC_Byte*) "ObjectTypes"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ObjectTypes")-1, 1, (SOPC_Byte*) "ObjectTypes"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 58}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 89},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("VariableTypes")-1, 1, (SOPC_Byte*) "VariableTypes"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("VariableTypes")-1, 1, (SOPC_Byte*) "VariableTypes"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 62}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 90},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("DataTypes")-1, 1, (SOPC_Byte*) "DataTypes"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("DataTypes")-1, 1, (SOPC_Byte*) "DataTypes"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 24}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 91},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("ReferenceTypes")-1, 1, (SOPC_Byte*) "ReferenceTypes"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ReferenceTypes")-1, 1, (SOPC_Byte*) "ReferenceTypes"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 31}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3048},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("EventTypes")-1, 1, (SOPC_Byte*) "EventTypes"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("EventTypes")-1, 1, (SOPC_Byte*) "EventTypes"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2253},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Server")-1, 1, (SOPC_Byte*) "Server"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Server")-1, 1, (SOPC_Byte*) "Server"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 9,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2254}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2255}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2256}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2267}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2994}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2268}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2274}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2295}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2296}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2254},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("ServerArray")-1, 1, (SOPC_Byte*) "ServerArray"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ServerArray")-1, 1, (SOPC_Byte*) "ServerArray"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 5}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
            .ValueRank = (1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2255},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("NamespaceArray")-1, 1, (SOPC_Byte*) "NamespaceArray"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("NamespaceArray")-1, 1, (SOPC_Byte*) "NamespaceArray"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 6}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
            .ValueRank = (1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2256},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("ServerStatus")-1, 1, (SOPC_Byte*) "ServerStatus"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ServerStatus")-1, 1, (SOPC_Byte*) "ServerStatus"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 6,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2257}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2258}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2259}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2260}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2992}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2993}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 7}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 862},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2257},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("StartTime")-1, 1, (SOPC_Byte*) "StartTime"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("StartTime")-1, 1, (SOPC_Byte*) "StartTime"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 8}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 294},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2258},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("CurrentTime")-1, 1, (SOPC_Byte*) "CurrentTime"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("CurrentTime")-1, 1, (SOPC_Byte*) "CurrentTime"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 9}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 294},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2259},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("State")-1, 1, (SOPC_Byte*) "State"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("State")-1, 1, (SOPC_Byte*) "State"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 10}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 852},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2260},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("BuildInfo")-1, 1, (SOPC_Byte*) "BuildInfo"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("BuildInfo")-1, 1, (SOPC_Byte*) "BuildInfo"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 6,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2262}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2263}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2261}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2264}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2265}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2266}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 11}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 338},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2262},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("ProductUri")-1, 1, (SOPC_Byte*) "ProductUri"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ProductUri")-1, 1, (SOPC_Byte*) "ProductUri"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 12}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2263},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("ManufacturerName")-1, 1, (SOPC_Byte*) "ManufacturerName"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ManufacturerName")-1, 1, (SOPC_Byte*) "ManufacturerName"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 13}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2261},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("ProductName")-1, 1, (SOPC_Byte*) "ProductName"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ProductName")-1, 1, (SOPC_Byte*) "ProductName"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 14}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2264},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("SoftwareVersion")-1, 1, (SOPC_Byte*) "SoftwareVersion"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("SoftwareVersion")-1, 1, (SOPC_Byte*) "SoftwareVersion"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 15}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2265},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("BuildNumber")-1, 1, (SOPC_Byte*) "BuildNumber"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("BuildNumber")-1, 1, (SOPC_Byte*) "BuildNumber"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 16}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2266},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("BuildDate")-1, 1, (SOPC_Byte*) "BuildDate"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("BuildDate")-1, 1, (SOPC_Byte*) "BuildDate"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 17}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 294},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2992},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("SecondsTillShutdown")-1, 1, (SOPC_Byte*) "SecondsTillShutdown"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("SecondsTillShutdown")-1, 1, (SOPC_Byte*) "SecondsTillShutdown"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 18}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2993},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("ShutdownReason")-1, 1, (SOPC_Byte*) "ShutdownReason"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ShutdownReason")-1, 1, (SOPC_Byte*) "ShutdownReason"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 19}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 21},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2267},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("ServiceLevel")-1, 1, (SOPC_Byte*) "ServiceLevel"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ServiceLevel")-1, 1, (SOPC_Byte*) "ServiceLevel"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 20}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2994},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("Auditing")-1, 1, (SOPC_Byte*) "Auditing"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Auditing")-1, 1, (SOPC_Byte*) "Auditing"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 21}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2268},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("ServerCapabilities")-1, 1, (SOPC_Byte*) "ServerCapabilities"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ServerCapabilities")-1, 1, (SOPC_Byte*) "ServerCapabilities"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 13,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2269}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2271}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2272}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2735}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2736}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2737}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3704}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11702}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11703}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12911}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11704}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2996}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2997}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2269},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("ServerProfileArray")-1, 1, (SOPC_Byte*) "ServerProfileArray"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ServerProfileArray")-1, 1, (SOPC_Byte*) "ServerProfileArray"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 22}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
            .ValueRank = (1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2271},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("LocaleIdArray")-1, 1, (SOPC_Byte*) "LocaleIdArray"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("LocaleIdArray")-1, 1, (SOPC_Byte*) "LocaleIdArray"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 23}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 295},
            .ValueRank = (1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2272},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MinSupportedSampleRate")-1, 1, (SOPC_Byte*) "MinSupportedSampleRate"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MinSupportedSampleRate")-1, 1, (SOPC_Byte*) "MinSupportedSampleRate"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 24}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 290},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2735},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MaxBrowseContinuationPoints")-1, 1, (SOPC_Byte*) "MaxBrowseContinuationPoints"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MaxBrowseContinuationPoints")-1, 1, (SOPC_Byte*) "MaxBrowseContinuationPoints"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 25}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 5},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2736},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MaxQueryContinuationPoints")-1, 1, (SOPC_Byte*) "MaxQueryContinuationPoints"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MaxQueryContinuationPoints")-1, 1, (SOPC_Byte*) "MaxQueryContinuationPoints"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 26}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 5},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2737},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MaxHistoryContinuationPoints")-1, 1, (SOPC_Byte*) "MaxHistoryContinuationPoints"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MaxHistoryContinuationPoints")-1, 1, (SOPC_Byte*) "MaxHistoryContinuationPoints"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 27}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 5},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3704},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("SoftwareCertificates")-1, 1, (SOPC_Byte*) "SoftwareCertificates"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("SoftwareCertificates")-1, 1, (SOPC_Byte*) "SoftwareCertificates"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 28}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 344},
            .ValueRank = (1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11702},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MaxArrayLength")-1, 1, (SOPC_Byte*) "MaxArrayLength"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MaxArrayLength")-1, 1, (SOPC_Byte*) "MaxArrayLength"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 29}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11703},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MaxStringLength")-1, 1, (SOPC_Byte*) "MaxStringLength"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MaxStringLength")-1, 1, (SOPC_Byte*) "MaxStringLength"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 30}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12911},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MaxByteStringLength")-1, 1, (SOPC_Byte*) "MaxByteStringLength"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MaxByteStringLength")-1, 1, (SOPC_Byte*) "MaxByteStringLength"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 31}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11704},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("OperationLimits")-1, 1, (SOPC_Byte*) "OperationLimits"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("OperationLimits")-1, 1, (SOPC_Byte*) "OperationLimits"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 7,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11705}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11707}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11710}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11711}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11712}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11713}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11714}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11705},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MaxNodesPerRead")-1, 1, (SOPC_Byte*) "MaxNodesPerRead"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MaxNodesPerRead")-1, 1, (SOPC_Byte*) "MaxNodesPerRead"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 32}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11707},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MaxNodesPerWrite")-1, 1, (SOPC_Byte*) "MaxNodesPerWrite"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MaxNodesPerWrite")-1, 1, (SOPC_Byte*) "MaxNodesPerWrite"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 33}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11710},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MaxNodesPerBrowse")-1, 1, (SOPC_Byte*) "MaxNodesPerBrowse"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MaxNodesPerBrowse")-1, 1, (SOPC_Byte*) "MaxNodesPerBrowse"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 34}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11711},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MaxNodesPerRegisterNodes")-1, 1, (SOPC_Byte*) "MaxNodesPerRegisterNodes"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MaxNodesPerRegisterNodes")-1, 1, (SOPC_Byte*) "MaxNodesPerRegisterNodes"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 35}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11712},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MaxNodesPerTranslateBrowsePathsToNodeIds")-1, 1, (SOPC_Byte*) "MaxNodesPerTranslateBrowsePathsToNodeIds"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MaxNodesPerTranslateBrowsePathsToNodeIds")-1, 1, (SOPC_Byte*) "MaxNodesPerTranslateBrowsePathsToNodeIds"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 36}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11713},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MaxNodesPerNodeManagement")-1, 1, (SOPC_Byte*) "MaxNodesPerNodeManagement"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MaxNodesPerNodeManagement")-1, 1, (SOPC_Byte*) "MaxNodesPerNodeManagement"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 37}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11714},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("MaxMonitoredItemsPerCall")-1, 1, (SOPC_Byte*) "MaxMonitoredItemsPerCall"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("MaxMonitoredItemsPerCall")-1, 1, (SOPC_Byte*) "MaxMonitoredItemsPerCall"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 38}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2996},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("ModellingRules")-1, 1, (SOPC_Byte*) "ModellingRules"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ModellingRules")-1, 1, (SOPC_Byte*) "ModellingRules"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 5,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 78}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 80}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11508}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 35},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11510}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2997},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("AggregateFunctions")-1, 1, (SOPC_Byte*) "AggregateFunctions"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("AggregateFunctions")-1, 1, (SOPC_Byte*) "AggregateFunctions"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2274},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("ServerDiagnostics")-1, 1, (SOPC_Byte*) "ServerDiagnostics"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ServerDiagnostics")-1, 1, (SOPC_Byte*) "ServerDiagnostics"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2294}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2294},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("EnabledFlag")-1, 1, (SOPC_Byte*) "EnabledFlag"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("EnabledFlag")-1, 1, (SOPC_Byte*) "EnabledFlag"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 39}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2295},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("VendorServerInfo")-1, 1, (SOPC_Byte*) "VendorServerInfo"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("VendorServerInfo")-1, 1, (SOPC_Byte*) "VendorServerInfo"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 0,
            .References = NULL,
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 2296},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("ServerRedundancy")-1, 1, (SOPC_Byte*) "ServerRedundancy"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ServerRedundancy")-1, 1, (SOPC_Byte*) "ServerRedundancy"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3709}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3709},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("RedundancySupport")-1, 1, (SOPC_Byte*) "RedundancySupport"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("RedundancySupport")-1, 1, (SOPC_Byte*) "RedundancySupport"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 40}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 851},
            .ValueRank = (-1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 17708},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("InterfaceTypes")-1, 1, (SOPC_Byte*) "InterfaceTypes"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("InterfaceTypes")-1, 1, (SOPC_Byte*) "InterfaceTypes"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 290},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("Duration")-1, 1, (SOPC_Byte*) "Duration"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Duration")-1, 1, (SOPC_Byte*) "Duration"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 11}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 294},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("UtcTime")-1, 1, (SOPC_Byte*) "UtcTime"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("UtcTime")-1, 1, (SOPC_Byte*) "UtcTime"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 13}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 295},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("LocaleId")-1, 1, (SOPC_Byte*) "LocaleId"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("LocaleId")-1, 1, (SOPC_Byte*) "LocaleId"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 344},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("SignedSoftwareCertificate")-1, 1, (SOPC_Byte*) "SignedSoftwareCertificate"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("SignedSoftwareCertificate")-1, 1, (SOPC_Byte*) "SignedSoftwareCertificate"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 4,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 22}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 38},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 346}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 38},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 345}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 38},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 15136}, {0, 0, NULL}, 0},
                }
            },
            .DataTypeDefinition = {{{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 122}, {0, 0, NULL}, 0},SOPC_ExtObjBodyEncoding_Object,.Body.Object = {
            (OpcUa_StructureDefinition[])
            {{&OpcUa_StructureDefinition_EncodeableType,
             {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 346},
             {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 22},
             OpcUa_StructureType_Structure,
             2,
             (OpcUa_StructureField[]){
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("CertificateData")-1, 1, (SOPC_Byte*) "CertificateData"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 15},
              -1,
              0,
              NULL,
              0,
              false}
            ,
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("Signature")-1, 1, (SOPC_Byte*) "Signature"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 15},
              -1,
              0,
              NULL,
              0,
              false}
            }}}
            , &OpcUa_StructureDefinition_EncodeableType}},
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 338},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("BuildInfo")-1, 1, (SOPC_Byte*) "BuildInfo"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("BuildInfo")-1, 1, (SOPC_Byte*) "BuildInfo"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 4,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 22}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 38},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 340}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 38},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 339}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 38},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 15361}, {0, 0, NULL}, 0},
                }
            },
            .DataTypeDefinition = {{{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 122}, {0, 0, NULL}, 0},SOPC_ExtObjBodyEncoding_Object,.Body.Object = {
            (OpcUa_StructureDefinition[])
            {{&OpcUa_StructureDefinition_EncodeableType,
             {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 340},
             {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 22},
             OpcUa_StructureType_Structure,
             6,
             (OpcUa_StructureField[]){
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("ProductUri")-1, 1, (SOPC_Byte*) "ProductUri"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
              -1,
              0,
              NULL,
              0,
              false}
            ,
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("ManufacturerName")-1, 1, (SOPC_Byte*) "ManufacturerName"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
              -1,
              0,
              NULL,
              0,
              false}
            ,
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("ProductName")-1, 1, (SOPC_Byte*) "ProductName"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
              -1,
              0,
              NULL,
              0,
              false}
            ,
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("SoftwareVersion")-1, 1, (SOPC_Byte*) "SoftwareVersion"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
              -1,
              0,
              NULL,
              0,
              false}
            ,
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("BuildNumber")-1, 1, (SOPC_Byte*) "BuildNumber"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
              -1,
              0,
              NULL,
              0,
              false}
            ,
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("BuildDate")-1, 1, (SOPC_Byte*) "BuildDate"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 294},
              -1,
              0,
              NULL,
              0,
              false}
            }}}
            , &OpcUa_StructureDefinition_EncodeableType}},
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 851},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("RedundancySupport")-1, 1, (SOPC_Byte*) "RedundancySupport"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("RedundancySupport")-1, 1, (SOPC_Byte*) "RedundancySupport"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7611}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 29}, {0, 0, NULL}, 0},
                }
            },
            .DataTypeDefinition = {{{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 123}, {0, 0, NULL}, 0},SOPC_ExtObjBodyEncoding_Object,.Body.Object = {
            (OpcUa_EnumDefinition[])
            {{&OpcUa_EnumDefinition_EncodeableType,
             6,
             (OpcUa_EnumField[]){
            {&OpcUa_EnumField_EncodeableType,
              0,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("None")-1, 1, (SOPC_Byte*) "None"}}
            ,
            {&OpcUa_EnumField_EncodeableType,
              1,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("Cold")-1, 1, (SOPC_Byte*) "Cold"}}
            ,
            {&OpcUa_EnumField_EncodeableType,
              2,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("Warm")-1, 1, (SOPC_Byte*) "Warm"}}
            ,
            {&OpcUa_EnumField_EncodeableType,
              3,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("Hot")-1, 1, (SOPC_Byte*) "Hot"}}
            ,
            {&OpcUa_EnumField_EncodeableType,
              4,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("Transparent")-1, 1, (SOPC_Byte*) "Transparent"}}
            ,
            {&OpcUa_EnumField_EncodeableType,
              5,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("HotAndMirrored")-1, 1, (SOPC_Byte*) "HotAndMirrored"}}
            }}}
            , &OpcUa_EnumDefinition_EncodeableType}},
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7611},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("EnumStrings")-1, 1, (SOPC_Byte*) "EnumStrings"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("EnumStrings")-1, 1, (SOPC_Byte*) "EnumStrings"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 41}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 21},
            .ValueRank = (1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 852},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("ServerState")-1, 1, (SOPC_Byte*) "ServerState"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ServerState")-1, 1, (SOPC_Byte*) "ServerState"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 2,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 46},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7612}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 29}, {0, 0, NULL}, 0},
                }
            },
            .DataTypeDefinition = {{{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 123}, {0, 0, NULL}, 0},SOPC_ExtObjBodyEncoding_Object,.Body.Object = {
            (OpcUa_EnumDefinition[])
            {{&OpcUa_EnumDefinition_EncodeableType,
             8,
             (OpcUa_EnumField[]){
            {&OpcUa_EnumField_EncodeableType,
              0,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("Running")-1, 1, (SOPC_Byte*) "Running"}}
            ,
            {&OpcUa_EnumField_EncodeableType,
              1,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("Failed")-1, 1, (SOPC_Byte*) "Failed"}}
            ,
            {&OpcUa_EnumField_EncodeableType,
              2,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("NoConfiguration")-1, 1, (SOPC_Byte*) "NoConfiguration"}}
            ,
            {&OpcUa_EnumField_EncodeableType,
              3,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("Suspended")-1, 1, (SOPC_Byte*) "Suspended"}}
            ,
            {&OpcUa_EnumField_EncodeableType,
              4,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("Shutdown")-1, 1, (SOPC_Byte*) "Shutdown"}}
            ,
            {&OpcUa_EnumField_EncodeableType,
              5,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("Test")-1, 1, (SOPC_Byte*) "Test"}}
            ,
            {&OpcUa_EnumField_EncodeableType,
              6,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("CommunicationFault")-1, 1, (SOPC_Byte*) "CommunicationFault"}}
            ,
            {&OpcUa_EnumField_EncodeableType,
              7,
              {{0, 0, NULL}, {0, 0, NULL}},
              {{0, 0, NULL}, {0, 0, NULL}},
              {sizeof("Unknown")-1, 1, (SOPC_Byte*) "Unknown"}}
            }}}
            , &OpcUa_EnumDefinition_EncodeableType}},
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7612},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {0, {sizeof("EnumStrings")-1, 1, (SOPC_Byte*) "EnumStrings"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("EnumStrings")-1, 1, (SOPC_Byte*) "EnumStrings"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 68}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 42}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 21},
            .ValueRank = (1),
            .AccessLevel = 1,
        }}
    },
    {
        OpcUa_NodeClass_DataType,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.data_type={
            .encodeableType = &OpcUa_DataTypeNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 862},
            .NodeClass = OpcUa_NodeClass_DataType,
            .BrowseName = {0, {sizeof("ServerStatusDataType")-1, 1, (SOPC_Byte*) "ServerStatusDataType"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("ServerStatusDataType")-1, 1, (SOPC_Byte*) "ServerStatusDataType"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 4,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 45},
                    true,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 22}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 38},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 864}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 38},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 863}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 38},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 15367}, {0, 0, NULL}, 0},
                }
            },
            .DataTypeDefinition = {{{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 122}, {0, 0, NULL}, 0},SOPC_ExtObjBodyEncoding_Object,.Body.Object = {
            (OpcUa_StructureDefinition[])
            {{&OpcUa_StructureDefinition_EncodeableType,
             {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 864},
             {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 22},
             OpcUa_StructureType_Structure,
             6,
             (OpcUa_StructureField[]){
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("StartTime")-1, 1, (SOPC_Byte*) "StartTime"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 294},
              -1,
              0,
              NULL,
              0,
              false}
            ,
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("CurrentTime")-1, 1, (SOPC_Byte*) "CurrentTime"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 294},
              -1,
              0,
              NULL,
              0,
              false}
            ,
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("State")-1, 1, (SOPC_Byte*) "State"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 852},
              -1,
              0,
              NULL,
              0,
              false}
            ,
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("BuildInfo")-1, 1, (SOPC_Byte*) "BuildInfo"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 338},
              -1,
              0,
              NULL,
              0,
              false}
            ,
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("SecondsTillShutdown")-1, 1, (SOPC_Byte*) "SecondsTillShutdown"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
              -1,
              0,
              NULL,
              0,
              false}
            ,
            {&OpcUa_StructureField_EncodeableType,
              {sizeof("ShutdownReason")-1, 1, (SOPC_Byte*) "ShutdownReason"},
              {{0, 0, NULL}, {0, 0, NULL}},
              {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 21},
              -1,
              0,
              NULL,
              0,
              false}
            }}}
            , &OpcUa_StructureDefinition_EncodeableType}},
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 346},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Default Binary")-1, 1, (SOPC_Byte*) "Default Binary"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Default Binary")-1, 1, (SOPC_Byte*) "Default Binary"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 76}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 340},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Default Binary")-1, 1, (SOPC_Byte*) "Default Binary"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Default Binary")-1, 1, (SOPC_Byte*) "Default Binary"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 76}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 864},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Default Binary")-1, 1, (SOPC_Byte*) "Default Binary"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Default Binary")-1, 1, (SOPC_Byte*) "Default Binary"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 76}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 345},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Default XML")-1, 1, (SOPC_Byte*) "Default XML"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Default XML")-1, 1, (SOPC_Byte*) "Default XML"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 76}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 339},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Default XML")-1, 1, (SOPC_Byte*) "Default XML"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Default XML")-1, 1, (SOPC_Byte*) "Default XML"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 76}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 863},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Default XML")-1, 1, (SOPC_Byte*) "Default XML"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Default XML")-1, 1, (SOPC_Byte*) "Default XML"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 76}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 15136},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Default JSON")-1, 1, (SOPC_Byte*) "Default JSON"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Default JSON")-1, 1, (SOPC_Byte*) "Default JSON"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 76}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 15361},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Default JSON")-1, 1, (SOPC_Byte*) "Default JSON"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Default JSON")-1, 1, (SOPC_Byte*) "Default JSON"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 76}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 15367},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {0, {sizeof("Default JSON")-1, 1, (SOPC_Byte*) "Default JSON"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("Default JSON")-1, 1, (SOPC_Byte*) "Default JSON"}},
            .Description = {{0, 0, NULL}, {0, 0, NULL}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 76}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubBool")-1, 1, (SOPC_Byte*) "PubBool"}},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {1, {sizeof("varBool")-1, 1, (SOPC_Byte*) "varBool"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("pubBool")-1, 1, (SOPC_Byte*) "pubBool"}},
            .Description = {{0, 0, NULL}, {sizeof("pubBoolDesc")-1, 1, (SOPC_Byte*) "pubBoolDesc"}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 43}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1},
            .ValueRank = (-1),
            .AccessLevel = 3,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubByte")-1, 1, (SOPC_Byte*) "PubByte"}},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {1, {sizeof("pubByte")-1, 1, (SOPC_Byte*) "pubByte"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("pubByte")-1, 1, (SOPC_Byte*) "pubByte"}},
            .Description = {{0, 0, NULL}, {sizeof("pubByteDesc")-1, 1, (SOPC_Byte*) "pubByteDesc"}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 44}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3},
            .ValueRank = (-1),
            .AccessLevel = 3,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubInt16")-1, 1, (SOPC_Byte*) "PubInt16"}},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {1, {sizeof("varInt16")-1, 1, (SOPC_Byte*) "varInt16"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("pubInt16")-1, 1, (SOPC_Byte*) "pubInt16"}},
            .Description = {{0, 0, NULL}, {sizeof("pubIntDesc")-1, 1, (SOPC_Byte*) "pubIntDesc"}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 45}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 4},
            .ValueRank = (-1),
            .AccessLevel = 3,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubStatusCode")-1, 1, (SOPC_Byte*) "PubStatusCode"}},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {1, {sizeof("pubStatusCode")-1, 1, (SOPC_Byte*) "pubStatusCode"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("PubStatusCode")-1, 1, (SOPC_Byte*) "PubStatusCode"}},
            .Description = {{0, 0, NULL}, {sizeof("PubStatusCodeDesc")-1, 1, (SOPC_Byte*) "PubStatusCodeDesc"}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 46}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 19},
            .ValueRank = (-1),
            .AccessLevel = 3,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubString")-1, 1, (SOPC_Byte*) "PubString"}},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {1, {sizeof("pubString")-1, 1, (SOPC_Byte*) "pubString"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("pubString")-1, 1, (SOPC_Byte*) "pubString"}},
            .Description = {{0, 0, NULL}, {sizeof("pubStringDesc")-1, 1, (SOPC_Byte*) "pubStringDesc"}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 47}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
            .ValueRank = (-1),
            .AccessLevel = 3,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubUInt32")-1, 1, (SOPC_Byte*) "PubUInt32"}},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {1, {sizeof("pubUInt32")-1, 1, (SOPC_Byte*) "pubUInt32"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("pubUInt32")-1, 1, (SOPC_Byte*) "pubUInt32"}},
            .Description = {{0, 0, NULL}, {sizeof("pubUInt32Desc")-1, 1, (SOPC_Byte*) "pubUInt32Desc"}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 48}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .ValueRank = (-1),
            .AccessLevel = 3,
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubVars")-1, 1, (SOPC_Byte*) "PubVars"}},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {1, {sizeof("PublisherVars")-1, 1, (SOPC_Byte*) "PublisherVars"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("PublisherVars")-1, 1, (SOPC_Byte*) "PublisherVars"}},
            .Description = {{0, 0, NULL}, {sizeof("PublisherVarsDesc")-1, 1, (SOPC_Byte*) "PublisherVarsDesc"}},
            .NoOfReferences = 7,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubString")-1, 1, (SOPC_Byte*) "PubString"}}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubByte")-1, 1, (SOPC_Byte*) "PubByte"}}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubUInt32")-1, 1, (SOPC_Byte*) "PubUInt32"}}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubInt16")-1, 1, (SOPC_Byte*) "PubInt16"}}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubBool")-1, 1, (SOPC_Byte*) "PubBool"}}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("PubStatusCode")-1, 1, (SOPC_Byte*) "PubStatusCode"}}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubBool")-1, 1, (SOPC_Byte*) "SubBool"}},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {1, {sizeof("subBool")-1, 1, (SOPC_Byte*) "subBool"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("subBool")-1, 1, (SOPC_Byte*) "subBool"}},
            .Description = {{0, 0, NULL}, {sizeof("subBoolDesc")-1, 1, (SOPC_Byte*) "subBoolDesc"}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 49}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 1},
            .ValueRank = (-1),
            .AccessLevel = 3,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubByte")-1, 1, (SOPC_Byte*) "SubByte"}},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {1, {sizeof("subByte")-1, 1, (SOPC_Byte*) "subByte"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("subByte")-1, 1, (SOPC_Byte*) "subByte"}},
            .Description = {{0, 0, NULL}, {sizeof("subByteDesc")-1, 1, (SOPC_Byte*) "subByteDesc"}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 50}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 3},
            .ValueRank = (-1),
            .AccessLevel = 3,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubInt16")-1, 1, (SOPC_Byte*) "SubInt16"}},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {1, {sizeof("subInt16")-1, 1, (SOPC_Byte*) "subInt16"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("subVarInt16")-1, 1, (SOPC_Byte*) "subVarInt16"}},
            .Description = {{0, 0, NULL}, {sizeof("subVarInt16Desc")-1, 1, (SOPC_Byte*) "subVarInt16Desc"}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 51}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 4},
            .ValueRank = (-1),
            .AccessLevel = 3,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubStatusCode")-1, 1, (SOPC_Byte*) "SubStatusCode"}},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {1, {sizeof("subStatusCode")-1, 1, (SOPC_Byte*) "subStatusCode"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("SubStatusCode")-1, 1, (SOPC_Byte*) "SubStatusCode"}},
            .Description = {{0, 0, NULL}, {sizeof("SubStatusCodeDesc")-1, 1, (SOPC_Byte*) "SubStatusCodeDesc"}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 52}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 19},
            .ValueRank = (-1),
            .AccessLevel = 3,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubString")-1, 1, (SOPC_Byte*) "SubString"}},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {1, {sizeof("subString")-1, 1, (SOPC_Byte*) "subString"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("subString")-1, 1, (SOPC_Byte*) "subString"}},
            .Description = {{0, 0, NULL}, {sizeof("subStringDesc")-1, 1, (SOPC_Byte*) "subStringDesc"}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 53}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 12},
            .ValueRank = (-1),
            .AccessLevel = 3,
        }}
    },
    {
        OpcUa_NodeClass_Variable,
        0x00,
        {0, 0},
        {.variable={
            .encodeableType = &OpcUa_VariableNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubUInt32")-1, 1, (SOPC_Byte*) "SubUInt32"}},
            .NodeClass = OpcUa_NodeClass_Variable,
            .BrowseName = {1, {sizeof("subUInt32")-1, 1, (SOPC_Byte*) "subUInt32"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("subUInt32")-1, 1, (SOPC_Byte*) "subUInt32"}},
            .Description = {{0, 0, NULL}, {sizeof("subUInt32Desc")-1, 1, (SOPC_Byte*) "subUInt32Desc"}},
            .NoOfReferences = 1,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 63}, {0, 0, NULL}, 0},
                }
            },
            .Value = 
                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 54}},
            .DataType = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 7},
            .ValueRank = (-1),
            .AccessLevel = 3,
        }}
    },
    {
        OpcUa_NodeClass_Object,
        OpcUa_UncertainInitialValue,
        {0, 0},
        {.object={
            .encodeableType = &OpcUa_ObjectNode_EncodeableType,
            .NodeId = {SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubVars")-1, 1, (SOPC_Byte*) "SubVars"}},
            .NodeClass = OpcUa_NodeClass_Object,
            .BrowseName = {1, {sizeof("SubscriberVars")-1, 1, (SOPC_Byte*) "SubscriberVars"}},
            .DisplayName = {{0, 0, NULL}, {sizeof("SubscriberVars")-1, 1, (SOPC_Byte*) "SubscriberVars"}},
            .Description = {{0, 0, NULL}, {sizeof("SubscriberVarsDesc")-1, 1, (SOPC_Byte*) "SubscriberVarsDesc"}},
            .NoOfReferences = 7,
            .References = (const OpcUa_ReferenceNode[]) {
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 40},
                    false,
                    {{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 61}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubString")-1, 1, (SOPC_Byte*) "SubString"}}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubByte")-1, 1, (SOPC_Byte*) "SubByte"}}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubUInt32")-1, 1, (SOPC_Byte*) "SubUInt32"}}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubInt16")-1, 1, (SOPC_Byte*) "SubInt16"}}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubBool")-1, 1, (SOPC_Byte*) "SubBool"}}, {0, 0, NULL}, 0},
                },
                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 47},
                    false,
                    {{SOPC_IdentifierType_String, 1, .Data.String = {sizeof("SubStatusCode")-1, 1, (SOPC_Byte*) "SubStatusCode"}}, {0, 0, NULL}, 0},
                }
            },
        }}
    },
};
SOPC_GCC_DIAGNOSTIC_RESTORE
const uint32_t SOPC_Embedded_AddressSpace_nNodes = 131;

// Index is provided by the corresponding Variable UInt32 Variant in SOPC_Embedded_AddressSpace_Nodes
SOPC_Variant SOPC_Embedded_VariableVariant[55] = {

                  {true,
                   SOPC_Int32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Int32 = 1}},

                  {true,
                   SOPC_Int32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Int32 = 1}},

                  {true,
                   SOPC_Int32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Int32 = 2}},

                  {true,
                   SOPC_Int32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Int32 = 2}},

                  {true,
                   SOPC_Int32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Int32 = 1}},

                  {true,
                   SOPC_String_Id,
                   SOPC_VariantArrayType_Array,
                   {.Array =
                     {1, {.StringArr = (SOPC_String[]){{sizeof("https://www.systerel.fr/S2OPC/demo/data/origin")-1, 1, (SOPC_Byte*) "https://www.systerel.fr/S2OPC/demo/data/origin"}}}}}},

                  {true,
                   SOPC_String_Id,
                   SOPC_VariantArrayType_Array,
                   {.Array =
                     {2, {.StringArr = (SOPC_String[]){{sizeof("http://opcfoundation.org/UA/")-1, 1, (SOPC_Byte*) "http://opcfoundation.org/UA/"},{sizeof("https://www.systerel.fr/S2OPC/demo/data/origin")-1, 1, (SOPC_Byte*) "https://www.systerel.fr/S2OPC/demo/data/origin"}}}}}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},

                  {true,
                   SOPC_LocalizedText_Id,
                   SOPC_VariantArrayType_Array,
                   {.Array =
                     {6, {.LocalizedTextArr = (SOPC_LocalizedText[]){{{0, 0, NULL}, {sizeof("None")-1, 1, (SOPC_Byte*) "None"}},{{0, 0, NULL}, {sizeof("Cold")-1, 1, (SOPC_Byte*) "Cold"}},{{0, 0, NULL}, {sizeof("Warm")-1, 1, (SOPC_Byte*) "Warm"}},{{0, 0, NULL}, {sizeof("Hot")-1, 1, (SOPC_Byte*) "Hot"}},{{0, 0, NULL}, {sizeof("Transparent")-1, 1, (SOPC_Byte*) "Transparent"}},{{0, 0, NULL}, {sizeof("HotAndMirrored")-1, 1, (SOPC_Byte*) "HotAndMirrored"}}}}}}},

                  {true,
                   SOPC_LocalizedText_Id,
                   SOPC_VariantArrayType_Array,
                   {.Array =
                     {8, {.LocalizedTextArr = (SOPC_LocalizedText[]){{{0, 0, NULL}, {sizeof("Running")-1, 1, (SOPC_Byte*) "Running"}},{{0, 0, NULL}, {sizeof("Failed")-1, 1, (SOPC_Byte*) "Failed"}},{{0, 0, NULL}, {sizeof("NoConfiguration")-1, 1, (SOPC_Byte*) "NoConfiguration"}},{{0, 0, NULL}, {sizeof("Suspended")-1, 1, (SOPC_Byte*) "Suspended"}},{{0, 0, NULL}, {sizeof("Shutdown")-1, 1, (SOPC_Byte*) "Shutdown"}},{{0, 0, NULL}, {sizeof("Test")-1, 1, (SOPC_Byte*) "Test"}},{{0, 0, NULL}, {sizeof("CommunicationFault")-1, 1, (SOPC_Byte*) "CommunicationFault"}},{{0, 0, NULL}, {sizeof("Unknown")-1, 1, (SOPC_Byte*) "Unknown"}}}}}}},

                  {true,
                   SOPC_Boolean_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Boolean = true}},

                  {true,
                   SOPC_Byte_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Byte = 127}},

                  {true,
                   SOPC_Int16_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Int16 = 0}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},

                  {true,
                   SOPC_String_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.String = {0, 0, NULL}}},

                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 1}},

                  {true,
                   SOPC_Boolean_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Boolean = true}},

                  {true,
                   SOPC_Byte_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Byte = 127}},

                  {true,
                   SOPC_Int16_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Int16 = 1}},
{true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {0}},

                  {true,
                   SOPC_String_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.String = {0, 0, NULL}}},

                  {true,
                   SOPC_UInt32_Id,
                   SOPC_VariantArrayType_SingleValue,
                   {.Uint32 = 1}},
};
const uint32_t SOPC_Embedded_VariableVariant_nb = 55;
