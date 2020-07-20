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
Example script: this script uses the automatically created subscription,
and add nodes to this subscription.

This is mostly illustrative, as the nodes do not change in value.
"""


import time

from pys2opc import PyS2OPC_Client as PyS2OPC, BaseClientConnectionHandler
from _connection_configuration import configuration_parameters_subscription


class PrintSubs(BaseClientConnectionHandler):
    """
    Derivates from BaseClientConnectionHandler to print values on datachange notifications.
    """
    def on_datachanged(self, nodeId, dataValue):
        #  dataValue contains the source and server timestamps, as well as the new value.
        print('Data changed "{}" -> {}, '.format(nodeId, dataValue.variant) + time.ctime(dataValue.timestampServer))


NODES = ['s=BRD.NC_000.VP_96.TM.TSEN1.PTSE_TS1_DELTAP_P20_RAW',
         's=BRD.NC_000.VP_96.TM.TF.PMC2_TF_MODE_MPPT_RAW',
         's=BRD.NC_000.VP_96.TC.OBC_TC_LOAD_NTEL.CHIFFRE03_RAW',
         's=BRD.NC_000.VP_96.TC.MC2_TC_MODE_SELECT_GS.MC2_AR_ID_MODE_SELECT_GS_RAW',
         's=BRD.NC_000.VP_96.TM.TMAI.POBC_MA_CALL_PERIOD_RAW',
         's=BRD.NC_000.VP_96.TM.TSEN2.PTSE_TS2_DP_SIGN_D20_RAW']


if __name__ == '__main__':
    with PyS2OPC.initialize():
        # Subscription parameters are defined in configuration_parameters_no_subscription
        config = PyS2OPC.add_configuration_unsecured(**configuration_parameters_subscription)
        PyS2OPC.mark_configured()
        with PyS2OPC.connect(config, PrintSubs) as connection:
            # Add multiple nodes to the subscription.
            # The subscription always notifies of the first value.
            connection.add_nodes_to_subscription(NODES)
            # We wait for notifications up to 5 seconds.
            time.sleep(5.)
            print('Closing the connection.')

