const sopc_client = require('../lib/sopc_client');

const default_endpoint = "opc.tcp://localhost:4841";
const default_security_policy = sopc_client.security_policy.None_URI;
const default_security_mode = sopc_client.security_mode.None;
const default_user_security_policy_id = "anonymous";

const yargs = require('yargs');

const argv = yargs
    .usage("$0 -e <endpointURL> <NODE_ID> <VALUE>")
    .option('endpoint', {
        alias: 'e',
        description: 'URL of the endpoint to connect to',
        type: 'string',
        default: default_endpoint
    })
    .option('type_id', {
        alias: 't',
        description: 'Type identifier of the value (default: int32)',
        type: 'number',
        default: 6
    })
    .option('array_type', {
        alias: 'a',
        description: 'Array Type of the value (default: SingleValue)',
        type: 'number',
        default: 0
    })

    .help()
    .alias('help', 'h')
    .argv;

let status = argv._.length > 1;
let type_id = argv.type_id;
let array_type = argv.array_type;

if(status){
    status = sopc_client.initialize("./log/", sopc_client.log_level.Debug);
    status = status === 0;
}else{
    throw "Missing node id or value. See '--help' option"
}

let connectionId = 0;
if(status){
    let endpoint = argv.endpoint;
    let securityCfg = new sopc_client.SecurityCfg(default_security_policy,
                                              default_security_mode,
                                              default_user_security_policy_id);
    connectionId = sopc_client.connect(endpoint, securityCfg);
    status = connectionId > 0;
    console.log("Connection status:", status ? "SUCCESS" : "FAILED");
    console.log("Connection Id:", connectionId);
}

if(status)
{
    let node_id = argv._[0];
    let value = argv._[1];

    let variant = new sopc_client.Variant()
                                 .setValue(type_id, array_type, value);
    let data_value = new sopc_client.DataValue()
                                    .setValue(variant);

    let writeValue = new sopc_client.WriteValue()
                          .setNodeId(node_id)
                          .setValue(data_value);
    let writeValueArray = [ writeValue ];

    let writeStatusCodes;
    [status, writeStatusCodes] = sopc_client.write(connectionId, writeValueArray);
    for (let i = 0; i < writeStatusCodes.length; i++) {
        console.log(`write #${i} status:`, (writeStatusCodes[i] == 0) ? "SUCCESS" : "FAILURE");
    }
    status = status === 0;
    console.log("Writing nodes status : ", status ? "SUCCESS" : "FAILED");
}

function disconnect(connectionId){
    console.log("Disconnecting client");
    sopc_client.unsubscribe(connectionId);
    sopc_client.disconnect(connectionId);
    sopc_client.finalize();
}

disconnect(connectionId);

