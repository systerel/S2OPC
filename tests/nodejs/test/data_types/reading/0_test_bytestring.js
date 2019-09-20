
const sopc_client = require('../../../lib/sopc_client');
const test_helper = require('../../helpers/connection');
const assert = require('assert');

describe("Read Single ByteString type", function () {
    let connectionId = 0;
    before(function(done){
        connectionId = test_helper.connect();
        done();
    });
    it("Every result shall be a scalar byteString", function (done) {
        let resultDataValues;
        let status;
        let read_value_bstring = new sopc_client.ReadValue()
            .setNodeId("ns=1;s=ByteString_029")
            .setAttributeId(13);
        let read_values = [read_value_bstring];

        [status, resultDataValues] = sopc_client.read(connectionId,
            read_values);
        assert.equal(status, 0, `Status is ${status}`);
        assert.equal(resultDataValues.length, 1);
        for (let elt of resultDataValues) {
            assert.equal(elt.value.array_type, 0, `element is not a scalar`);
            assert.ok(typeof elt.value.value === "string",
                `${elt.value.value} is not a byteString`);
        }
        done();
    });
    after(function(done) {
        test_helper.disconnect(connectionId);
        done();
    });
});
