#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

from opcua import ua

var_path_prefixes = ["1:scalar", "1:rw"]
var_types=["Boolean", "ByteString", "Byte", "Double", "Float", "Int16", "Int32", "SByte", "UInt16", "UInt32", "UInt64" ]
var_max_counter = 3

def get_bp_of_organized_path(organized_path_qname_list):
    bp = ua.uaprotocol_auto.BrowsePath()
    bp.StartingNode=ua.NodeId(85)
    # Add organized path
    for organized_path_qname in organized_path_qname_list:
        organized_path_ns, organized_path_name = organized_path_qname.split(":")
        rpe = ua.uaprotocol_auto.RelativePathElement()
        rpe.IsInverse=False
        rpe.ReferenceTypeId=ua.NodeId(35) # Organizes reference required
        rpe.TargetName.NamespaceIndex=int(organized_path_ns)
        rpe.TargetName.Name=organized_path_name
        bp.RelativePath.Elements.append(rpe)

    return bp

def get_bp_to_component_with_intermediate_organized_path(organized_path_qname_list,
                                                         target_component_qname):
    bp = get_bp_of_organized_path(organized_path_qname_list)
    # Add final component path
    target_component_ns, target_component_name = target_component_qname.split(":")
    rpe = ua.uaprotocol_auto.RelativePathElement()
    rpe.ReferenceTypeId=ua.NodeId(47)
    rpe.IsInverse=False
    rpe.TargetName.NamespaceIndex=int(target_component_ns)
    rpe.TargetName.Name=target_component_name
    bp.RelativePath.Elements.append(rpe)

    return bp

