/**
 * S2OPC Client module
 *
 * High level javascript module for the S2OPC C client library
 *
 * For restrictions, see restrictions of the C client library
 * @module sopc_client
 * @see module:datavalue
 * @see module:variant
 */
const ffi = require('ffi');
const ref = require('ref');
const Enum = require('enum');
const process = require('process');
const fs = require('fs');

const bind = require('./bind_sopc_client')
const variant = require('./variant');
const data_value = require('./datavalue');
const write_value = require('./writevalue');
const read_value = require('./readvalue');
const browse_request = require('./browse_request');
const browse_result = require('./browse_result');
const browse_result_reference = require('./browse_result_reference');

/** Log levels */
const SOPC_Toolkit_Log_Level = new Enum({
    'Error': 0,
    'Warning': 1,
    'Info': 2,
    'Debug': 3
});

/** Security modes */
const SOPC_MessageSecurityMode = new Enum({
    'None' : 1,
    'Sign' : 2,
    'SignAndEncrypt' : 3
});

/** Security policies */
const SOPC_SecurityPolicy = {
    'None_URI' : "http://opcfoundation.org/UA/SecurityPolicy#None",
    'Basic256_URI' : "http://opcfoundation.org/UA/SecurityPolicy#Basic256",
    'Basic256Sha256_URI' : "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"
};

/**
 * Create a C security configuration
 * @param {String} security_policy
 * @param {Number} security_mode
 * @param {String} user_policy_id
 * @param {String} path_cert_auth
 * @param {String} path_cert_srv
 * @param {String} path_cert_cli
 * @param {String} path_key_cli
 * @param {String} user_name
 * @param {String} user_password
 */
function SecurityCfg(security_policy, security_mode, user_policy_id,
                     path_cert_auth = ref.NULL, path_cert_srv = ref.NULL,
                     path_cert_cli = ref.NULL, path_key_cli = ref.NULL,
                     user_name = ref.NULL, user_password = ref.NULL)
{
    if (Object.values(SOPC_SecurityPolicy).indexOf(security_policy) < 0) {
        throw `Invalid security_policy (${security_policy}).`;
    }

    if (user_policy_id !== "anonymous" && user_policy_id !== "username") {
        throw `user_policy_id value (${user_policy_id}) is not correct("anonymous" or "username").`;
    }

    if (user_policy_id === "username" && (user_name === ref.NULL || user_password === ref.NULL)) {
        throw `user_name and user_password shall be specified when using username policy.`;
    }

    if (user_policy_id === "anonymous" && (user_name !== ref.NULL || user_password !== ref.NULL)) {
        throw `user_name and user_password shall not be specified when using anonymous policy.`;
    }

    let fileList = [path_cert_auth, path_cert_srv, path_cert_cli, path_key_cli];
    if (security_mode === SOPC_MessageSecurityMode.Sign || security_mode === SOPC_MessageSecurityMode.SignAndEncrypt) {
        for (let file of fileList) {
            if (file === ref.NULL) {
                throw "All certificates and keys shall be specified when using Sign or SignAndEncrypt.";
            }
            else if (!fs.existsSync(file)) {
                throw `${file} is specified but does not exist.`;
            }
        }
    }

    let securityCfg = bind.security_cfg({
        security_policy : security_policy,
        security_mode : security_mode.value,
        path_cert_auth : path_cert_auth,
        path_cert_srv : path_cert_srv,
        path_cert_cli : path_cert_cli,
        path_key_cli : path_key_cli,
        policyId : user_policy_id,
        username : user_name,
        password : user_password
    });

    return securityCfg;
}

const SOPC_DataValuePtr = ref.refType(bind.SOPC_DataValue);

/**
 * Initialize the S2OPC client library
 * @param {String} toolkit_log_path path of the log directory (should end with a directory separator)
 * @param {SOPC_Toolkit_Log_Level} toolkit_log_level log level
 * @returns 0 if OK else C error code
 */
function initialize(toolkit_log_path, toolkit_log_level){
    return bind.sopc_client.SOPC_ClientHelper_Initialize(toolkit_log_path, toolkit_log_level.value);
}

/**
 * Close the S2OPC toolkit
 * @returns 0 if OK else C error code
 */
function finalize(){
    return bind.sopc_client.SOPC_ClientHelper_Finalize();
}

/**
 * Connect to an endpoint using given security configuration
 * @param {String} endpoint_url url of the endpoint
 * @param {SecurityCfg} security security configuration
 * @returns 0 if OK else C error code
 */
function connect(endpoint_url, security){
    return bind.sopc_client.SOPC_ClientHelper_Connect(endpoint_url,
                                                      security);
}

/**
 * Create a subscription
 * @param {Number} connectionId id of the connection
 * @param {Function} user_callback callback that will be callback when a datachange is received
 * @returns 0 if OK else C error code
 */
