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


from contextlib import contextmanager
import time
import os
import sys
import json
import signal
import getpass

from collections import namedtuple

from _pys2opc import ffi, lib as libsub
from .connection import BaseClientConnectionHandler
from .types import DataValue, ReturnStatus, SecurityPolicy, SecurityMode, LogLevel
from .responses import Response
from .server_callbacks import BaseAddressSpaceHandler
from .request import LocalAsyncRequestHandler, Request

VERSION = json.load(open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'version.json')))['version']

NULL = ffi.NULL

allocator_no_gc = ffi.new_allocator(alloc=libsub.SOPC_Malloc, free=None, should_clear_after_alloc=True)

# Note: this level only concerns the following function _callback_log
# TODO: make this configurable for the end user
LOG_LEVEL = libsub.SOPC_LOG_LEVEL_DEBUG

# TODO: merge all logs system
@ffi.def_extern()
def _callback_log(level, text):
    """
    Receives log information from the LibSub (not from the S2OPC toolkit).
    """
    dLevel = {libsub.SOPC_LOG_LEVEL_ERROR: '# Error: ',
              libsub.SOPC_LOG_LEVEL_WARNING: '# Warning: ',
              libsub.SOPC_LOG_LEVEL_INFO: '# Info: ',
              libsub.SOPC_LOG_LEVEL_DEBUG: '# Debug: '}
    if level <= LOG_LEVEL:
        print(dLevel[level] + ffi.string(text).decode(), file=sys.stderr)

@ffi.def_extern()
def _callback_disconnected(clientId):
    return PyS2OPC_Client._callback_disconnected(clientId)

@ffi.def_extern()
def _callback_datachanged(connectionId, dataId, c_value):
    return PyS2OPC_Client._callback_datachanged(connectionId, dataId, c_value)

@ffi.def_extern()
def _callback_client_event(connectionId, event, status, responsePayload, responseContext):
    timestamp = time.time()
    return PyS2OPC_Client._callback_client_event(connectionId, event, status, responsePayload, responseContext, timestamp)

@ffi.def_extern()
def _callback_toolkit_event(event, status, param, appContext):
    timestamp = time.time()
    return PyS2OPC._callback_toolkit_event(event, status, param, appContext, timestamp)

@ffi.def_extern()
def _callback_address_space_event(ctx, event, operationParam, operationStatus):
    return PyS2OPC_Server._callback_address_space_event(ctx, event, operationParam, operationStatus)

@ffi.def_extern()
def _callback_validate_user_identity(authenticationManager, pUser, pUserAuthenticated):
    return PyS2OPC_Server._callback_validate_user_identity(authenticationManager, pUser, pUserAuthenticated)

@ffi.def_extern()
def _callback_authorize_operation(authorizationManager, operationType, nodeId, attributeId, pUser, pbOperationAuthorized):
    return PyS2OPC_Server._callback_authorize_operation(authorizationManager, operationType, nodeId, attributeId, pUser, pbOperationAuthorized);

@ffi.def_extern()
def _callback_get_client_key_password(password):
    try:
        pwd = PyS2OPC_Client.get_client_key_password()
        pwd = pwd.encode() + b'\0' # Add protection to avoid buffer overrun with C code
        password[0] = allocator_no_gc('char[{}]'.format(len(pwd)), pwd)
    except Exception:
        return False
    return True

class PyS2OPC:
    """
    Python version of the S2OPC + client subscription libraries.
    Base class for components that are common to both Clients and Servers.

    For now, only either a `PyS2OPC_Client` or a `PyS2OPC_Server`
    can be initialized in a process.
    """
    _initialized_cli = False
    _initialized_srv = False
    _configured = False
    _events_client = {libsub.SE_SESSION_ACTIVATION_FAILURE,
                      libsub.SE_ACTIVATED_SESSION,
                      libsub.SE_SESSION_REACTIVATING,
                      libsub.SE_RCV_SESSION_RESPONSE,
                      libsub.SE_CLOSED_SESSION,
                      libsub.SE_RCV_DISCOVERY_RESPONSE,
                      libsub.SE_SND_REQUEST_FAILED}
    _events_server = {libsub.SE_CLOSED_ENDPOINT,
                      libsub.SE_LOCAL_SERVICE_RESPONSE}

    @staticmethod
    def initialize():
        raise NotImplementedError

    @staticmethod
    def _assert_not_init():
        assert not PyS2OPC._initialized_cli, 'This PyS2OPC process is alread initialized as a Client'
        assert not PyS2OPC._initialized_srv, 'This PyS2OPC process is alread initialized as a Server'


    @staticmethod
    def get_version():
        # TODO: use build infos
        """Returns complete version string (PyS2OPC, LibSub, S2OPC)"""
        return 'PyS2OPC v' + VERSION + ' on ' + ffi.string(libsub.SOPC_LibSub_GetVersion()).decode()

    @staticmethod
    def mark_configured():
        """
        Freeze the configuration of the toolkit.
        Must be called after all calls to:

        - `PyS2OPC_Client.add_configuration_unsecured`, `PyS2OPC_Client.add_configuration_secured`,
        - `PyS2OPC_Server.load_address_space`, and `PyS2OPC_Server.load_configuration`,

        and before calls to:

        - `PyS2OPC_Client.connect`, `PyS2OPC_Client.get_endpoints`,
        - `PyS2OPC_Server.serve`, and `PyS2OPC_Server.serve_forever`.
        """
        assert (PyS2OPC._initialized_cli or PyS2OPC._initialized_srv) and not PyS2OPC._configured,\
            'Toolkit is not initialized or already configured, cannot add new configurations.'
        PyS2OPC._configured = True

    @staticmethod
    def _callback_toolkit_event(event, status, param, appContext, timestamp):
        assert event in PyS2OPC._events_client | PyS2OPC._events_server, 'Unknown event received from Toolkit "{}"'.format(event)
        if event in PyS2OPC._events_server:
            PyS2OPC_Server._callback_toolkit_event(event, status, param, appContext, timestamp)
        else:
            assert event not in PyS2OPC._events_client, 'Only server events are supported yet'

    @staticmethod
    def _get_password(prompt):
        """
        Return a zero-terminated byte string which contain the password.
        """
        pwd = getpass.getpass(prompt='{}'.format(prompt))
        return pwd

