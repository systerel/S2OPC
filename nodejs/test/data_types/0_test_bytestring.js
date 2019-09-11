
const sopc_client = require('../../lib/sopc_client');
const assert = require('assert');

const default_endpoint = "opc.tcp://localhost:4841";
const default_security_policy = sopc_client.security_policy.None_URI;
const default_security_mode = sopc_client.security_mode.None;
const default_user_security_policy_id = "anonymous";

var connectionId = 0;

function connect() {
    var status = true;
    if (status) {
        status = sopc_client.initialize("./log/", sopc_client.log_level.Error);
    }

    if (status) {
        var endpoint = default_endpoint;
        var securityCfg = new sopc_client.SecurityCfg(default_security_policy,
            default_security_mode,
            default_user_security_policy_id);
        connectionId = sopc_client.connect(endpoint, securityCfg);
        status = (connectionId > 0);
    }
    return status;
}

function disconnect(connectionId){
    sopc_client.disconnect(connectionId);
    sopc_client.finalize();
}

describe("ByteString type", function () {
    before(function(done){
        connect();
        done();
    });
    it("Every result shall be a scalar byteString", function (done) {
        var resultDataValues;
        var status;
        var read_value_bstring = new sopc_client.ReadValue()
            .setNodeId("ns=1;s=ByteString_029")
            .setAttributeId(13);
        var read_values = [read_value_bstring];

        [status, resultDataValues] = sopc_client.read(connectionId,
            read_values);
        assert.equal(status, 0, `Status is ${status}`);
        assert.equal(resultDataValues.length, 1);
        for (var elt of resultDataValues) {
            assert.equal(elt.value.array_type, 0, `element is not a scalar`);
            assert.ok(typeof elt.value.value == "string",
                `${elt.value.value} is not a byteString`);
        }
        done();
    });
    after(function(done) {
        disconnect(connectionId);
        done();
    });
});
