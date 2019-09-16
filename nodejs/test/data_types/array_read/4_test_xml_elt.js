const sopc_client = require('../../../lib/sopc_client');
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

describe("Xml Elt type", function () {
    before(function (done) {
        connect();
        done();
    });
    it("Every result shall be a Xml Elt array", function (done) {
        var read_value_string = new sopc_client.ReadValue()
            .setNodeId("ns=1;i=1028")
            .setAttributeId(13);
        var read_values = [read_value_string];

        var resultDataValues1;
        var status;
        [status, resultDataValues1] = sopc_client.read(connectionId,
            read_values);
        assert.equal(resultDataValues1.length, 1);
        assert.equal(status, 0, `Status is ${status}`);
        for (var elt of resultDataValues1) {
            assert.equal(elt.value.array_type, 1, `element is not an array`);
            assert.ok(Array.isArray(elt.value.value));
            assert.ok(elt.value.value.length > 0);
            for (var arrElt of elt.value.value) {
                assert.ok(typeof arrElt == "string",
                    `${arrElt} is not a string`);
            }
        }
        done();
    });
    after(function (done) {
        disconnect(connectionId);
        done();
    });
});
