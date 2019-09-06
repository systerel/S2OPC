const bind = require('./bind_sopc_client')
const ffi = require('ffi');
const ref = require('ref');
const Enum = require('enum');
const Struct = require('ref-struct');
const process = require('process');


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

//var SOPC_SecurityPolicy = new Enum({
//    'None_URI' : "http://opcfoundation.org/UA/SecurityPolicy#None",
//    'Basic256_URI' : "http://opcfoundation.org/UA/SecurityPolicy#Basic256",
//    'Basic256Sha256_URI' : "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"
//});

const SOPC_SecurityPolicy = {
    'None_URI' : "http://opcfoundation.org/UA/SecurityPolicy#None",
    'Basic256_URI' : "http://opcfoundation.org/UA/SecurityPolicy#Basic256",
    'Basic256Sha256_URI' : "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"
};

const SOPC_LibSub_DataType = new Enum({
    'bool': 1,
    'integer': 2,
    'string': 3,
    'bytestring': 4,
    'other': 5
});

const SOPC_LibSub_Value = Struct({
    'type': 'int32',
    'quality': 'uint32',
    'value': 'pointer',
    'source_timestamp': 'uint64',
    'server_timestamp': 'uint64',
    'raw_value': 'pointer'
});

function DataValue(value) {
    console.log(value);
    if(value.type == SOPC_LibSub_DataType.integer.value ||
       value.type == SOPC_LibSub_DataType.bool.value){
        var buffer = value.value
        var intval = ref.readInt64LE(buffer, 0)
        this.value = intval;
    }else if(value.type == SOPC_LibSub_DataType.string.value ||
             value.type == SOPC_LibSub_DataType.bytestring.value){
        console.log("DataValue string value: TODO")
    }else{
        console.log("DataValue not managed: TODO");
    }
    this.quality = value.quality;
    this.src_ts = value.source_timestamp;
    this.srv_ts = value.server_timestamp;
}

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

    console.log(securityCfg);
    return securityCfg;
}

var SOPC_DataValuePtr = ref.refType(SOPC_LibSub_Value);

function initialize(toolkit_log_path, toolkit_log_level){
    return (0 == bind.sopc_client.SOPC_ClientHelper_Initialize(toolkit_log_path, toolkit_log_level.value));
}

function finalize(){
    return bind.sopc_client.SOPC_ClientHelper_Finalize();
}

function connect(endpoint_url, security){
    var connectionId = bind.sopc_client.SOPC_ClientHelper_Connect(endpoint_url,
                                                                  security);
    return [connectionId > 0, connectionId];
}

function createSubscription(connectionId, user_callback)
{
    var ffiCallback = ffi.Callback('void', ['int32', 'CString', SOPC_DataValuePtr],
                                   function(connectionId, nodeId, value) {
                                       var dereferenced_value = value.deref();
                                       var data_value = new DataValue(dereferenced_value);
                                       user_callback(connectionId, nodeId, data_value);
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
