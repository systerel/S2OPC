const sopc_client = require('../../../lib/sopc_client');
const test_helper = require('../../helpers/connection');
const assert = require('assert');

describe("Write Array simple types", function() {
    describe("Boolean type", function () {
        let connectionId = 0;
        before(function(done) {
            connectionId = test_helper.connect();
            done();
        });
        it("Every result shall be OK", function (done) {
            let variant = new sopc_client.Variant()
                .setValue(1, 1, [false, true, false, true, false]);
            let data_value = new sopc_client.DataValue()
                                            .setValue(variant);
            let write_value_bool = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1015")
                .setValue(data_value);
            let write_values = [write_value_bool];

            let resultStatusCodes;
            let status;
            [status, resultStatusCodes] = sopc_client.write(connectionId,
                write_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultStatusCodes.length, 1);
            for (let elt of resultStatusCodes) {
                assert.equal(elt, 0, `Status is ${elt}`);
            }
            done();
        });
        after(function(done) {
            test_helper.disconnect(connectionId);
            done();
        });
    });

    describe("Integer types: SByte, 16, 32, 64", function () {
        let connectionId = 0;
        before(function(done) {
            connectionId = test_helper.connect();
            done();
        });
        it("Every result shall be OK", function (done) {
            let variant_8 = new sopc_client.Variant()
                .setValue(2, 1, [0, 1, 2, 3, 110]);
            let data_value_8 = new sopc_client.DataValue()
                                            .setValue(variant_8);
            let write_value_int8 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1020")
                .setValue(data_value_8);
            let variant_16 = new sopc_client.Variant()
                .setValue(4, 1, [0, 1, 2, 3, 2225]);
            let data_value_16 = new sopc_client.DataValue()
                                            .setValue(variant_16);
            let write_value_int16 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1017")
                .setValue(data_value_16);
            let variant_32 = new sopc_client.Variant()
                .setValue(6, 1, [0, 1, 2, 3, 4425]);
            let data_value_32 = new sopc_client.DataValue()
                                            .setValue(variant_32);
            let write_value_int32 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1018")
                .setValue(data_value_32);
            let variant_64 = new sopc_client.Variant()
                .setValue(8, 1, [0, "-11", 2, 3, 4256896097]);
            let data_value_64 = new sopc_client.DataValue()
                                            .setValue(variant_64);
            let write_value_int64 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1019")
                .setValue(data_value_64);
            let write_values = [write_value_int8,
                                write_value_int16,
                                write_value_int32,
                                write_value_int64 ];

            let resultStatusCodes;
            let status;
            [status, resultStatusCodes] = sopc_client.write(connectionId,
                write_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultStatusCodes.length, 4);
            for (let elt of resultStatusCodes) {
                assert.equal(elt, 0, `Status is ${elt}`);
            }
            done();
        });

        after(function(done) {
            test_helper.disconnect(connectionId);
            done();
        });
    });

    describe("Unsigned Integer types: Byte, 16, 32, 64", function () {
        let connectionId = 0;
        before(function(done) {
            connectionId = test_helper.connect();
            done();
        });
        it("Every result shall be OK", function (done) {
            let variant_8 = new sopc_client.Variant()
                .setValue(3, 1, [2, 3, 4, 5, 110]);
            let data_value_8 = new sopc_client.DataValue()
                                            .setValue(variant_8);
            let write_value_uint8 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1016")
                .setValue(data_value_8);
            let variant_16 = new sopc_client.Variant()
                .setValue(5, 1, [2, 3, 4, 5, 2225]);
            let data_value_16 = new sopc_client.DataValue()
                                            .setValue(variant_16);
            let write_value_uint16 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1021")
                .setValue(data_value_16);
            let variant_32 = new sopc_client.Variant()
                .setValue(7, 1, [1, 2, 3, 4, 4425]);
            let data_value_32 = new sopc_client.DataValue()
                                            .setValue(variant_32);
            let write_value_uint32 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1022")
                .setValue(data_value_32);
            let variant_64 = new sopc_client.Variant()
                .setValue(9, 1, [2, 3, 4, 5, "4256896097"]);
            let data_value_64 = new sopc_client.DataValue()
                                            .setValue(variant_64);
            let write_value_uint64 = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1023")
                .setValue(data_value_64);
            let write_values = [write_value_uint8,
                                write_value_uint16,
                                write_value_uint32,
                                write_value_uint64 ];

            let resultStatusCodes;
            let status;
            [status, resultStatusCodes] = sopc_client.write(connectionId,
                write_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultStatusCodes.length, 4);
            for (let elt of resultStatusCodes) {
                assert.equal(elt, 0, `Status is ${elt}`);
            }
            done();
        });
        after(function(done) {
            test_helper.disconnect(connectionId);
            done();
        })
    });

    describe("Float types: float, double", function () {
        let connectionId = 0;
        before(function (done) {
            connectionId = test_helper.connect();
            done();
        });

        it("Every result shall be OK", function (done) {
            let variant_float = new sopc_client.Variant()
                .setValue(10, 1, [1.1, 2.2, 3.3, 4.4, 110.13]);
            let data_value_float = new sopc_client.DataValue()
                                            .setValue(variant_float);
            let write_value_float = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1024")
                .setValue(data_value_float);
            let variant_double = new sopc_client.Variant()
                .setValue(11, 1, [0.1, 0.2, 0.3, 0.4, 2225.1345]);
            let data_value_double = new sopc_client.DataValue()
                                            .setValue(variant_double);
            let write_value_double = new sopc_client.WriteValue()
                .setNodeId("ns=1;i=1025")
                .setValue(data_value_double);
            let write_values = [write_value_float,
                                write_value_double];

            let resultStatusCodes;
            let status;
            [status, resultStatusCodes] = sopc_client.write(connectionId,
                write_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultStatusCodes.length, 2);
            for (let elt of resultStatusCodes) {
                assert.equal(elt, 0, `Status is ${elt}`);
            }
            done();
        });
        after(function(done) {
            test_helper.disconnect(connectionId);
            done();
        });
    });
});
