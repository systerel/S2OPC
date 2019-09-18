/**
 * WriteValue Object
 * @module writevalue
 * @see module:datavalue
 */
const bind = require('./bind_sopc_client');
const data_value = require('./datavalue');
const ref = require('ref');

/**
 * Class representing a WriteValue
 */
class WriteValue
{
    /**
     * Creates an empty Write Value
     */
    constructor(){
        this.nodeId = null;
        this.indexRange = null;
        this.DataValue = new data_value.DataValue();
    }

    /**
     * Set the node id of the write value
     * @param {String} nodeId node identifier
     * @returns this
     */
    setNodeId(nodeId) {
        this.nodeId = nodeId;
        return this;
    }

    /**
     * Set the index range
     * NULL by default
     * @param {String} indexRange index range
     * @returns this
     */
    setIndexRange(indexRange) {
        this.indexRange = indexRange;
    }

    /**
     * Set the DataValue
     * @param {DataValue} data_value DataValue to be written
     * @returns this
     * @see module:datavalue
     */
    setValue(data_value){
        this.DataValue = data_value
        return this;
    }

    /**
     * Internal function used to convert JS WriteValue to C WriteValue
     * @returns C_WriteValue
     */
    ToC() {
        return new bind.SOPC_ClientHelper_WriteValue({
            nodeId : this.nodeId === null ? ref.NULL_POINTER : this.nodeId,
            indexRange : this.indexRange === null ? ref.NULL_POINTER : this.indexRange,
            value : this.DataValue.ToC().ref()
        });
    }
}

module.exports = {
    WriteValue
};
