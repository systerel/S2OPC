const bind = require('./bind_sopc_client')
const ffi = require('ffi');
const ref = require('ref');
const Enum = require('enum');
const process = require('process');
const data_value = require('./datavalue');
const write_value = require('./writevalue');
const variant = require('./variant');


const SOPC_Toolkit_Log_Level = new Enum({
    'Error': 0,
    'Warning': 1,
    'Info': 2,
    'Debug': 3
});

const SOPC_MessageSecurityMode = new Enum({
    'None' : 1,
    'Sign' : 2,
    'SignAndEncrypt' : 3
});

const SOPC_SecurityPolicy = {
    'None_URI' : "http://opcfoundation.org/UA/SecurityPolicy#None",
    'Basic256_URI' : "http://opcfoundation.org/UA/SecurityPolicy#Basic256",
    'Basic256Sha256_URI' : "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"
};

function SecurityCfg(security_policy, security_mode, user_policy_id,
                     path_cert_auth=ref.NULL, path_cert_srv = ref.NULL,
                     path_cert_cli = ref.NULL, path_key_cli = ref.NULL,
                     user_name = ref.NULL, user_password = ref.NULL)
{
    var securityCfg = bind.security_cfg({
        security_policy : security_policy,
        security_mode : security_mode.value,
        path_cert_auth : path_cert_auth,
        path_cert_srv : path_cert_srv,
        path_cert_cli : path_cert_cli,
        path_key_cli : path_key_cli,
        policyId : user_policy_id,
        user_name : user_name,
        user_password : user_password
    })

    return securityCfg;
}

var SOPC_DataValuePtr = ref.refType(bind.SOPC_DataValue);

function initialize(toolkit_log_path, toolkit_log_level){
    return (0 == bind.sopc_client.SOPC_ClientHelper_Initialize(toolkit_log_path, toolkit_log_level.value));
}

function finalize(){
    return bind.sopc_client.SOPC_ClientHelper_Finalize();
}

function connect(endpoint_url, security){
    return bind.sopc_client.SOPC_ClientHelper_Connect(endpoint_url,
                                                      security);
}

function createSubscription(connectionId, user_callback)
{
    var ffiCallback = ffi.Callback('void', ['int32', 'CString', SOPC_DataValuePtr],
                                   function(connectionId, nodeId, value) {
                                       var dereferenced_value = value.deref();
                                       var dv = new data_value.DataValue()
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


    return (0 == bind.sopc_client.SOPC_ClientHelper_CreateSubscription(connectionId, ffiCallback));
}

function addMonitoredItems(connectionId, nodeIdArray){
    return (0 == bind.sopc_client.SOPC_ClientHelper_AddMonitoredItems(connectionId, nodeIdArray, nodeIdArray.length));
}

function write(connectionId, writeValueArray)
{
    var writeValueArrayC = [];
    for (var elt of writeValueArray)
    {
        writeValueArrayC.push(elt.ToC());
    }
    var status_codes = new bind.UInt32Array(writeValueArrayC.length);
    var status = bind.sopc_client.SOPC_ClientHelper_Write(connectionId,
                                                          writeValueArrayC,
                                                          writeValueArrayC.length,
                                                          status_codes);
    console.log("write status: ", status);
    //TODO return status codes for each write
    console.log("write status codes: ", status_codes);
    return status == 0;
}

function disconnect(connectionId){
    return (0 == bind.sopc_client.SOPC_ClientHelper_Disconnect(connectionId));
}

module.exports.log_level = SOPC_Toolkit_Log_Level;
module.exports.security_mode = SOPC_MessageSecurityMode;
module.exports.security_policy = SOPC_SecurityPolicy;
module.exports.SecurityCfg = SecurityCfg;
module.exports.initialize = initialize;
module.exports.finalize = finalize;
module.exports.connect = connect;
module.exports.disconnect = disconnect;
module.exports.addMonitoredItems = addMonitoredItems;
module.exports.createSubscription = createSubscription;

module.exports.Variant = variant.Variant;
module.exports.DataValue = data_value.DataValue;

module.exports.write = write;
module.exports.WriteValue = write_value.WriteValue;
