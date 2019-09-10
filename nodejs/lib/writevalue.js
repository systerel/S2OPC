const bind = require('./bind_sopc_client');
const data_value = require('./datavalue');
const ref = require('ref');

class WriteValue
{
    constructor(){
        this.nodeId = null;
        this.indexRange = null;
        this.DataValue = new data_value.DataValue();
    }
    setNodeId(nodeId) {
        this.nodeId = nodeId;
        return this;
    }
    setIndexRange(indexRange) {
        this.indexRange = indexRange;
    }
    setValue(data_value){
        this.DataValue = data_value
        return this;
    }
    ToC() {
        return new bind.SOPC_ClientHelper_WriteValue({
            nodeId : this.nodeId === null ? ref.NULL_POINTER : this.nodeId,
            indexRange : this.indexRange === null ? ref.NULL_POINTER : this.indexRange,
            value : this.DataValue.ToC().ref()
        });
    }
}

module.exports.WriteValue = WriteValue;
