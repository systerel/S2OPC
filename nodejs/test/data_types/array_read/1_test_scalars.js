const sopc_client = require('../../../lib/sopc_client');
const test_helper = require('../../helpers/connection');
const assert = require('assert');

describe("Array types", function() {
    describe("Boolean type", function () {
        let connectionId = 0;
        before(function(done) {
            connectionId = test_helper.connect();
            done();
        });
        it("Every result shall be a boolean array", function (done) {
            let read_value_bool = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1015")
                .setAttributeId(13);
            let read_values = [read_value_bool];

            let resultDataValues;
            let status;
            [status, resultDataValues] = sopc_client.read(connectionId,
                read_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultDataValues.length, 1);
            for (let elt of resultDataValues) {
                assert.equal(elt.value.array_type, 1, `element is not an array`);
                assert.ok(Array.isArray(elt.value.value));
                assert.ok(elt.value.value.length > 0);
                for (let arrElt of elt.value.value) {
                    assert.ok(typeof arrElt === "boolean",
                        `${arrElt} is not a boolean`);
                }
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
        it("Every result shall be an integer array", function (done) {
            let read_value_int8 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1020")
                .setAttributeId(13);
            let read_value_int16 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1017")
                .setAttributeId(13);
            let read_value_int32 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1018")
                .setAttributeId(13);
            let read_value_int64 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1019")
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
                assert.equal(elt.value.array_type, 1, `element is not an array`);
                assert.ok(Array.isArray(elt.value.value));
                assert.ok(elt.value.value.length > 0);
                for (let arrElt of elt.value.value) {
                    assert.ok(Number.isInteger(arrElt) || (!isNaN(parseFloat(arrElt)) && isFinite(arrElt)),
                              `${arrElt} is not an integer`);
                }
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
        it("Every result shall be an unsigned integer array", function (done) {
            let read_value_uint8 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1016")
                .setAttributeId(13);
            let read_value_uint16 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1021")
                .setAttributeId(13);
            let read_value_uint32 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1022")
                .setAttributeId(13);
            let read_value_uint64 = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1023")
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
                assert.equal(elt.value.array_type, 1, `element is not an array`);
                assert.ok(Array.isArray(elt.value.value));
                assert.ok(elt.value.value.length > 0);
                for (let arrElt of elt.value.value) {
                    assert.ok(Number.isInteger(arrElt) || (!isNaN(parseFloat(arrElt)) && isFinite(arrElt)),
                              `${arrElt} is not an integer`);
                }
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

        it("Every result shall be a float array", function (done) {
            let read_value_float = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1024")
                .setAttributeId(13);
            let read_value_double = new sopc_client.ReadValue()
                .setNodeId("ns=1;i=1025")
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
                assert.equal(elt.value.array_type, 1, `element is not an array`);
                assert.ok(Array.isArray(elt.value.value));
                assert.ok(elt.value.value.length > 0);
                for (let arrElt of elt.value.value) {
                    assert.ok(Number.isInteger(arrElt) || (!isNaN(parseFloat(arrElt)) && isFinite(arrElt)),
                              `${arrElt} is not an integer`);
                }
            }
            done();
        });
        after(function(done) {
            test_helper.disconnect(connectionId);
            done();
        });
    });
});
