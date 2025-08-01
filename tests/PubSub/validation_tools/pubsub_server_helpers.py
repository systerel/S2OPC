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

from pubsub_server import PubSubState
from time import sleep

def waitForEvent(res_fcn, maxWait_s=2.0, period_s=0.05):
    """
        @param res_fcn a callable function (no parameters). Must return a boolean.
        @param maxWait_s Timeout in second waiting for res_fcn() to return True
        @param period_s Polling period
        @return True if res_fcn() returned true within expected time
    """
    _t = maxWait_s;
    res = res_fcn()
    while _t >= 0 and not res:
        _t -= period_s
        sleep(period_s)
        res = res_fcn()
    return res

def helpAssertState(psserver, expected, pLogger):
    waitForEvent(lambda:psserver.getPubSubState() == expected)
    state = psserver.getPubSubState()
    pLogger.add_test(f'PubSub Module state is {state}, should be {expected}', state == expected)

# TODO: group these helpers in an helper class that wraps both the client to the pubsub_server and the logger instances
# Test connection and status depending of pStart command
def helpTestStopStart(pPubsubserver, pStart, pLogger, possibleFail=False):
    if not possibleFail:
        connected = waitForEvent(lambda:pPubsubserver.isConnected())
        expectedPubSubStatus = waitForEvent(lambda: pStart == pPubsubserver.isStart())
        pLogger.add_test('Connected to pubsub_server', connected)
        if pStart:
            pLogger.add_test('PubSub Module is started' , expectedPubSubStatus)
        else:
            pLogger.add_test('PubSub Module is stopped', expectedPubSubStatus)
    # TODO: for now "possibleFail" is in fact "expectedFail"
    if pStart:
        if not possibleFail:
            helpAssertState(pPubsubserver, PubSubState.OPERATIONAL, pLogger)
    else:
        helpAssertState(pPubsubserver, PubSubState.DISABLED, pLogger)

# Send stop command following by start command
def helpRestart(pPubsubserver, pLogger, possibleFail):
    pLogger.add_test('Stop Server', True)
    pPubsubserver.stop()
    helpTestStopStart(pPubsubserver, False, pLogger, possibleFail)
    pLogger.add_test('Start Server', True)
    pPubsubserver.start()
    helpTestStopStart(pPubsubserver, True, pLogger, possibleFail)

# Change the configuration
def helpConfigurationChange(pPubsubserver, pConfig, pLogger):
    pLogger.add_test('Set Configuration', True)
    pPubsubserver.setConfiguration(pConfig)
    pLogger.add_test('Connected to OPCUA Server', pPubsubserver.isConnected())
    pubsubconfiguration = pPubsubserver.getConfiguration()
    pLogger.add_test('PubSub Configuration Node is changed', pConfig == pubsubconfiguration)

# Change the configuration and restart PubSub Server
# Test connection, status and configuration
def helpConfigurationChangeAndStart(defaultXmlPath, pPubsubserver, pConfig, pLogger, static=False, possibleFail=False):
    fd = open(defaultXmlPath, "r")
    oldConfig = fd.read()
    fd.close()
    helpConfigurationChange(pPubsubserver, pConfig, pLogger)
    helpRestart(pPubsubserver, pLogger, possibleFail)
    # check configuration saved in default file
    if not possibleFail:
        fd = open(defaultXmlPath, "r")
        if static:
            pLogger.add_test('Default PubSub Configuration file is not changed', oldConfig == fd.read())
        else:
            rd = fd.read()
            pLogger.add_test('Default PubSub Configuration file is changed', pConfig == rd)
            if not(pConfig == rd):
                pLogger.add_test(f"pConfig={pConfig}",True)
                pLogger.add_test(f"fd.read()={rd}",False)
        fd.close()
