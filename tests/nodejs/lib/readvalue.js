/**
 * ReadValue Object
 * @module readvalue
 */
const bind = require('./bind_sopc_client');
const ref = require('ref');

/**
 * Class representing a read value
 */
class ReadValue {
    /**
     * Creates an empty read value
     */
    constructor() {
        this.nodeId = null;
        this.indexRange = null;
        this.attributeId = 13;
    }

    /**
     * Set the node id
     * @param {String} nodeId node identifier
     * @returns this
     */
    setNodeId(nodeId) {
        this.nodeId = nodeId;
        return this;
    }

    /**
     * Set the index range
     * NULL if not set
     * @param {String} indexRange index range
     */
    setIndexRange(indexRange) {
        this.indexRange = indexRange;
        return this;
    }

    /**
     * Set the attribute id
     * @param {Number} attributeId attribute to be read
     * @returns this
     */
    setAttributeId(attributeId) {
        this.attributeId = attributeId;
        return this;
    }

    /**
     * Internal function used to convert JS ReadValue to C ReadValue
     * @returns C_ReadValue
     */
    ToC() {
        return new bind.SOPC_ClientHelper_ReadValue({
            nodeId : this.nodeId === null ? ref.NULL_POINTER : this.nodeId,
            attributeId : this.attributeId,
            indexRange : this.indexRange === null ? ref.NULL_POINTER : this.indexRange
        });
    }
}

module.exports = {
    ReadValue
};
