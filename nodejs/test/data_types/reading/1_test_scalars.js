const sopc_client = require('../../../lib/sopc_client');
const test_helper = require('../../helpers/connection');
const assert = require('assert');

describe("Scalar types", function() {
    describe("Boolean type", function () {
        let connectionId = 0;
        before(function(done) {
            connectionId = test_helper.connect();
            done();
        });
        it("Every result shall be a scalar boolean", function (done) {
            let read_value_bool = new sopc_client.ReadValue()
                .setNodeId("ns=1;s=Boolean_030")
                .setAttributeId(13);
            let read_values = [read_value_bool];

            let resultDataValues;
            let status;
            [status, resultDataValues] = sopc_client.read(connectionId,
                read_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultDataValues.length, 1);
            for (let elt of resultDataValues) {
                assert.equal(elt.value.array_type, 0, `element is not a scalar`);
                assert.ok(typeof elt.value.value == "boolean",
                    `${elt.value.value} is not a boolean`);
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
        it("Every result shall be a scalar integer", function (done) {
            let read_value_int8 = new sopc_client.ReadValue()
                .setNodeId("ns=1;s=SByte_030")
                .setAttributeId(13);
            let read_value_int16 = new sopc_client.ReadValue()
                .setNodeId("ns=1;s=Int16_030")
                .setAttributeId(13);
            let read_value_int32 = new sopc_client.ReadValue()
                .setNodeId("ns=1;s=Int32_030")
                .setAttributeId(13);
            let read_value_int64 = new sopc_client.ReadValue()
                .setNodeId("ns=1;s=Int64_030")
                .setAttributeId(13);
            let read_values = [read_value_int8,
                read_value_int16,
                read_value_int32,
                read_value_int64];

            let resultDataValues;
            let status;
            [status, resultDataValues] = sopc_client.read(connectionId,
                read_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultDataValues.length, 4);
            for (let elt of resultDataValues) {
                assert.equal(elt.value.array_type, 0, `element is not a scalar`);
                assert.ok(Number.isInteger(elt.value.value), `${elt.value.value} is not an integer`);
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
        it("Every result shall be a scalar unsigned integer", function (done) {
            let read_value_uint8 = new sopc_client.ReadValue()
                .setNodeId("ns=1;s=Byte_030")
                .setAttributeId(13);
            let read_value_uint16 = new sopc_client.ReadValue()
                .setNodeId("ns=1;s=UInt16_030")
                .setAttributeId(13);
            let read_value_uint32 = new sopc_client.ReadValue()
                .setNodeId("ns=1;s=UInt32_030")
                .setAttributeId(13);
            let read_value_uint64 = new sopc_client.ReadValue()
                .setNodeId("ns=1;s=UInt64_030")
                .setAttributeId(13);
            let read_values = [read_value_uint8,
                read_value_uint16,
                read_value_uint32,
                read_value_uint64];

            let resultDataValues;
            let status;
            [status, resultDataValues] = sopc_client.read(connectionId,
                read_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultDataValues.length, 4);
            for (let elt of resultDataValues) {
                assert.equal(elt.value.array_type, 0, `element is not a scalar`);
                assert.ok(Number.isInteger(elt.value.value), `${elt.value.value} is not an integer`);
                assert.ok(elt.value.value >= 0);
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

        it("Every result shall be a scalar float", function (done) {
            let read_value_float = new sopc_client.ReadValue()
                .setNodeId("ns=1;s=Float_030")
                .setAttributeId(13);
            let read_value_double = new sopc_client.ReadValue()
                .setNodeId("ns=1;s=Double_030")
                .setAttributeId(13);
            let read_values = [read_value_float,
                read_value_double];

            let resultDataValues;
            let status;
            [status, resultDataValues] = sopc_client.read(connectionId,
                read_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultDataValues.length, 2);
            for (let elt of resultDataValues) {
                assert.equal(elt.value.array_type, 0, `element is not a scalar`);
                assert.ok(typeof elt.value.value == "number",
                    `${elt.value.value} is not a float`);
            }
            done();
        });
        after(function(done) {
            test_helper.disconnect(connectionId);
            done();
        });
    });
});
