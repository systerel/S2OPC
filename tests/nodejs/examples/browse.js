const sopc_client = require('../lib/sopc_client');

const default_endpoint = "opc.tcp://localhost:4841";
const default_security_policy = sopc_client.security_policy.None_URI;
const default_security_mode = sopc_client.security_mode.None;
const default_user_security_policy_id = "anonymous";

const yargs = require('yargs');

const argv = yargs
    .usage("$0 -e <endpointURL> <NODE_ID> [<NODE_ID> ...]")
    .option('endpoint', {
        alias: 'e',
        description: 'URL of the endpoint to connect to',
        type: 'string',
        default: default_endpoint
    })
    .help()
    .alias('help', 'h')
    .argv;

let status = argv._.length > 0;

if(status){
    status = sopc_client.initialize("./log/", sopc_client.log_level.Debug);
    status = status === 0;
}else{
    throw "Missing node id(s). See '--help' option"
}

let connectionId = 0;
if(status){
    let endpoint = (argv.endpoint ? argv.endpoint : default_endpoint);
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
    let browse_req_array = [];
    for (let elt of argv._) {
        let browse_req = new sopc_client.BrowseRequest()
            .setNodeId(elt)
            //.setReferenceTypeId("") //not mandatory (NULL, by default)
            .setDirection(0) //forward
            .setIncludeSubtypes(true); //false by default
        browse_req_array.push(browse_req);
    }
    let browse_result_array;
    [status, browse_result_array] = sopc_client.browse(connectionId, browse_req_array);
    console.log(JSON.stringify(browse_result_array, null, 2));
    status = status === 0;
    console.log("Browsing nodes status:", status ? "SUCCESS" : "FAILED");
}

function disconnect(connectionId){
    console.log("Disconnecting client");
    sopc_client.unsubscribe(connectionId);
    sopc_client.disconnect(connectionId);
    sopc_client.finalize();
}

disconnect(connectionId);