function createSubscription(connectionId, user_callback)
{
    let ffiCallback = ffi.Callback('void', ['int32', 'CString', SOPC_DataValuePtr],
                                   function(connectionId, nodeId, value) {
                                       let dereferenced_value = value.deref();
                                       let dv = new data_value.DataValue()
                                                                      .FromC(dereferenced_value);
                                       user_callback(connectionId, nodeId, dv);
                                   });

    // Make an extra reference to the callback pointer to avoid GC
    process.on('exit', function() {
        ffiCallback
    });
    // Make an extra reference to the callback pointer to avoid GC
    process.on('exit', function() {
        user_callback
    });

    return bind.sopc_client.SOPC_ClientHelper_CreateSubscription(connectionId, ffiCallback);
}

/**
 * Add monitored items
 * @param {Number} connectionId id of the connection
 * @param {String[]} nodeIdArray node ids of items to monitor
 * @returns 0 if OK else C error code
 */
function addMonitoredItems(connectionId, nodeIdArray){
    return bind.sopc_client.SOPC_ClientHelper_AddMonitoredItems(connectionId, nodeIdArray, nodeIdArray.length);
}

/**
 * Write values to attributes "Value" of one or more Nodes
 * @param {Number} connectionId id of the connection
 * @param {WriteValue[]} writeValueArray array of write values
 * @see module:writevalue
 * @return array containing two elements: the status code of the write response,
 *         and an array containing the status codes of each write request.
 */
function write(connectionId, writeValueArray)
{
    let writeValueArrayC = [];
    for (let elt of writeValueArray)
    {
        writeValueArrayC.push(elt.ToC());
    }
    let status_codes = new bind.UInt32Array(writeValueArrayC.length);
    let status = bind.sopc_client.SOPC_ClientHelper_Write(connectionId,
                                                          writeValueArrayC,
                                                          writeValueArrayC.length,
                                                          status_codes);
    return [status, status_codes.toArray()];
}

/**
 * Read attributes of one or more nodes
 * @param {Number} connectionId id of the connection
 * @param {ReadValue[]} readValuesArray array of read values
 * @see module:readvalue
 * @return array containing two elements: the status code of the read response,
 *         and an array containing the data value of each read request.
 * @see module:datavalue
 */
function read(connectionId, readValuesArray) {
    let readValuesArrayC = [];
    for (let elt of readValuesArray) {
        readValuesArrayC.push(elt.ToC());
    }
    let dataValuesPtrArray = new bind.SOPC_DataValuePtrArray(readValuesArrayC.length);

    let status = bind.sopc_client.SOPC_ClientHelper_Read(connectionId,
                                                         readValuesArrayC,
                                                         readValuesArrayC.length,
                                                         dataValuesPtrArray);

    let resultDataValues = [];
    if (status === 0) {
        for (let i = 0; i < readValuesArrayC.length; i++) {
            let dv = new data_value.DataValue()
                .FromC(dataValuesPtrArray[i].deref());
            resultDataValues.push(dv);
        }
    }
    return [status, resultDataValues];
}

/**
 * Discover the references of a node using Browse and browseNext services
 * @param {Number} connectionId id of the connection
 * @param {BrowseRequest[]} browseRequests array of browse request
 * @see module:browse_request
 * @return array containing two elements: the status code of the browse response,
 *         and an array containing the browse result of each browse request.
 * @see module:browse_result
 * @see module:browse_result_reference
 */
function browse(connectionId, browseRequests) {
    let browseRequestsC = [];
    for(let elt of browseRequests) {
        browseRequestsC.push(elt.ToC());
    }
    let browseResultsC = new bind.SOPC_ClientHelper_BrowseResultArray(browseRequestsC.length);
    let status = bind.sopc_client.SOPC_ClientHelper_Browse(connectionId,
                                                           browseRequestsC,
                                                           browseRequestsC.length,
                                                           browseResultsC);
    let browseResults = [];
    for (let i = 0; i < browseResultsC.length; i++) {
        browseResults.push(new browse_result.BrowseResult()
                                            .FromC(browseResultsC[i]));
    }
    return [status, browseResults];
}

/**
 * Delete the current subscription
 * @param {Number} connectionId id of the connection
 * @return 0 if OK else C error code
 */
function unsubscribe(connectionId) {
    return bind.sopc_client.SOPC_ClientHelper_Unsubscribe(connectionId);
}

/**
 * Disconnect the given connection
 * @param {Number} connectionId id of the connection
 * @return 0 if OK else C error code
 */
function disconnect(connectionId){
    return bind.sopc_client.SOPC_ClientHelper_Disconnect(connectionId);
}

module.exports = {
    log_level : SOPC_Toolkit_Log_Level,
    security_mode : SOPC_MessageSecurityMode,
    security_policy : SOPC_SecurityPolicy,
    SecurityCfg,
    initialize,
    finalize,
    connect,
    disconnect,
    addMonitoredItems,
    createSubscription,
    unsubscribe,
    Variant : variant.Variant,
    DataValue : data_value.DataValue,
    write,
    WriteValue : write_value.WriteValue,
    read,
    ReadValue : read_value.ReadValue,
    browse,
    BrowseRequest : browse_request.BrowseRequest,
    BrowseResult : browse_result.BrowseResult,
    BrowseResultReference : browse_result_reference.BrowseResultReference
};
