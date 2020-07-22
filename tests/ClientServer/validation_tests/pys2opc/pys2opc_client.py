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


"""
PyS2OPC library validation tests.
Connects to the S2OPC validation server (secured and unsecured),
adds nodes to a subscription,
TODO: get endpoints,
makes a read,
makes a write,
check that the change in value is received through the subscription,
browse some nodes.

Does this again with two or more connections in parallel.

Creates a subscription with unreasonable publish cycle, longer than the request timeoutHint of a PublishRequest
(this is the (SOPC_REQUEST_TIMEOUT_MS/2) constant from src/ClientServer/configuration/sopc_toolkit_config_constants.h, defined as 5 seconds).
"""

# Manage relative paths
S2OPC_ROOT='../../../../'
BINARY_DIR= S2OPC_ROOT + 'build/bin'
VALIDATION_DIR= S2OPC_ROOT + 'validation'

from itertools import product
import time
import threading

from pys2opc import PyS2OPC_Client as PyS2OPC, BaseClientConnectionHandler, SecurityMode, SecurityPolicy, AttributeId, VariantType, Variant, NodeClass, DataValue, StatusCode

import sys; sys.path.insert(0, VALIDATION_DIR)
from tap_logger import TapLogger
# WARNING, the following import makes a dependency on freeopcua
from common import sUri as serverUrl, variantInfoList, ua, browseSubTree


TIMEOUT_RESPONSE = 5.

# Modular configuration parameters
config_base = {'server_url': serverUrl}
config_no_subscription = {'token_target': 0}
config_subscription = {'publish_period' : 500,
                       'n_max_keepalive' : 3,
                       'token_target': 3}
config_subscription_timeout = {'publish_period': 10000,
                               'token_target': 1}
config_no_security = {'add_configuration_function': PyS2OPC.add_configuration_unsecured}
config_security = {'security_mode': SecurityMode.Sign,
                   'security_policy': SecurityPolicy.Basic256,
                   'path_cert_auth': BINARY_DIR + '/trusted/cacert.der',
                   'path_crl': BINARY_DIR + '/revoked/cacrl.der',
                   'path_cert_srv': BINARY_DIR + '/server_public/server_2k_cert.der',
                   'path_cert_cli': BINARY_DIR + '/client_public/client_2k_cert.der',
                   'path_key_cli': BINARY_DIR + '/client_private/client_2k_key.pem',
                   'add_configuration_function': PyS2OPC.add_configuration_secured}


# Transform freeopcua types to pys2opc types
PYS2OPC_TYPES = {ua.VariantType.Int64: VariantType.Int64,
                 ua.VariantType.UInt32: VariantType.UInt32,
                 ua.VariantType.Double: VariantType.Double,
                 ua.VariantType.String: VariantType.String,
                 ua.VariantType.ByteString: VariantType.ByteString,
                 ua.VariantType.XmlElement: VariantType.XmlElement,
                 ua.VariantType.SByte: VariantType.SByte,
                 ua.VariantType.Byte: VariantType.Byte,
                 ua.VariantType.Int16: VariantType.Int16,
                 ua.VariantType.UInt16: VariantType.UInt16,
                 ua.VariantType.Int32: VariantType.Int32,
                 ua.VariantType.UInt64: VariantType.UInt64,
                 ua.VariantType.Float: VariantType.Float}


