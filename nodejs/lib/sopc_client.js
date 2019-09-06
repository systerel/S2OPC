const bind = require('./bind_sopc_client')
const ffi = require('ffi');
const ref = require('ref');
const Enum = require('enum');
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

const SOPC_SecurityPolicy = {
    'None_URI' : "http://opcfoundation.org/UA/SecurityPolicy#None",
    'Basic256_URI' : "http://opcfoundation.org/UA/SecurityPolicy#Basic256",
    'Basic256Sha256_URI' : "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"
};

function DataValue(value) {
    const type_id = value.value.built_in_type_id;
    this.value = null;
    switch(type_id)
    {
        case bind.SOPC_BuiltinId.SOPC_Boolean_Id.value:
            console.log("DataValue Boolean: TODO");
            break;
        case bind.SOPC_BuiltinId.SOPC_Int32_Id.value:
            this.value = value.value.variant_value.int32;
            break;
        case bind.SOPC_BuiltinId.SOPC_String_Id.value:
            this.value = ref.readCString(value.value.variant_value.string.data);
            break;
        case bind.SOPC_BuiltinId.SOPC_ByteString_Id.value:
            console.log("DataValue ByteString: TODO");
            break;
        default:
            console.log("DataValue type", type_id, "not managed: TODO");
            break;
    }
    this.status = value.status;
    this.src_ts = value.src_ts;
    this.src_ps = value.src_ps;
    this.srv_ts = value.srv_ts;
    this.srv_ps = value.srv_ps;
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
