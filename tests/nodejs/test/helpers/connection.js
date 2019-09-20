
const sopc_client = require('../../lib/sopc_client');

const default_endpoint = "opc.tcp://localhost:4841";
const default_security_policy = sopc_client.security_policy.None_URI;
const default_security_mode = sopc_client.security_mode.None;
const default_user_security_policy_id = "anonymous";

function connect() {
    let connectionId = 0;
    let status = true;
    if (status) {
        status = sopc_client.initialize("./log/", sopc_client.log_level.Error);
        status = status === 0;
    }

    if (status) {
        let endpoint = default_endpoint;
        let securityCfg = new sopc_client.SecurityCfg(default_security_policy,
            default_security_mode,
            default_user_security_policy_id);
        connectionId = sopc_client.connect(endpoint, securityCfg);
        status = (connectionId > 0);
    }
    return connectionId;
}

function disconnect(connectionId){
    sopc_client.disconnect(connectionId);
    sopc_client.finalize();
}

module.exports = {
    connect,
    disconnect
}