class ConnectionHandler(BaseClientConnectionHandler):
    """
    BaseClientConnectionHandler with datachange callback and new methods which implement the tests.
    """
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.pendingRequests = {}  # {nid: request}
        self.responses = {}  # {nid: response}
        self.logger = None
        # Set on the call to on_datachanged that empties expectedChanges dicts
        self.subscriptionComplete = threading.Event()
        self.expectedChangesInit = {}  # {nid: val}
        self.expectedChangesNew = {}  # {nid: val}
        self.i = 0

    def set_logger(self, logger):
        self.logger = logger

    def _wait_for_responses(self):
        """
        Waits that all requests from self.pendingRequests have a response.
        Stores the response in self.responses.
        Waits at most TIMEOUT_RESPONSE.
        """
        t0 = time.time()
        while self.pendingRequests:
            for nid,req in list(self.pendingRequests.items()):
                resp = self.get_response(req)
                if resp is not None:
                    del self.pendingRequests[nid]
                    self.responses[nid] = resp
            time.sleep(.1)
            if time.time() - t0 > TIMEOUT_RESPONSE:
                raise TimeoutError

    def on_datachanged(self, nodeId, dataValue):
        # On each change notification, finds if the notification is expected or not,
        #  and mark that it is received by deleting the expectation.
        initValue = self.expectedChangesInit.get(nodeId, None)
        newValue = self.expectedChangesNew.get(nodeId, None)
        if dataValue.variant.variantType == VariantType.Float:
            if initValue is not None and abs((dataValue.variant - initValue)/dataValue.variant) <= 2**(-24):
                del self.expectedChangesInit[nodeId]
            if newValue is not None and abs((dataValue.variant - newValue)/dataValue.variant) <= 2**(-24):
                del self.expectedChangesNew[nodeId]
        elif dataValue.variant.variantType == VariantType.XmlElement:
            if initValue is not None and dataValue.variant == initValue.Value.encode():
                del self.expectedChangesInit[nodeId]
            if newValue is not None and dataValue.variant == newValue.Value.encode():
                del self.expectedChangesNew[nodeId]
        else:
            if initValue is not None and dataValue.variant == initValue:
                del self.expectedChangesInit[nodeId]
            if newValue is not None and dataValue.variant == newValue:
                del self.expectedChangesNew[nodeId]

        # When no more notifications are expected, flag the Event.
        if not self.expectedChangesInit and not self.expectedChangesNew:
            self.subscriptionComplete.set()

    def test_read_send_requests(self, onlyValues = False):
        """
        Sends all the requests and does not wait on them.

        Args:
            onlyValues: When set, sends a single request for the values.
        """
        # Check browse name, display name, description, value, node class, variant type (TODO DataType)
        # Create a request per node to read
        if not onlyValues:
            attrs_per_node = [AttributeId.BrowseName, AttributeId.DisplayName, AttributeId.Description, AttributeId.Value, AttributeId.NodeClass]
            for i,(typeName,uaType,initValue,newValue) in enumerate(variantInfoList):
                nid = 'ns={};i={}'.format(1, 1001+i)
                assert nid not in self.pendingRequests
                self.pendingRequests[nid] = self.read_nodes([nid]*len(attrs_per_node), attrs_per_node, bWaitResponse=False)
        else:
            self.pendingRequests['read'] = self.read_nodes(['ns={};i={}'.format(1, 1001+i) for i,_ in enumerate(variantInfoList)], bWaitResponse=False)

    def test_read_check_results(self, readInitValues = True, onlyValues = False):
        """
        Waits for the responses and check the contents.

        Args:
            readInitValues: When reset, expects the newValue instead of the initValue.
            onlyValues: When set, expects a single response with values only.
        """
        try:
            self._wait_for_responses()
            self.logger.add_test('Receive asynch responses.', True)
        except TimeoutError:
            self.logger.add_test('Receive asynch responses.', False)
            print('Error, could not synchronize read responses, abort.')
            return

        def check_value(nid, uaType, value, initValue, newValue, readInitValues):
            if readInitValues:
                expectedValue = initValue
            else:
                expectedValue = newValue
            if PYS2OPC_TYPES[uaType] == VariantType.Float:
                self.logger.add_test('Simple precision float value of Node {}'.format(nid), abs((value.variant - expectedValue)/value.variant) <= 2**(-24))
            elif PYS2OPC_TYPES[uaType] == VariantType.XmlElement:
                self.logger.add_test('XmlElement value of Node {}'.format(nid), value.variant == expectedValue.Value.encode())
            else:
                self.logger.add_test('Value of Node {}'.format(nid), value.variant == expectedValue)

        if not onlyValues:
            for i,(typeName,uaType,initValue,newValue) in enumerate(variantInfoList):
                nid = 'ns={};i={}'.format(1, 1001+i)
                assert nid in self.responses
                browseName, displayName, description, value, nodeClass = self.responses.pop(nid).results
                check_value(nid, uaType, value, initValue, newValue, readInitValues)
                self.logger.add_test('BrowseName of Node {}'.format(nid), browseName.variant == (1, typeName))
                self.logger.add_test('DisplayName of Node {}'.format(nid), displayName.variant == ('', '{}_1dn'.format(typeName)))
                self.logger.add_test('NodeClass of Node {}'.format(nid), nodeClass.variant == NodeClass.Variable)
        else:
            for i,((_,uaType,initValue,newValue),value) in enumerate(zip(variantInfoList, self.responses.pop('read').results)):
                nid = 'ns={};i={}'.format(1, 1001+i)
                check_value(nid, uaType, value, initValue, newValue, readInitValues)

    def test_read(self, onlyValues = False):
        """
        Sends the requests and check the contents.

        Args:
            onlyValues: When set, expects a single response with values only.
        """
        self.logger.begin_section('Read Tests -')
        self._test_read(readInitValues = True, onlyValues = onlyValues)

    def _test_read(self, readInitValues = True, onlyValues = False):
        self.test_read_send_requests(onlyValues = onlyValues)
        self.test_read_check_results(readInitValues = readInitValues, onlyValues = onlyValues)

    def test_write_and_assert(self, resetInitValues = False):
        """
        Writes the new random value for each node.
        Sends a write request and assert the server responded OKs (waits).
        Does not read the results.

        Args:
            resetInitValues: when set, writes the initValues instead of the new random values.
        """
        # Explicitly filters the values to replace the XmlElement with something less freeopcua.
        nids, values = [], []
        for i, (_, uaType, initVal, newVal) in enumerate(variantInfoList):
            nids.append('ns={};i={}'.format(1, 1001+i))
            if not resetInitValues:
                value = newVal
            else:
                value = initVal
            if PYS2OPC_TYPES[uaType] == VariantType.XmlElement:
                value = value.Value.encode()
            values.append(DataValue.from_python(value))
        assert self.write_nodes(nids, values).is_ok()

    def test_write(self):
        """
        Sends the write request, wait for the response, sends multiple requests to assert the values changed.
        """
        # Writes the new values
        self.logger.begin_section('Write Tests -')
        self.test_write_and_assert()
        self._test_read(readInitValues = False, onlyValues = True)
        # Writes back the old values
        self.logger.begin_section('Write initValue Tests -')
        self.test_write_and_assert(resetInitValues = True)
        self._test_read(onlyValues = True)

    def test_browse(self):
        """
        Sends a Browse requests, waits for the response and analyzes it.
        """
        self.logger.begin_section('Browse Tests -')
        nid, forwardHierBrowseNames, forwardNonHierNids, backwardNid = browseSubTree
        forwardNonHierNids = list(forwardNonHierNids)
        for bn in forwardHierBrowseNames:
            forwardNonHierNids.append('ns=1;s=' + bn)
        # Make the request
        response = self.browse_nodes([nid])
        result, = response.results  # Asserts that there is a single response
        self.logger.add_test('Response status OK', result.status == StatusCode.Good)
        # Expects forward references to sub nodes, computes the parent and expects it in the backward references
        forwardNodes = set(ref.nodeId for ref in result.references if ref.isForward)
        self.logger.add_test('Forward references are correct', set(forwardNonHierNids) == forwardNodes)
        backwardNodes = set(ref.nodeId for ref in result.references if not ref.isForward)
        self.logger.add_test('Backward is correct', {backwardNid} == backwardNodes)

    def configure_subscription(self):
        if self.configuration.parameters['token_target'] > 0:
            nids = []
            for i, (_, _, initVal, newVal) in enumerate(variantInfoList):
                nid = 'ns={};i={}'.format(1, 1001+i)
                nids.append(nid)
                self.expectedChangesInit[nid] = initVal
                self.expectedChangesNew[nid] = newVal
            self.add_nodes_to_subscription(nids)
        else:
            self.subscriptionComplete.set()