def translate_browse_paths_to_node_ids_tests(client, logger):
    # Simple variable paths test
    tbpaths = []
    simple_paths = []
    expected_node_ids = []
    nsIndex = 1
    for var_type in var_types:
        for count in range(1,var_max_counter+1):
            target_name = "{}_{:03d}".format(var_type, count)
            target_qname = "{}:{}".format(nsIndex, target_name)
            simple_paths.append("{}.{}.{}".format(var_path_prefixes[0], var_path_prefixes[1], target_qname))
            # in this case nodeIds are the target name in the NS=1
            expected_node_ids.append(ua.NodeId(target_name, nsIndex))
            tbpaths.append(get_bp_to_component_with_intermediate_organized_path(var_path_prefixes, target_qname))

    print("TranslateBrowsePathToNodeIds for paths {}".format(simple_paths))
    tbp_results = client.uaclient.translate_browsepaths_to_nodeids(tbpaths)
    res = len(tbp_results) == len(expected_node_ids)
    if res:
        for i in range(0, len(tbp_results)):
            tbp_result = tbp_results[i]
            res = res and tbp_result.StatusCode.is_good()
            res = res and len(tbp_result.Targets) == 1
            res = res and tbp_result.Targets[0].TargetId == expected_node_ids[i]
            res = res and tbp_result.Targets[0].RemainingPathIndex == 4294967295 # means no remaining path to treat
    logger.add_test('TBPtoNodeIds Test - NodeIds for paths {}'.format(simple_paths), res)

    # Simple degraded variable paths test (incorrect namespace with same paths)
    tbpaths.clear()
    simple_paths.clear()
    expected_node_ids.clear()

    not_expected_node_ids = []
    nsIndex = 0
    for var_type in var_types:
        for count in range(1,var_max_counter+1):
            target_name = "{}_{:03d}".format(var_type, count)
            # build a target qname in NS=0
            target_qname = "{}:{}".format(nsIndex, target_name)
            # in this case the valid nodeIds are the same target name in the NS=1 but we will ask for those in NS=0
            simple_paths.append("{}.{}.{}".format(var_path_prefixes[0], var_path_prefixes[1], target_qname))
            not_expected_node_ids.append(ua.NodeId(target_name, namespaceidx=0))
            tbpaths.append(get_bp_to_component_with_intermediate_organized_path(var_path_prefixes, target_qname))

    print("TranslateBrowsePathToNodeIds for paths with incorrect NS index {}".format(simple_paths))
    tbp_results = client.uaclient.translate_browsepaths_to_nodeids(tbpaths)
    res = len(tbp_results) == len(not_expected_node_ids)
    if res:
        for i in range(0, len(tbp_results)):
            tbp_result = tbp_results[i]
            res = res and not tbp_result.StatusCode.is_good()
    logger.add_test('TBPtoNodeIds Test - NodeIds are not found for incorrect paths {}'.format(simple_paths), res)

    # Variable with common path name (but different qualified name or different final target node)
    tbpaths.clear()
    simple_paths.clear()
    expected_node_ids.clear()
    not_expected_node_ids.clear()
    organized_path = "1:15361.1:BLOCKs.1:B_1.1:RM"
    component_qname = "1:LeftSubRoute"
    nsIndex = 1
    tbpaths.append(get_bp_to_component_with_intermediate_organized_path(organized_path.split("."),
                                                                        component_qname))
    simple_paths.append("{}.{}".format(organized_path, component_qname))
    expected_node_ids.append(ua.NodeId("Objects.15361.BLOCKs.B_1.RM.LeftSubRoute", nsIndex))

    organized_path = "1:15361.1:BLOCKs.1:B_1"
    component_qname = "1:TSRApplied"
    tbpaths.append(get_bp_to_component_with_intermediate_organized_path(organized_path.split("."),
                                                                        component_qname))
    simple_paths.append("{}.{}".format(organized_path, component_qname))
    expected_node_ids.append(ua.NodeId("Objects.7681.BLOCKs.B_1.TSRApplied", nsIndex))

    organized_path = "1:15361.2:BLOCKs.2:B_1.2:RM"
    component_qname = "2:LeftSubRoute"
    nsIndex = 2
    tbpaths.append(get_bp_to_component_with_intermediate_organized_path(organized_path.split("."),
                                                                        component_qname))
    simple_paths.append("{}.{}".format(organized_path, component_qname))
    expected_node_ids.append(ua.NodeId("Objects.15362.BLOCKs.B_1.RM.LeftSubRoute", nsIndex))

    print("TranslateBrowsePathToNodeIds for paths with common path with different qualified name or same non qualified name {}".format(simple_paths))
    tbp_results = client.uaclient.translate_browsepaths_to_nodeids(tbpaths)
    res = len(tbp_results) == len(expected_node_ids)
    if res:
        for i in range(0, len(tbp_results)):
            tbp_result = tbp_results[i]
            res = res and tbp_result.StatusCode.is_good()
            res = res and len(tbp_result.Targets) == 1
            res = res and tbp_result.Targets[0].TargetId == expected_node_ids[i]
            res = res and tbp_result.Targets[0].RemainingPathIndex == 4294967295 # means no remaining path to treat
    logger.add_test('TBPtoNodeIds Test - NodeIds for paths with common path with different qualified name or same non qualified name {}'.format(simple_paths), res)


    # Folder with several matches regarding browse path
    tbpaths.clear()
    simple_paths.clear()
    expected_node_ids.clear()
    organized_path = "1:15361.1:BLOCKs.1:B_1"
    nsIndex = 1
    tbpaths.append(get_bp_of_organized_path(organized_path.split(".")))
    simple_paths.append(organized_path)
    expected_node_ids.append(ua.NodeId("Objects.15361.BLOCKs.B_1", nsIndex))
    expected_node_ids.append(ua.NodeId("Objects.7681.BLOCKs.B_1", nsIndex))

    print("TranslateBrowsePathToNodeIds for common path {} with several possible target NodeIds {}".format(simple_paths, expected_node_ids))
    tbp_results = client.uaclient.translate_browsepaths_to_nodeids(tbpaths)
    # A path requested
    res = len(tbp_results) == len(simple_paths) == 1
    # Several results expected
    res = res and len(tbp_results[0].Targets) == len(expected_node_ids)
    res = res and tbp_result.StatusCode.is_good()
    tmp_expected_node_ids = expected_node_ids.copy()
    if res:
        tbp_result = tbp_results[0]
        for i in range(0, len(tbp_result.Targets)):
            is_expected_node_id = False
            for expNodeId in tmp_expected_node_ids:
                # An expected NodeId shall found
                if tbp_result.Targets[i].TargetId == expNodeId:
                    is_expected_node_id = True
            if is_expected_node_id:
                # Ensure all expected node id will be consumed in the end
                tmp_expected_node_ids.remove(tbp_result.Targets[i].TargetId)
            res = res and is_expected_node_id
            res = res and tbp_result.Targets[i].RemainingPathIndex == 4294967295 # means no remaining path to treat
    res = res and len(tmp_expected_node_ids) == 0
    logger.add_test('TBPtoNodeIds Test - Different NodeIds expected {} for common path {}'.format(expected_node_ids, simple_paths), res)

