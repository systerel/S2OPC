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

describe("Array types", function() {
    describe("Boolean type", function () {
        before(function(done) {
            connect();
            done();
        });
        it("Every result shall be a boolean array", function (done) {
            var read_value_bool = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1015")
                .setAttributeId(13);
            var read_values = [read_value_bool];

            var resultDataValues;
            var status;
            [status, resultDataValues] = sopc_client.read(connectionId,
                read_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultDataValues.length, 1);
            for (var elt of resultDataValues) {
                assert.equal(elt.value.array_type, 1, `element is not an array`);
                assert.ok(Array.isArray(elt.value.value));
                assert.ok(elt.value.value.length > 0);
                for (var arrElt of elt.value.value) {
                    assert.ok(typeof arrElt == "boolean",
                        `${arrElt} is not a boolean`);
                }
            }
            done();
        });
        after(function(done) {
            disconnect(connectionId);
            done();
        });
    });

    describe("Integer types: SByte, 16, 32, 64", function () {
        before(function(done) {
            connect();
            done();
        });
        it("Every result shall be an integer array", function (done) {
            var read_value_int8 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1020")
                .setAttributeId(13);
            var read_value_int16 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1017")
                .setAttributeId(13);
            var read_value_int32 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1018")
                .setAttributeId(13);
            var read_value_int64 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1019")
                .setAttributeId(13);
            var read_values = [read_value_int8,
                read_value_int16,
                read_value_int32,
                read_value_int64];

            var resultDataValues;
            var status;
            [status, resultDataValues] = sopc_client.read(connectionId,
                read_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultDataValues.length, 4);
            for (var elt of resultDataValues) {
                assert.equal(elt.value.array_type, 1, `element is not an array`);
                assert.ok(Array.isArray(elt.value.value));
                assert.ok(elt.value.value.length > 0);
                for (var arrElt of elt.value.value) {
                    assert.ok(Number.isInteger(arrElt) || (!isNaN(parseFloat(arrElt)) && isFinite(arrElt)),
                              `${arrElt} is not an integer`);
                }
            }
            done();
        });
        after(function(done) {
            disconnect(connectionId);
            done();
        });
    });

    describe("Unsigned Integer types: Byte, 16, 32, 64", function () {
        before(function(done) {
            connect();
            done();
        });
        it("Every result shall be an unsigned integer array", function (done) {
            var read_value_uint8 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1016")
                .setAttributeId(13);
            var read_value_uint16 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1021")
                .setAttributeId(13);
            var read_value_uint32 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1022")
                .setAttributeId(13);
            var read_value_uint64 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1023")
                .setAttributeId(13);
            var read_values = [read_value_uint8,
                read_value_uint16,
                read_value_uint32,
                read_value_uint64];

            var resultDataValues;
            var status;
            [status, resultDataValues] = sopc_client.read(connectionId,
                read_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultDataValues.length, 4);
            for (var elt of resultDataValues) {
                assert.equal(elt.value.array_type, 1, `element is not an array`);
                assert.ok(Array.isArray(elt.value.value));
                assert.ok(elt.value.value.length > 0);
                for (var arrElt of elt.value.value) {
                    assert.ok(Number.isInteger(arrElt) || (!isNaN(parseFloat(arrElt)) && isFinite(arrElt)),
                              `${arrElt} is not an integer`);
                }
            }
            done();
        });
        after(function(done) {
            disconnect(connectionId);
            done();
        })
    });

    describe("Float types: float, double", function () {
        before(function (done) {
            connect();
            done();
        });

        it("Every result shall be a float array", function (done) {
            var read_value_float = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1024")
                .setAttributeId(13);
            var read_value_double = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1025")
                .setAttributeId(13);
            var read_values = [read_value_float,
                read_value_double];

            var resultDataValues;
            var status;
            [status, resultDataValues] = sopc_client.read(connectionId,
                read_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultDataValues.length, 2);
            for (var elt of resultDataValues) {
                assert.equal(elt.value.array_type, 1, `element is not an array`);
                assert.ok(Array.isArray(elt.value.value));
                assert.ok(elt.value.value.length > 0);
                for (var arrElt of elt.value.value) {
                    assert.ok(Number.isInteger(arrElt) || (!isNaN(parseFloat(arrElt)) && isFinite(arrElt)),
                              `${arrElt} is not an integer`);
                }
            }
            done();
        });
        after(function(done) {
            disconnect(connectionId);
            done();
        });
    });
});
