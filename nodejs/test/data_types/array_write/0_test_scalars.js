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

describe("Write Array simple types", function() {
    describe("Boolean type", function () {
        before(function(done) {
            connect();
            done();
        });
        it("Every result shall be OK", function (done) {
            var variant = new sopc_client.Variant()
                .setValue(1, 1, [false, true, false, true, false]);
            var data_value = new sopc_client.DataValue()
                                            .setValue(variant);
            var write_value_bool = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1015")
                .setValue(data_value);
            var write_values = [write_value_bool];

            var resultStatusCodes;
            var status;
            [status, resultStatusCodes] = sopc_client.write(connectionId,
                write_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultStatusCodes.length, 1);
            for (var elt of resultStatusCodes) {
                assert.equal(elt, 0, `Status is ${elt}`);
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
        it("Every result shall be OK", function (done) {
            var variant_8 = new sopc_client.Variant()
                .setValue(2, 1, [0, 1, 2, 3, 110]);
            var data_value_8 = new sopc_client.DataValue()
                                            .setValue(variant_8);
            var write_value_int8 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1020")
                .setValue(data_value_8);
            var variant_16 = new sopc_client.Variant()
                .setValue(4, 1, [0, 1, 2, 3, 2225]);
            var data_value_16 = new sopc_client.DataValue()
                                            .setValue(variant_16);
            var write_value_int16 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1017")
                .setValue(data_value_16);
            var variant_32 = new sopc_client.Variant()
                .setValue(6, 1, [0, 1, 2, 3, 4425]);
            var data_value_32 = new sopc_client.DataValue()
                                            .setValue(variant_32);
            var write_value_int32 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1018")
                .setValue(data_value_32);
            var variant_64 = new sopc_client.Variant()
                .setValue(8, 1, [0, "-11", 2, 3, 4256896097]);
            var data_value_64 = new sopc_client.DataValue()
                                            .setValue(variant_64);
            var write_value_int64 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1019")
                .setValue(data_value_64);
            var write_values = [write_value_int8,
                                write_value_int16,
                                write_value_int32,
                                write_value_int64 ];

            var resultStatusCodes;
            var status;
            [status, resultStatusCodes] = sopc_client.write(connectionId,
                write_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultStatusCodes.length, 4);
            for (var elt of resultStatusCodes) {
                assert.equal(elt, 0, `Status is ${elt}`);
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
        it("Every result shall be OK", function (done) {
            var variant_8 = new sopc_client.Variant()
                .setValue(3, 1, [2, 3, 4, 5, 110]);
            var data_value_8 = new sopc_client.DataValue()
                                            .setValue(variant_8);
            var write_value_uint8 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1016")
                .setValue(data_value_8);
            var variant_16 = new sopc_client.Variant()
                .setValue(5, 1, [2, 3, 4, 5, 2225]);
            var data_value_16 = new sopc_client.DataValue()
                                            .setValue(variant_16);
            var write_value_uint16 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1021")
                .setValue(data_value_16);
            var variant_32 = new sopc_client.Variant()
                .setValue(7, 1, [1, 2, 3, 4, 4425]);
            var data_value_32 = new sopc_client.DataValue()
                                            .setValue(variant_32);
            var write_value_uint32 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1022")
                .setValue(data_value_32);
            var variant_64 = new sopc_client.Variant()
                .setValue(9, 1, [2, 3, 4, 5, "4256896097"]);
            var data_value_64 = new sopc_client.DataValue()
                                            .setValue(variant_64);
            var write_value_uint64 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1023")
                .setValue(data_value_64);
            var write_values = [write_value_uint8,
                                write_value_uint16,
                                write_value_uint32,
                                write_value_uint64 ];

            var resultStatusCodes;
            var status;
            [status, resultStatusCodes] = sopc_client.write(connectionId,
                write_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultStatusCodes.length, 4);
            for (var elt of resultStatusCodes) {
                assert.equal(elt, 0, `Status is ${elt}`);
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

        it("Every result shall be OK", function (done) {
            var variant_float = new sopc_client.Variant()
                .setValue(10, 1, [1.1, 2.2, 3.3, 4.4, 110.13]);
            var data_value_float = new sopc_client.DataValue()
                                            .setValue(variant_float);
            var write_value_float = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1024")
                .setValue(data_value_float);
            var variant_double = new sopc_client.Variant()
                .setValue(11, 1, [0.1, 0.2, 0.3, 0.4, 2225.1345]);
            var data_value_double = new sopc_client.DataValue()
                                            .setValue(variant_double);
            var write_value_double = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1025")
                .setValue(data_value_double);
            var write_values = [write_value_float,
                                write_value_double];

            var resultStatusCodes;
            var status;
            [status, resultStatusCodes] = sopc_client.write(connectionId,
                write_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultStatusCodes.length, 2);
            for (var elt of resultStatusCodes) {
                assert.equal(elt, 0, `Status is ${elt}`);
            }
            done();
        });
        after(function(done) {
            disconnect(connectionId);
            done();
        });
    });
});
