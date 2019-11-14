const sopc_client = require('../../lib/sopc_client');
const assert = require('assert');
const path = require('path');

const default_endpoint = "opc.tcp://localhost:4841";
const default_security_policy = sopc_client.security_policy.Basic256_URI;
const default_security_mode = sopc_client.security_mode.Sign;
const default_user_security_policy_id = "anonymous";

let prefixPathCert = path.join(__dirname, "../../../data/cert/");
let path_cert_cli  = path.join(prefixPathCert, "client_2k_cert.der");
let path_cert_auth = path.join(prefixPathCert, "cacert.der");
let path_cert_srv  = path.join(prefixPathCert, "server_2k_cert.der");
let path_key_cli   = path.join(prefixPathCert, "client_2k_key.pem");

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
            default_user_security_policy_id,
            path_cert_auth = path_cert_auth,
            path_cert_srv = path_cert_srv,
            path_cert_cli = path_cert_cli,
            path_key_cli = path_key_cli,
        );
        connectionId = sopc_client.connect(endpoint, securityCfg);
        status = (connectionId > 0);
    }
    return connectionId;
}

function disconnect(connectionId){
    sopc_client.disconnect(connectionId);
    sopc_client.finalize();
}

describe("Sign anonymous", function () {
    let connectionId = 0;
    it("Sign connection shall be successful", function (done) {
        connectionId = connect();
        assert.ok(connectionId > 0, "Connection Id shall be greater than 0");
        done();
    });
    after(function (done) {
        disconnect(connectionId);
        done();
    });
});