if __name__ == '__main__':
    logger_ = TapLogger('validation_pys2opc.tap')

    with PyS2OPC.initialize():
        # Creates the configurations by combining parameter groups
        configs = []
        for cfgb, cfgsub, cfgsec in product([config_base],
                                            [config_no_subscription, config_subscription],
                                            [config_no_security, config_security]):
            cfg = cfgb.copy()
            cfg.update(cfgsub)
            cfg.update(cfgsec)
            add_configuration_function = cfg.pop('add_configuration_function')
            configs.append(add_configuration_function(**cfg))
        PyS2OPC.mark_configured()

        # Do the tests on all configurations
        for config in configs:
            with PyS2OPC.connect(configs[0], ConnectionHandler) as connection:
                print('New Connection')
                connection.set_logger(logger_)
                print('Read Tests')
                connection.test_read()
                print('Write Tests')
                connection.test_write()
                print('Browse Tests')
                connection.test_browse()

        # Do the read/write with multiple connections
        print('Asynch Write/Reads on new connections')
        connections = [PyS2OPC.connect(cfg, ConnectionHandler) for cfg in configs]
        for conn in connections:
            conn.set_logger(logger_)
            conn.configure_subscription()
        try:
            # Do the write on one connection, reads on all others to assert the result.
            logger_.begin_section('ReadWrite Tests -')
            connA, *conns = connections
            connA.test_write_and_assert()
            for conn in conns:
                conn._test_read(readInitValues = False, onlyValues = True)
            connA.test_write_and_assert(resetInitValues = True)
            # Wait for subscriptions to end. Does not wait more than the publish cycle,
            #  as the last notifications should be received by then.
            timeout = .001*max(cfg.parameters.get('publish_period_ms', 0.) for cfg in configs)
            for conn in connections:
                assert conn.subscriptionComplete.wait(timeout)
            logger_.add_test('Values all correctly notified', True)
        finally:
            for conn in connections:
                conn.disconnect()

    logger_.finalize_report()
    sys.exit(1 if logger_.has_failed_tests else 0)