class PyS2OPC_Client(PyS2OPC):
    """
    The Client side of the PyS2OPC library.
    Used to create clients only.
    """
    _dConfigurations = {}  # Stores client known configurations {Id: configurationParameters} (client and server configurations may have the same index)
    _dConnections = {}  # Stores known active connections {Id: ConnectionHandler()}

    @staticmethod
    @contextmanager
    def initialize(logLevel=LogLevel.Debug, logPath='logs/', logFileMaxBytes=1048576, logMaxFileNumber=50):
        """
        Toolkit and LibSub initializations for Clients.
        When the toolkit is initialized for clients, it cannot be used to make a server before a `clear()`.

        This function supports the context management:
        >>> with PyS2OPC_Client.initialize():
        ...     # Do things here
        ...     pass

        When reaching out of the `with` statement, the Toolkit is automatically cleared.
        See `clear()`.

        Args:
            logLevel: log level for the toolkit logs (one of the `pys2opc.types.LogLevel` values).
            logPath: the path for logs (the current working directory) to logPath.
                     logPath is created if it does not exist.
            logFileMAxBytes: The maximum size (best effort) of the log files, before changing the log index.
            logMaxFileNumber: The maximum number of log indexes before cycling logs and reusing the first log.

        """
        PyS2OPC._assert_not_init()

        status = libsub.SOPC_LibSub_Initialize([(libsub._callback_log,
                                                 libsub._callback_disconnected,
                                                 (logLevel,
                                                  ffi.new('char[]', logPath.encode()),
                                                  logFileMaxBytes,
                                                  logMaxFileNumber))])
        assert status == ReturnStatus.OK, 'Library initialization failed with status {}.'.format(ReturnStatus.get_both_from_id(status))
        PyS2OPC._initialized_cli = True

        try:
            yield
        finally:
            PyS2OPC_Client.clear()

    @staticmethod
    def clear():
        """
        Disconnect current servers and clears the Toolkit.
        Existing `ClientConfiguration`s and Connections are then invalid and should be freed.
        """
        # TODO: Disconnect existing clients
        libsub.SOPC_LibSub_Clear()
        PyS2OPC._initialized_cli = False
        PyS2OPC._configured = False

    @staticmethod
    def add_configuration_unsecured(*,
                                    server_url = 'opc.tcp://localhost:4841',
                                    publish_period = 500,
                                    n_max_keepalive = 3,
                                    n_max_lifetime = 10,
                                    timeout_ms = 10000,
                                    sc_lifetime = 3600000,
                                    token_target = 3):
        """
        Returns a configuration that can be later used in `PyS2OPC_Client.connect` or `PyS2OPC_Client.get_endpoints`.

        Args:
            server_url: The endpoint and server url to connect to.
            publish_period: The period of the subscription, in ms.
            n_max_keepalive: The number of times the subscription has no notification to send before
                             sending an empty `PublishResponse` (the KeepAlive message). It is necessary
                             to keep `n_max_keepalive*timeout_ms*token_target < REQUEST_TIMEOUT (5000ms)`.
            n_max_lifetime: The maximum number of times a subscription has notifications to send
                            but no available token. In this case, the subscription is destroyed.
            timeout_ms: The `PyS2OPC_Client.connect` timeout, in ms.
            sc_lifetime: The target lifetime of the secure channel, before renewal, in ms.
            token_target: The number of subscription tokens (PublishRequest) that should be
                          made available to the server at anytime.
        """
        assert PyS2OPC._initialized_cli and not PyS2OPC._configured,\
            'Toolkit is not initialized or already configured, cannot add new configurations.'

        pCfgId = ffi.new('SOPC_LibSub_ConfigurationId *')
        dConnectionParameters = {'server_url': ffi.new('char[]', server_url.encode()),
                                 'security_policy': SecurityPolicy.PolicyNone,
                                 'security_mode': SecurityMode.ModeNone,
                                 'disable_certificate_verification': True,
                                 'path_cert_auth': NULL,
                                 'path_cert_srv': NULL,
                                 'path_cert_cli': NULL,
                                 'path_key_cli': NULL,
                                 'path_crl': NULL,
                                 'policyId': ffi.new('char[]', b"anonymous"),
                                 'username': NULL,
                                 'password': NULL,
                                 'publish_period_ms': publish_period,
                                 'n_max_keepalive': n_max_keepalive,
                                 'n_max_lifetime': n_max_lifetime,
                                 'data_change_callback': libsub._callback_datachanged,
                                 'timeout_ms': timeout_ms,
                                 'sc_lifetime': sc_lifetime,
                                 'token_target': token_target,
                                 'generic_response_callback': libsub._callback_client_event}
        status = libsub.SOPC_LibSub_ConfigureConnection([dConnectionParameters], pCfgId)
        assert status == ReturnStatus.OK, 'Configuration failed with status {}.'.format(ReturnStatus.get_both_from_id(status))

        cfgId = pCfgId[0]
        config = ClientConfiguration(cfgId, dConnectionParameters)
        PyS2OPC_Client._dConfigurations[cfgId] = config
        return config

    @staticmethod
    def add_configuration_secured(*,
                                  server_url = 'opc.tcp://localhost:4841',
                                  publish_period = 500,
                                  n_max_keepalive = 3,
                                  n_max_lifetime = 10,
                                  timeout_ms = 10000,
                                  sc_lifetime = 3600000,
                                  token_target = 3,
                                  security_mode = SecurityMode.Sign,
                                  security_policy = SecurityPolicy.Basic256,
                                  path_cert_auth = '../../../build/bin/trusted/cacert.der',
                                  path_crl = '../../../build/bin/revoked/cacrl.der',
                                  path_cert_srv = '../../../build/bin/server_public/server_2k_cert.der',
                                  path_cert_cli = '../../../build/bin/client_public/client_2k_cert.der',
                                  path_key_cli = '../../../build/bin/client_private/encrypted_client_2k_key.pem',
                                  client_key_encrypted = True,
                                  policy_id = 'anonymous',
                                  username = None,
                                  password = None):
        """
        Returns a configuration that can be later used in `PyS2OPC_Client.connect` or `PyS2OPC_Client.get_endpoints`.

        Args:
            server_url: The endpoint and server url to connect to.
            publish_period: The period of the subscription, in ms.
            n_max_keepalive: The number of times the subscription has no notification to send before
                             sending an empty PublishResponse (the KeepAlive message). It is necessary
                             to keep `n_max_keepalive*timeout_ms*token_target < 5000ms`.
            n_max_lifetime: The maximum number of times a subscription has notifications to send
                            but no available token. In this case, the subscription is destroyed.
            timeout_ms: The `PyS2OPC_Client.connect` timeout, in ms.
            sc_lifetime: The target lifetime of the secure channel, before renewal, in ms.
            token_target: The number of subscription tokens (PublishRequest) that should be
                          made available to the server at anytime.
            security_mode: The configured security mode, one of the `pys2opc.types.SecurityMode` constants.
            security_policy: The configured security policy, one of the `pys2opc.types.SecurityPolicy` constants.
            path_cert_auth: The path to the certificate authority (in DER or PEM format).
            path_crl      : The path to the CertificateRevocationList of the certificate authority (mandatory)
            path_cert_srv: The path to the expected server certificate (in DER or PEM format).
                           It must be signed by the certificate authority.
            path_cert_cli: The path to the certificate of the client.
            path_key_cli: The path to the private key of the client certificate.
            client_key_encrypted: Whether the client private key is encrypted or not. Password is asked interactively.
            policy_id: To know which policy id to use, When username is NULL, the AnonymousIdentityToken
                       is used and the policy id must correspond to an anonymous UserIdentityPolicy.
                       Otherwise, the UserNameIdentityToken is used and the policy id must correspond to
                       an username UserIdentityPolicy.
            username: None for anonymous access, see policyId the password will be encrypted,
                      or not, depending on the user token security policy associated to the policyId
                      or if it is empty depending on the SecureChannel security policy.
            password: The password is ignored when username is NULL.
        """
        _username = NULL
        if username:
            _username = ffi.new('char[]', username.encode())
        _password = NULL
        if password:
            _password = ffi.new('char[]', password.encode())

        # TODO: factorize code with add_configuration_unsecured
        assert PyS2OPC._initialized_cli and not PyS2OPC._configured,\
            'Toolkit is not initialized or already configured, cannot add new configurations.'

        pCfgId = ffi.new('SOPC_LibSub_ConfigurationId *')
        dConnectionParameters = {'server_url': ffi.new('char[]', server_url.encode()),
                                 'security_policy': security_policy,
                                 'security_mode': security_mode,
                                 'disable_certificate_verification': False,
                                 'path_cert_auth': ffi.new('char[]', path_cert_auth.encode()),
                                 'path_crl': ffi.new('char[]', path_crl.encode()),
                                 'path_cert_srv': ffi.new('char[]', path_cert_srv.encode()),
                                 'path_cert_cli': ffi.new('char[]', path_cert_cli.encode()),
                                 'path_key_cli': ffi.new('char[]', path_key_cli.encode()),
                                 'policyId': ffi.new('char[]', policy_id.encode()),
                                 'username': _username,
                                 'password': _password,
                                 'publish_period_ms': publish_period,
                                 'n_max_keepalive': n_max_keepalive,
                                 'n_max_lifetime': n_max_lifetime,
                                 'data_change_callback': libsub._callback_datachanged,
                                 'timeout_ms': timeout_ms,
                                 'sc_lifetime': sc_lifetime,
                                 'token_target': token_target,
                                 'generic_response_callback': libsub._callback_client_event}

        if client_key_encrypted:
            status = libsub.SOPC_HelperConfigClient_SetKeyPasswordCallback(libsub._callback_get_client_key_password)
            assert status == ReturnStatus.OK, 'Enable to configure the callback to retrieve the password for decryption of the client private key.'
        status = libsub.SOPC_LibSub_ConfigureConnection([dConnectionParameters], pCfgId)
        assert status == ReturnStatus.OK, 'Configuration failed with status {}.'.format(ReturnStatus.get_both_from_id(status))

        cfgId = pCfgId[0]
        config = ClientConfiguration(cfgId, dConnectionParameters)
        PyS2OPC_Client._dConfigurations[cfgId] = config
        return config

    @staticmethod
    def mark_configured():
        PyS2OPC.mark_configured()
        try:
            assert libsub.SOPC_LibSub_Configured() == ReturnStatus.OK
        except:
            PyS2OPC._configured = False
            raise

    @staticmethod
    def get_endpoints(configuration):
        """
        Optional call to fetch the endpoints of the server through the configuration.

        Not implemented yet.
        """
        assert isinstance(configuration, ClientConfiguration)
        raise NotImplementedError()


    @staticmethod
    def connect(configuration, ConnectionHandlerClass):
        """
        Connects to the server with the given `configuration`.
        The configuration is returned by a call to add_configuration_unsecured().
        The ConnectionHandlerClass is a class that must be inherited from BaseClientConnectionHandler,
        and that at least overrides the callbacks.
        It will be instantiated and the instance is returned.

        It can be optionally used in a `with` statement, which automatically disconnects the connection.
        """
        assert PyS2OPC._initialized_cli and PyS2OPC._configured, 'Toolkit not configured, cannot connect().'
        assert isinstance(configuration, ClientConfiguration)
        cfgId = configuration._id
        assert cfgId in PyS2OPC_Client._dConfigurations, 'Unknown configuration, see add_configuration_unsecured().'
        assert issubclass(ConnectionHandlerClass, BaseClientConnectionHandler)

        pConId = ffi.new('SOPC_LibSub_DataId *')
        status = libsub.SOPC_LibSub_Connect(cfgId, pConId)
        if status != ReturnStatus.OK:
            raise ConnectionError('Could not connect to the server with the given configuration with status {}.'.format(ReturnStatus.get_name_from_id(status)))

        connectionId = pConId[0]
        assert connectionId not in PyS2OPC_Client._dConnections,\
            'Subscription library returned a connection id that is already taken by an active connection.'

        connection = ConnectionHandlerClass(connectionId, configuration)
        PyS2OPC_Client._dConnections[connectionId] = connection
        return connection

    @staticmethod
    def _callback_disconnected(clientId):
        if clientId not in PyS2OPC_Client._dConnections:
            print('# Warning: Disconnected unknown client', clientId, file=sys.stderr)
            return
        connection = PyS2OPC_Client._dConnections.pop(clientId)
        connection._on_disconnect()

    @staticmethod
    def _callback_datachanged(connectionId, dataId, c_value):
        value = DataValue.from_sopc_libsub_value(c_value)
        #print('Data changed (connection {}, data_id {}), new value {}'.format(connection_id, data_id, value.value))
        assert connectionId in PyS2OPC_Client._dConnections, 'Data change notification on unknown connection'
        connection = PyS2OPC_Client._dConnections[connectionId]
        connection._on_datachanged(dataId, value)

    @staticmethod
    def _callback_client_event(connectionId, event, status, responsePayload, responseContext, timestamp):
        assert connectionId in PyS2OPC_Client._dConnections, 'Event notification on unknown connection'
        connection = PyS2OPC_Client._dConnections[connectionId]
        # It is not possible to store the payload, as it is freed by the caller of the callback.
        connection._on_response(event, status, responsePayload, responseContext, timestamp)

    @staticmethod
    def get_client_key_password():
        """
        Default method that is called during configuration phase if an encrypted private key is used, 
        it shall return the password to decrypt the client private key.
        It uses the `PyS2OPC._get_password` which uses get_pass library.
        """
        return PyS2OPC._get_password("Client private key password:")

