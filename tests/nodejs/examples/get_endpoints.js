const sopc_client = require('../lib/sopc_client');

const default_endpoint = "opc.tcp://localhost:4841";

const yargs = require('yargs');

const argv = yargs
    .usage("$0 -e <endpointURL>")
    .option('endpoint', {
        alias: 'e',
        description: 'URL of the endpoint to connect to',
        type: 'string',
    })
    .help()
    .alias('help', 'h')
    .argv;

let status = sopc_client.initialize("./log/", sopc_client.log_level.Debug);
status = status === 0;

if(status){
    let endpoint = (argv.endpoint ? argv.endpoint : default_endpoint);

    let endpoints;
    [status, endpoints] = sopc_client.getEndpoints(endpoint);
    console.log(JSON.stringify(endpoints, null, 2));
    status = (status === 0);

    console.log("getEndpoints status:", status ? "SUCCESS" : "FAILED");
}

sopc_client.finalize();

