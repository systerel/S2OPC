const sopc_client = require('../../lib/sopc_client');
const test_helper = require('../helpers/connection');
const assert = require('assert');

describe("Operations tests", function() {
    describe("Successful Read", function () {
        let connectionId = 0;
        before(function(done) {
            connectionId = test_helper.connect();
            done();
        });
        it("Read shall be successful", function (done) {
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
    describe("Failed Read", function () {
        let connectionId = 0;
        before(function(done) {
            connectionId = test_helper.connect();
            done();
        });
        it("Read shall fail", function (done) {
            let read_value_bool = new sopc_client.ReadValue()
                .setNodeId("ns=1;s=nodeThatShouldNotExist")
                .setAttributeId(13);
            let read_values = [read_value_bool];

            let resultDataValues;
            let status;
            [status, resultDataValues] = sopc_client.read(connectionId,
                read_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultDataValues.length, 1);
            for (let elt of resultDataValues) {
                assert.notEqual(elt.status, 0, `Status shall not be equal not 0, is ${elt.status}`);
            }
            done();
        });
        after(function(done) {
            test_helper.disconnect(connectionId);
            done();
        });
    });
    describe("Successful Write", function () {
        let connectionId = 0;
        before(function(done) {
            connectionId = test_helper.connect();
            done();
        });
        it("Every result shall be OK", function (done) {
            let variant = new sopc_client.Variant()
                .setValue(1, 0, true);
            let data_value = new sopc_client.DataValue()
                                            .setValue(variant);
            let write_value_bool = new sopc_client.WriteValue()
                .setNodeId("ns=1;s=Boolean_030")
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
    describe("Failed Write", function () {
        let connectionId = 0;
        before(function(done) {
            connectionId = test_helper.connect();
            done();
        });
        it("Every result shall be OK", function (done) {
            let variant = new sopc_client.Variant()
                .setValue(1, 0, true);
            let data_value = new sopc_client.DataValue()
                                            .setValue(variant);
            let write_value_bool = new sopc_client.WriteValue()
                .setNodeId("ns=1;s=NodeThatDoesNotExist")
                .setValue(data_value);
            let write_values = [write_value_bool];

            let resultStatusCodes;
            let status;
            [status, resultStatusCodes] = sopc_client.write(connectionId,
                write_values);
            assert.equal(status, 0, `Status is ${status}`);
            assert.equal(resultStatusCodes.length, 1);
            for (let elt of resultStatusCodes) {
                assert.notEqual(elt, 0, `Status is ${elt}`);
            }
            done();
        });
        after(function(done) {
            test_helper.disconnect(connectionId);
            done();
        });
    });
    describe("Successful Browse", function () {
        let connectionId = 0;
        before(function(done) {
            connectionId = test_helper.connect();
            done();
        });
        it("Every result shall be OK", function (done) {
            let browse_req = new sopc_client.BrowseRequest()
                                            .setNodeId("ns=1;s=Int32_030")
                                            .setReferenceTypeId("")
                                            .setDirection(1)
                                            .setIncludeSubtypes(true);
            let browse_req_array = [browse_req];
            let browse_result_array;
            let status = -1;
            [status, browse_result_array] = sopc_client.browse(connectionId, browse_req_array);
            assert.equal(status, 0);
            for (let elt of browse_result_array) {
                assert.equal(elt.status, 0);
                assert.ok(elt.references.length > 0);
            }
            done();
        });
        after(function(done) {
            test_helper.disconnect(connectionId);
            done();
        });
    });
    describe("Failed Browse", function () {
        let connectionId = 0;
        before(function(done) {
            connectionId = test_helper.connect();
            done();
        });
        it("Every result shall be OK", function (done) {
            let browse_req = new sopc_client.BrowseRequest()
                                            .setNodeId("ns=1;s=NodeThatShouldNotExist")
                                            .setReferenceTypeId("")
                                            .setDirection(1)
                                            .setIncludeSubtypes(true);
            let browse_req_array = [browse_req];
            let browse_result_array;
            let status = -1;
            [status, browse_result_array] = sopc_client.browse(connectionId, browse_req_array);
            assert.equal(status, 0);
            for (let elt of browse_result_array) {
                assert.notEqual(elt.status, 0);
            }
            done();
        });
        after(function(done) {
            test_helper.disconnect(connectionId);
            done();
        });
    });
    describe("Successful Subscribe", function () {
        let connectionId = 0;
        before(function(done) {
            connectionId = test_helper.connect();
            done();
        });
        it("Every result shall be OK", function (done) {
            let node_id = "ns=1;i=1004";
            let dataChange_callback = function (connectionId_Cb, nodeId_Cb, dataValue_Cb) {
                assert.equal(connectionId_Cb, connectionId);
                assert.equal(nodeId_Cb, node_id);
                done(); // should end the test when we receive the callback,
                        // else test will fail (timeout)
            }

            let res = sopc_client.createSubscription(connectionId, dataChange_callback);
            assert.equal(res, 0);
            res = sopc_client.addMonitoredItems(connectionId, [ node_id ]);
            assert.equal(res, 0);
        });
        after(function(done) {
            test_helper.disconnect(connectionId);
            done();
        });
    });
});
