const sopc_client = require('../lib/sopc_client');

const default_endpoint = "opc.tcp://localhost:4841";
const default_security_policy = sopc_client.security_policy.None_URI;
const default_security_mode = sopc_client.security_mode.None;
const default_user_security_policy_id = "anonymous";

const yargs = require('yargs');

const argv = yargs
    .usage("$0 -e <endpointURL> -t <timeout> <NODE_ID> [<NODE_ID> ...]")
    .option('endpoint', {
        alias: 'e',
        description: 'URL of the endpoint to connect to',
        type: 'string',
        default: default_endpoint
    })
    .option('timeout', {
        alias: 't',
        description: 'Subscription timeout in ms',
        type: 'number',
        default: 10000
    })
    .help()
    .alias('help', 'h')
    .argv;

let status = argv._.length > 0;

let endpoint = argv.endpoint;
let timeout = argv.timeout;

if(status){
    status = sopc_client.initialize("./log/", sopc_client.log_level.Debug);
    status = status === 0;
}else{
    throw "Missing node id(s). See '--help' option"
}

let dataChange_callback = function(connectionId, nodeId, dataValue) {
    console.log("connectionId:", connectionId);
    console.log("nodeId:", nodeId);
    console.log("Data Value:", dataValue)
}

let connectionId = 0;
if(status){
    let securityCfg = new sopc_client.SecurityCfg(default_security_policy,
                                              default_security_mode,
                                              default_user_security_policy_id);
    connectionId = sopc_client.connect(endpoint, securityCfg);
    status = connectionId > 0;
    console.log("Connection status:", status ? "SUCCESS" : "FAILED");
    console.log("Connection Id:", connectionId);
}

if (status)
{
    let res = sopc_client.createSubscription(connectionId, dataChange_callback);
    status = res === 0;
    console.log("Subscription status:", status ? "SUCCESS" : "FAILED");
}

if(status){
    argv._.forEach((v) => {
        if(typeof v !== 'string'){
            throw `'${v}' is not a string`
        }
    });
    status = sopc_client.addMonitoredItems(connectionId, argv._);
    status = status === 0;
    console.log("Adding monitored items status:", status ? "SUCCESS" : "FAILED");
}

function disconnect(connectionId){
    console.log("Disconnecting client");
    sopc_client.unsubscribe(connectionId);
    sopc_client.disconnect(connectionId);
    sopc_client.finalize();
}

if (status)
{
    setTimeout(disconnect, timeout, connectionId);
}
else
{
    disconnect(connectionId);
}

