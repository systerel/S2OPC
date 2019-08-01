var bind = require('./bind_sopc_client')
var ffi = require('ffi');
var ref = require('ref');
var Enum = require('enum');
var Struct = require('ref-struct');


var SOPC_Toolkit_Log_Level = new Enum({
    'Error': 0,
    'Warning': 1,
    'Info': 2,
    'Debug': 3
});

var SOPC_MessageSecurityMode = new Enum({
    'None' : 1,
    'Sign' : 2,
    'SignAndEncrypt' : 3
});

var SOPC_SecurityPolicy = new Enum({
    'None_URI' : "http://opcfoundation.org/UA/SecurityPolicy#None",
    'Basic256_URI' : "http://opcfoundation.org/UA/SecurityPolicy#Basic256",
    'Basic256Sha256_URI' : "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"
});

var SOPC_LibSub_DataType = new Enum({
    'bool': 1,
    'integer': 2,
    'string': 3,
    'bytestring': 4,
    'other': 5
});

var SOPC_LibSub_Value = Struct({
    'type': 'int32',
    'quality': 'uint32',
    'value': 'pointer',
    'source_timestamp': 'uint64',
    'server_timestamp': 'uint64',
    'raw_value': 'pointer'
});

function DataValue(libsub_value) {
    if(libsub_value.type == SOPC_LibSub_DataType.integer.value ||
       libsub_value.type == SOPC_LibSub_DataType.bool.value){
        var buffer = libsub_value.value
        var intval = ref.readInt64LE(buffer, 0)
        this.value = intval;
    }else if(libsub_value.type == SOPC_LibSub_DataType.string.value ||
             libsub_value.type == SOPC_LibSub_DataType.bytestring.value){
        throw "value string value: TODO"
    }else{
        throw "value not managed: TODO"
    }
    this.quality = libsub_value.quality;
    this.src_ts = libsub_value.source_timestamp;
    this.srv_ts = libsub_value.server_timestamp;
}

function SecurityCfg(security_policy, security_mode, user_policy_id,
                     path_cert_auth=ref.NULL, path_cert_srv = ref.NULL,
                     path_cert_cli = ref.NULL, path_key_cli = ref.NULL,
                     user_name = ref.NULL, user_password = ref.NULL)
{
    console.log(user_policy_id);
    this.policy = security_policy.value;
    this.mode = security_mode.value;
    this.user_policy_id = user_policy_id;
    this.path_cert_auth = path_cert_auth;
    this.path_cert_srv = path_cert_srv;
    this.path_cert_cli = path_cert_cli;
    this.path_key_cli = path_key_cli;
    this.user_name = user_name;
    this.user_password = user_password;
}

var SOPC_LibSub_ValuePtr = ref.refType(SOPC_LibSub_Value);

function initialize(toolkit_log_path, toolkit_log_level){
    return bind.sopc_client.SOPC_ClientHelper_Initialize(toolkit_log_path, toolkit_log_level.value) == 0;
}

function finalize(){
    return bind.sopc_client.SOPC_ClientHelper_Finalize();
}

function connect(endpoint_url, security, callback){
    var ffiCallback = ffi.Callback('void', ['uint32', 'uint32', SOPC_LibSub_ValuePtr],
                                   function(connectionId, dataId, v) {
                                       var libsub_value = v.deref();
                                       var value = new DataValue(libsub_value);
                                       callback(connectionId, dataId, value);
                                   });

    // Make an extra reference to the callback pointer to avoid GC
    process.on('exit', function() {
        ffiCallback
    });
    // Make an extra reference to the callback pointer to avoid GC
    process.on('exit', function() {
        callback
    });


    connectionId = bind.sopc_client.SOPC_ClientHelper_Connect(endpoint_url,
                                                              security.policy,
                                                              security.mode,
                                                              security.path_cert_auth,
                                                              security.path_cert_srv,
                                                              security.path_cert_cli,
                                                              security.path_key_cli,
                                                              security.user_policy_id,
                                                              security.user_name,
                                                              security.user_password,
                                                              ffiCallback);
    return [connectionId > 0, connectionId];
}

function subscribe(connectionId, nodeIdArray){
    return bind.sopc_client.SOPC_ClientHelper_Subscribe(connectionId, nodeIdArray, nodeIdArray.length) == 0;
}

function disconnect(connectionId){
    return bind.sopc_client.SOPC_ClientHelper_Disconnect(connectionId) == 0;
}

module.exports.log_level = SOPC_Toolkit_Log_Level;
module.exports.secu_mode = SOPC_MessageSecurityMode;
module.exports.secu_policy = SOPC_SecurityPolicy;
module.exports.SecurityCfg = SecurityCfg;
module.exports.initialize = initialize;
module.exports.finalize = finalize;
module.exports.connect = connect;
module.exports.disconnect = disconnect;
module.exports.subscribe = subscribe;
