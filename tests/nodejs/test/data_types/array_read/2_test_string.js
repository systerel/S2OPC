
const sopc_client = require('../../../lib/sopc_client');
const test_helper = require('../../helpers/connection');
const assert = require('assert');

describe("Read Array String type", function () {
    let connectionId = 0;
    before(function(done) {
        connectionId = test_helper.connect();
        done();
    });
    it("Every result shall be a string array", function (done) {
        let read_value_string = new sopc_client.ReadValue()
            .setNodeId("ns=1;i=1026")
            .setAttributeId(13);
        let read_values = [read_value_string];

        let resultDataValues1;
        let status;
        [status, resultDataValues1] = sopc_client.read(connectionId,
            read_values);
        assert.equal(resultDataValues1.length, 1);
        assert.equal(status, 0, `Status is ${status}`);
        for (let elt of resultDataValues1) {
            assert.equal(elt.value.array_type, 1, `element is not an array`);
            assert.ok(Array.isArray(elt.value.value));
            assert.ok(elt.value.value.length > 0);
            for (let arrElt of elt.value.value) {
                assert.ok(typeof arrElt === "string",
                    `${arrElt} is not a string`);
            }
        }
        done();
    });
    after(function(done) {
        test_helper.disconnect(connectionId);
        done();
    });
});