class PyS2OPC_Server(PyS2OPC):
    """
    The Server side of the PyS2OPC library.
    When the toolkit is `PyS2OPC_Server.initialize`d for a server, it cannot be `PyS2OPC_Client.initialize`d for a client before it is `PyS2OPC_Server.clear`ed.
    """

    # Tuple to manage the user authentications and authorizations
    XMLUserManager = namedtuple('XMLUserManager', ['authentication', 'authorization'])

    _adds_handler = None  # Instance of BaseAddressSpaceHandler
    _adds = None  # The address space loaded through the xml loader
    _config = None  # SOPC_S2OPC_Config
    # Stores endpoint indexes and the corresponding configurations {Id: config.serverConfig.endpoint}
    # (note that endpoint.serverConfigPtr points to its parent serverConfig, which is the only field in config yet)
    _dEpIdx = {}  # {endpoint_index: SOPC_Endpoint_Config}
    _dOpenedEp = {}  # Opened endpoint {endpoint_index: True}
    _req_hdler = LocalAsyncRequestHandler()
    _xml_user_manager = None # XML user authentications and authorizations
    _custom_user_handler = None # User defined user authentications and authorizations functions (instance of BaseUserHandler)

    @staticmethod
    @contextmanager
    def initialize(logLevel=LogLevel.Debug, logPath='logs/', logFileMaxBytes=1048576, logMaxFileNumber=50):
        """
        Toolkit initialization for Server.
        When the toolkit is initialized for servers, it cannot be used to make a server before a call to `PyS2OPC_Server.clear`.

        This function supports the context management:
        >>> with PyS2OPC_Server.initialize():
        ...     # Do things here, namely configure then wait
        ...     pass

        When reaching out of the `with` statement, the Toolkit is automatically cleared.
        See `PyS2OPC_Server.clear`.

        Args:
            logLevel: log level (0: error, 1: warning, 2: info, 3: debug)
            logPath: the path for logs (the current working directory) to logPath.
                     logPath is created if it does not exist.
            logFileMAxBytes: The maximum size (best effort) of the log files, before changing the log index.
            logMaxFileNumber: The maximum number of log indexes before cycling logs and reusing the first log.

        """
        PyS2OPC._assert_not_init()

        logConfig = libsub.SOPC_Common_GetDefaultLogConfiguration()
        logConfig.logLevel = logLevel
        # Note: we don't keep a copy of logDirPath as the string content copied internally in SOPC_Log_CreateInstance
        #  but we must keep it alive until then
        logDirPath = ffi.new('char[]', logPath.encode())
        logConfig.logSysConfig.fileSystemLogConfig.logDirPath = logDirPath
        logConfig.logSysConfig.fileSystemLogConfig.logMaxBytes = logFileMaxBytes
        logConfig.logSysConfig.fileSystemLogConfig.logMaxFiles = logMaxFileNumber

        status = libsub.SOPC_Common_Initialize(logConfig)
        assert status == ReturnStatus.OK, 'Common initialization failed with status {}'.format(ReturnStatus.get_both_from_id(status))
        status = libsub.SOPC_Toolkit_Initialize(libsub._callback_toolkit_event)
        assert status == ReturnStatus.OK, 'Toolkit initialization failed with status {}.'.format(ReturnStatus.get_both_from_id(status))
        PyS2OPC._initialized_srv = True

        try:
            yield
        finally:
            PyS2OPC_Server.clear()

    @staticmethod
    def clear():
        """
        Disconnect current servers and clears the Toolkit.
        Existing `ServerConfiguration`s are then invalid and should be freed.
        """
        # TODO: Disconnect existing clients
        libsub.SOPC_Toolkit_Clear()  # Calls SOPC_Common_Clear
        PyS2OPC._initialized_srv = False
        PyS2OPC._configured = False
        PyS2OPC_Server._adds_handler = None
        if PyS2OPC_Server._adds is not None:
            libsub.SOPC_AddressSpace_Delete(PyS2OPC_Server._adds)
            PyS2OPC_Server._adds = None
        PyS2OPC_Server._dEpIdx = {}
        if PyS2OPC_Server._config is not None:
            # Avoid double free during SOPC_EndpointConfig_Clear in case of user XML configuration
            # Indeed, we use a shared authenticationManager and authorizationManager for all the endpoints (same address)
            serverCfg = PyS2OPC_Server._config.serverConfig
            for i in range(serverCfg.nbEndpoints):
                endpoint = serverCfg.endpoints[i]
                # Free only the first authenticationManager and authorizationManager
                # Avoid the others by seting to NULL (avoid the double free)
                if PyS2OPC_Server._xml_user_manager is not None and i > 0:
                    endpoint.authenticationManager = NULL
                    endpoint.authorizationManager = NULL
            libsub.SOPC_S2OPC_Config_Clear(PyS2OPC_Server._config)
            PyS2OPC_Server._config = None
            PyS2OPC_Server._xml_user_manager = None
            PyS2OPC_Server._custom_user_manager = None

    @staticmethod
    def _callback_toolkit_event(event, epIdx, param, appContext, timestamp):
        # For now, only support server events
        if event == libsub.SE_CLOSED_ENDPOINT:
            # id = endpoint configuration index,
            # auxParam = SOPC_ReturnStatus
            print(epIdx, 'closed')
            PyS2OPC_Server._dOpenedEp[epIdx] = False
        elif event == libsub.SE_LOCAL_SERVICE_RESPONSE:
            # id = endpoint configuration index,
            # param = (OpcUa_<MessageStruct>*) OPC UA message header + payload structure
            #         (deallocated by toolkit after callback call ends)
            # auxParam = user application request context
            PyS2OPC_Server._req_hdler._on_response(param, appContext, timestamp)

    @staticmethod
    def _callback_address_space_event(ctx, event, operationParam, operationStatus):
        assert PyS2OPC_Server._adds_handler is not None
        PyS2OPC_Server._adds_handler._on_datachanged(ctx, event, operationParam, operationStatus)

    @staticmethod
    def _callback_validate_user_identity(authenticationManager, pUser, pUserAuthenticated):
        return PyS2OPC_Server._custom_user_handler._on_user_authentication(authenticationManager, pUser, pUserAuthenticated)

    @staticmethod
    def _callback_authorize_operation(authorizationManager, operationType, nodeId, attributeId, pUser, pbOperationAuthorized):
        return PyS2OPC_Server._custom_user_handler._on_user_authorisation(authorizationManager, operationType, nodeId, attributeId, pUser, pbOperationAuthorized)

    @staticmethod
    def load_address_space(xml_path):
        """
        Loads an address space from the XML file `xml_path`.
        This must be done after `PyS2OPC_Server.initialize`, and before `PyS2OPC_Server.mark_configured`.

        Note: only one address space can be loaded. Once an address space is loaded, this function is not callable anymore.

        Args:
            xml_path: Path to an Address Space in the format specified by the OPC Foundation
                      (see also http://opcfoundation.org/UA/2011/03/UANodeSet.xsd).
        """
        assert PyS2OPC._initialized_srv and not PyS2OPC._configured,\
            'Toolkit is either not initialized, initialized as a Client, or already marked_configured.'
        assert PyS2OPC_Server._adds is None, 'Only one address space can be loaded.'

        with open(xml_path, 'r') as fd:
            space = libsub.SOPC_UANodeSet_Parse(fd)
        assert space != NULL,\
            'Cannot load address space from file {}'.format(xml_path)
        assert libsub.SOPC_ToolkitServer_SetAddressSpaceConfig(space) == ReturnStatus.OK
        PyS2OPC_Server._adds = space  # Kept to avoid double inits, and to clear it

    @staticmethod
    def load_users(xml_path):
        """
        Loads configuration for the user authentications and authorizations from the XML file `xml_path`.
        This must be done after `PyS2OPC_Server.initialize`, and before `PyS2OPC_Server.mark_configured` and \
        `PyS2OPC_Server.load_configuration`.

        Args:
            xml_path: Path to XML file compliant with s2opc_clientserver_users_config.xsd schema.
        """

        assert PyS2OPC._initialized_srv and not PyS2OPC._configured,\
            'Toolkit is either not initialized, initialized as a Client, or already marked_configured.'
        assert PyS2OPC_Server._config is None, 'Server is already configure'
        assert PyS2OPC_Server._xml_user_manager is None, 'user authentications and authorizations are already configure'

        # Creates the user authentication manager
        ppAuthenticationManager = ffi.new('SOPC_UserAuthentication_Manager **')
        # Creates the user authorization manager
        ppAuthorizationManager = ffi.new('SOPC_UserAuthorization_Manager **')
        with open(xml_path, 'r') as fd:
            res = libsub.SOPC_UsersConfig_Parse(fd, ppAuthenticationManager, ppAuthorizationManager)
            assert res, 'Cannot load user configuration from file {}'.format(xml_path)
        pAuthenticationManager = ppAuthenticationManager[0]
        pAuthorizationManager = ppAuthorizationManager[0]
        PyS2OPC_Server._xml_user_manager = PyS2OPC_Server.XMLUserManager(pAuthenticationManager, pAuthorizationManager)

    @staticmethod
    def load_configuration(xml_path, address_space_handler=None, custom_user_handler=None, method_handler=None, pki_handler=None):
        """
        Creates a configuration structure for a server from an XML file.
        This configuration is later used to open an endpoint.
        There should be only one created configuration.

        The XML configuration format is specific to S2OPC and follows the s2opc_config.xsd scheme.
        If the server private key is encrypted then the password is asked interactively.

        Optionally configure the callbacks of the server.
        If handlers are left None, the following default behaviors are used:

        - address space: no notification of address space events,
        - user authentications and authorizations: allow all user and all operations if `PyS2OPC_Server.load_users` is not called,
        - methods: no callable methods,
        - pki: the default secure Public Key Infrastructure,
          which thoroughly checks the validity of certificates based on trusted issuers, untrusted issuers, and issued certificates.

        This function must be called after `PyS2OPC_Server.initialize`, and before `PyS2OPC_Server.mark_configured`.
        It must be called at most once.

        Note: limitation: for now, changes in user authentications and authorizations, methods, and pki, are not supported.

        Args:
            xml_path: Path to the configuration in the s2opc_config.xsd format
            address_space_handler: None (no notification) or an instance of a subclass of
                                   `pys2opc.server_callbacks.BaseAddressSpaceHandler`
            custom_user_handler: None (authenticate all user and authorize all operations if `PyS2OPC_Server.load_users` is not called)
            method_handler: None (no method available)
            pki_handler: None (certificate authentications based on certificate authorities)
        """
        assert PyS2OPC._initialized_srv and not PyS2OPC._configured and PyS2OPC_Server._config is None,\
            'Toolkit is either not initialized, initialized as a Client, or already configured.'

        assert custom_user_handler is None or PyS2OPC_Server._xml_user_manager is None, 'User handler cannot be defined both (XML and custom callbacks)'
        assert custom_user_handler is None, 'Custom User Manager not implemented yet'
        assert method_handler is None, 'Custom Method Manager not implemented yet'
        assert pki_handler is None, 'Custom PKI Manager not implemented yet'
        if address_space_handler is not None:
            assert isinstance(address_space_handler, BaseAddressSpaceHandler)

        # Note: if part of the configuration fails, this leaves the toolkit in an half-configured configuration.
        # In this case, a Clear is required before trying to configure it again.

        # Creates the configuration
        config = ffi.new('SOPC_S2OPC_Config *')
        with open(xml_path, 'r') as fd:
            assert libsub.SOPC_Config_Parse(fd, config)

        # Finish the configuration by setting the manual fields: server certificate and key, create the pki,
        #  the user auth* managers, and the method call manager
        # If any of them fails, we must still clear the config!
        try:
            # Cryptography
            serverCfg = config.serverConfig
            if serverCfg.serverCertPath != NULL or serverCfg.serverKeyPath != NULL:
                assert serverCfg.serverCertPath != NULL and serverCfg.serverKeyPath != NULL,\
                    'The server private key and server certificate work by pair. Either configure them both of them or none of them.'
                ppCert = ffi.addressof(serverCfg, 'serverCertificate')
                status = libsub.SOPC_KeyManager_SerializedCertificate_CreateFromFile(serverCfg.serverCertPath, ppCert)
                assert status == ReturnStatus.OK,\
                    'Cannot load server certificate file {} with status {}. Is path correct?'\
                    .format(ffi.string(serverCfg.serverCertPath), ReturnStatus.get_both_from_id(status))

                ppKey = ffi.addressof(serverCfg, 'serverKey')

                password = NULL
                lenPassword = 0
                # Retrieve the password if the key is encrypted
                if serverCfg.serverKeyEncrypted:
                    password = PyS2OPC_Server.get_server_key_password()
                    password = password.encode() + b'\0' # Add protection to avoid buffer overrun with C code
                    password = ffi.new('char[]', password)
                    lenPassword = len(password)
                status = libsub.SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd(serverCfg.serverKeyPath, ppKey, password, lenPassword)
                assert status == ReturnStatus.OK,\
                    'Cannot load secret key file {} with status {}. Is path correct?'\
                    .format(ffi.string(serverCfg.serverKeyPath), ReturnStatus.get_both_from_id(status))

            # PKI is not required if no CA is configured
            if (serverCfg.trustedRootIssuersList != NULL and serverCfg.trustedRootIssuersList[0] != NULL) or\
               (serverCfg.issuedCertificatesList != NULL and serverCfg.issuedCertificatesList[0] != NULL):
                ppPki = ffi.addressof(serverCfg, 'pki')
                status = libsub.SOPC_PKIProviderStack_CreateFromPaths(
                    serverCfg.trustedRootIssuersList, serverCfg.trustedIntermediateIssuersList,
                    serverCfg.untrustedRootIssuersList, serverCfg.untrustedIntermediateIssuersList,
                    serverCfg.issuedCertificatesList, serverCfg.certificateRevocationPathList,
                    ppPki)

            # Methods
            serverCfg.mcm  # Leave NULL

            # Endpoints have the user management
            for i in range(serverCfg.nbEndpoints):
                endpoint = serverCfg.endpoints[i]
                if PyS2OPC_Server._xml_user_manager is None and custom_user_handler is None:
                    # By default, creates user managers that accept all users and allow all operations
                    endpoint.authenticationManager = libsub.SOPC_UserAuthentication_CreateManager_AllowAll()
                    endpoint.authorizationManager = libsub.SOPC_UserAuthorization_CreateManager_AllowAll()
                elif PyS2OPC_Server._xml_user_manager is not None :
                    # Use the user authentications and authorizations configured with the user XML file
                    endpoint.authenticationManager = PyS2OPC_Server._xml_user_manager.authentication
                    endpoint.authorizationManager = PyS2OPC_Server._xml_user_manager.authorization
                else:
                    # TODO: For each endpoints create the 2 managers using _callback_validate_user_identity and _callback_authorize_operation
                    # Note: BaseUserHandler implem also to be done to have Python API
                    assert False
                assert endpoint.authenticationManager != NULL and endpoint.authorizationManager != NULL

                # Register endpoint
                epConfigIdx = libsub.SOPC_ToolkitServer_AddEndpointConfig(ffi.addressof(endpoint))
                assert epConfigIdx,\
                    'Cannot add endpoint configuration. There may be no more endpoint left, or the configuration parameters are incorrect.'
                assert epConfigIdx not in PyS2OPC_Server._dEpIdx,\
                    'Internal failure, epConfigIdx already reserved by another configuration.'
                PyS2OPC_Server._dEpIdx[epConfigIdx] = endpoint
        except:
            libsub.SOPC_S2OPC_Config_Clear(config)
            config = None
            raise
        PyS2OPC_Server._config = config

        # Set address space handler
        if address_space_handler is not None:
            PyS2OPC_Server._adds_handler = address_space_handler
            # Note: SetAddressSpaceNotifCb cannot be called twice, or with NULL
            assert libsub.SOPC_ToolkitServer_SetAddressSpaceNotifCb(libsub._callback_address_space_event) == ReturnStatus.OK

    @staticmethod
    def load_server_configuration_from_files(xml_address_space_config_path, xml_users_config_path, xml_server_config_path, address_space_handler=None):
        """
        Configure server from XML configuration files for: server endpoints, address space and users credential and rights.
        This function must be called after `PyS2OPC_Server.initialize`, and before `PyS2OPC_Server.mark_configured`.

        Args:
            xml_address_space_config_path: path to address space configuration XML file (UANodeSet.xsd schema)
            xml_users_config_path: path to users credential and rights configuration XML file (s2opc_clientserver_users_config.xsd schema)
            xml_server_config_path: path to server configuration XML file (s2opc_clientserver_config.xsd schema)
            address_space_handler: None (no notification) or an instance of a subclass of `pys2opc.server_callbacks.BaseAddressSpaceHandler`
        """
        PyS2OPC_Server.load_address_space(xml_address_space_config_path)
        PyS2OPC_Server.load_users(xml_users_config_path)
        PyS2OPC_Server.load_configuration(xml_server_config_path,
                                          address_space_handler=address_space_handler,
                                          custom_user_handler=None,
                                          method_handler=None,
                                          pki_handler=None)

    @staticmethod
    def mark_configured():
        PyS2OPC.mark_configured()
        try:
            assert libsub.SOPC_ToolkitServer_Configured() == ReturnStatus.OK
        except:
            PyS2OPC._configured = False
            raise

    @staticmethod
    def serving(endpointIndexes=None):
        """
        Returns true if at least one of the enpoints is opened.
        Use endpointIndexes to restrict the query to the selected endpoints.
        """
        if endpointIndexes is None:
            return any(PyS2OPC_Server._dOpenedEp.values())
        return any(opened for epIdx, opened in PyS2OPC_Server._dOpenedEp.items() if epIdx in endpointIndexes)

    @staticmethod
    @contextmanager
    def serve(endpointIndexes=None):
        """
        Open the configured endpoint(s).
        If no endpoints are given, all configured endpoints are opened.
        Use serving(endpointIndexes) to know if the server is still opened.

        Supports the context management facilities to close the endpoint(s):
        >>> with PyS2OPC_Server.serve():
        ...     while PyS2OPC_Server.serving():
        ...         # All applicative code may live here
        ...         pass
        ... # Endpoint and server capabilities are cleanly stopped upon context exit

        If you don't have applicative application, and callbacks are enough,
        see instead `PyS2OPC_Server.serve_forever()`.
        """
        for epIdx in (endpointIndexes or PyS2OPC_Server._dEpIdx):
            libsub.SOPC_ToolkitServer_AsyncOpenEndpoint(epIdx)
            PyS2OPC_Server._dOpenedEp[epIdx] = True
        print('Opening server')
        try:
            yield
        finally:
            for epIdx in (endpointIndexes or PyS2OPC_Server._dEpIdx):
                if PyS2OPC_Server._dOpenedEp[epIdx]:
                    libsub.SOPC_ToolkitServer_AsyncCloseEndpoint(epIdx)
        print('Server stopped')

    @staticmethod
    def serve_forever(endpointIndexes=None):
        def sigterm_handler(_signo, _stack_frame):
            raise KeyboardInterrupt

        signal.signal(signal.SIGTERM, sigterm_handler)

        """
        Open and serve endpoint(s) forever.
        Can be interrupted with a `KeyboardInterrupt` (`SIGINT`).
        """
        with PyS2OPC_Server.serve(endpointIndexes=endpointIndexes):
            try:
                while PyS2OPC_Server.serving(endpointIndexes):
                    time.sleep(.1)  # Sleepy wait
                print('No more opened endpoint -> closing server')
            except KeyboardInterrupt:
                pass

    @staticmethod
    def get_server_key_password():
        """
        Default method that is called during configuration phase if an encrypted private key is used, 
        it shall return the password to decrypt the server private key.
        It uses the `PyS2OPC._get_password` which uses get_pass library.
        """
        return PyS2OPC._get_password("Server private key password:")
            
    # -----------------------------
    # Local services implementation

    @staticmethod
    def _send_request(request, bWaitResponse, epIdx):
        if epIdx is None:
            for epIdx in PyS2OPC_Server._dEpIdx:
                break
        assert epIdx is not None, 'No configured endpoint'
        return PyS2OPC_Server._req_hdler.send_generic_request(epIdx, request, bWaitResponse)

    @staticmethod
    def read_nodes(nodeIds, attributes=None, bWaitResponse=True, epIdx=None):
        """
        Forges an `OpcUa_ReadRequest` and sends it as a local request.
        `epIdx` is the local endpoint index to send this request to.
        If `None`, this function chooses an endpoint.

        See `pys2opc.connection.BaseConnectionHandler.read_nodes` for more details.
        """
        request = Request.new_read_request(nodeIds, attributes=attributes)
        return PyS2OPC_Server._send_request(request, bWaitResponse, epIdx)

    @staticmethod
    def write_nodes(nodeIds, datavalues, attributes=None, types=None, bWaitResponse=True, bAutoTypeWithRead=True, epIdx=None):
        """
        Forges an `OpcUa_WriteRequest` and sends it as a local request.
        `epIdx` is the local endpoint index to send this request to.
        If `None`, this function chooses an endpoint.

        See `pys2opc.connection.BaseConnectionHandler.write_nodes` for more details.
        """
        # Where there are unknown types, makes a read request first
        if bAutoTypeWithRead:
            sendFct = lambda request,**kwargs: PyS2OPC_Server._send_request(request, epIdx=epIdx, **kwargs)
            types = Request.helper_maybe_read_types(nodeIds, datavalues, attributes, types, sendFct)

        # Make the actual write request
        request = Request.new_write_request(nodeIds, datavalues, attributes=attributes, types=types)
        return PyS2OPC_Server._send_request(request, bWaitResponse, epIdx)

    def browse_nodes(self, nodeIds, maxReferencesPerNode=1000, bWaitResponse=True, epIdx=None):
        """
        Forges an `OpcUa_BrowseRequest` and sends it as a local request.
        `epIdx` is the local endpoint index to send this request to.
        If `None`, this function chooses an endpoint.

        See `pys2opc.connection.BaseConnectionHandler.browse_nodes` for more details.
        """
        request = Request.new_browse_request(nodeIds, maxReferencesPerNode=maxReferencesPerNode)
        return PyS2OPC_Server._send_request(request, bWaitResponse, epIdx)


class Configuration:
    """
    Stores configuration given to the subscription library.
    Such configurations should not be modified, as modifying these will have no impact on the Toolkit.
    """
    def __init__(self, id, parameters):
        self._id = id
        self._parameters = parameters

    @property
    def parameters(self):
        """
        Returns a copy of the parameters given to the configuration.
        Modifying these will have no effect on the configuration whatsoever.
        """
        return self._parameters.copy()

class ClientConfiguration(Configuration):
    pass

class ServerConfiguration(Configuration):
    pass
