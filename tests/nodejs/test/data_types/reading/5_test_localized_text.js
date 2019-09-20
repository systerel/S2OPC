const sopc_client = require('../../../lib/sopc_client');
const test_helper = require('../../helpers/connection');
const assert = require('assert');

describe("Read Single Localized Text type", function () {
    let connectionId = 0;
    before(function (done) {
        connectionId = test_helper.connect();
        done();
    });
    it("Every result shall be a scalar localized text", function (done) {
        let read_value_string = new sopc_client.ReadValue()
            .setNodeId("ns=1;s=String_030")
            .setAttributeId(4);
        let read_values = [read_value_string];

        let resultDataValues1;
        let status;
        [status, resultDataValues1] = sopc_client.read(connectionId,
            read_values);
        assert.equal(resultDataValues1.length, 1);
        assert.equal(status, 0, `Status is ${status}`);
        for (let elt of resultDataValues1) {
            assert.equal(elt.value.array_type, 0, `element is not a scalar`);
            assert.ok(typeof elt.value.value === "string",
                `${elt.value.value} is not a string`);
        }
        done();
    });
    after(function (done) {
        test_helper.disconnect(connectionId);
        done();
    });
});
