const util = require('util');
const sopc_client = require('./sopc_client');

const default_endpoint = "opc.tcp://localhost:4841";
const default_secu_policy = sopc_client.secu_policy.None_URI;
const default_secu_mode = sopc_client.secu_mode.None;
const default_user_secu_policy_id = "anonymous";

const yargs = require('yargs');

const argv = yargs
    .usage("$0 -e <endpointURL> <NODE_ID> [<NODE_ID> ...]")
    .option('endpoint', {
        alias: 'e',
        description: 'URL of the endpoint to connect to',
        type: 'string',
    })

    .help()
    .alias('help', 'h')
    .argv;

var bres = argv._.length > 0;

if(bres){
    bres = sopc_client.initialize("./log/", sopc_client.log_level.Debug);
}else{
    throw "Missing node id(s). See '--help' option"
}
    
var callback = function(connectionId, dataId, dataValue) {
    console.log("connectionId: ", connectionId);
    console.log("dataId: ", dataId);
    console.log("value quality: ", dataValue.quality);
    console.log("value source TS: ", dataValue.src_ts);
    console.log("value server TS: ", dataValue.srv_ts);  
    console.log("value: ", dataValue.value);
}

var connectionId = 0;
var bres_connection = false;
if(bres){
    var endpoint = (argv.endpoint ? argv.endpoint : default_endpoint);
    var secuCfg = new sopc_client.SecurityCfg(default_secu_policy,
                                              default_secu_mode,
                                              default_user_secu_policy_id);
    connectionRes = sopc_client.connect(endpoint, secuCfg, callback);
    bres_connection = connectionRes[0];
    connectionId = connectionRes[1];
}

if(bres_connection){
    argv._.forEach((v) => {
        if(typeof v !== 'string'){
            throw `'${v}' is not a string`
        }
    });
    res = sopc_client.subscribe(connectionId, argv._);
}else{
    bres = false;
}

function disconnect(connectionId){
    if(bres_connection){
        res = sopc_client.disconnect(connectionId);
    }

    sopc_client.finalize();
}

setTimeout(disconnect, 1000000, connectionId);

