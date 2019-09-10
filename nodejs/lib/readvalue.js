const bind = require('./bind_sopc_client');
const ref = require('ref');

class ReadValue {
    constructor() {
        this.nodeId = null;
        this.indexRange = null;
        this.attributeId = 13;
    }

    setNodeId(nodeId) {
        this.nodeId = nodeId;
        return this;
    }

    setIndexRange(indexRange) {
        this.indexRange = indexRange;
        return this;
    }

    setAttributeId(attributeId) {
        this.attributeId = attributeId;
        return this;
    }

    ToC() {
        return new bind.SOPC_ClientHelper_ReadValue({
            nodeId : this.nodeId === null ? ref.NULL_POINTER : this.nodeId,
            attributeId : this.attributeId,
            indexRange : this.indexRange === null ? ref.NULL_POINTER : this.indexRange
        });
    }
}

module.exports.ReadValue = ReadValue;
