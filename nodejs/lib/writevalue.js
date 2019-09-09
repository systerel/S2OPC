const bind = require('./bind_sopc_client');
const data_value = require('./datavalue');

class WriteValue
{
    constructor(){
        this.nodeId = '';
        this.indexRange = '';
        this.DataValueC = new data_value.DataValue();
    }
    setNodeId(nodeId) {
        this.nodeId = nodeId;
        return this;
    }
    setIndexRange(indexRange) {
        this.indexRange = indexRange;
    }
    setValue(data_value){
        this.DataValueC = data_value
        return this;
    }
    ToC() {
        console.log(this);
        return new bind.SOPC_ClientHelper_WriteValue({
            nodeId : this.nodeId,
            indexRange : this.indexRange,
            value : this.DataValueC.ToC()
        });
    }
}

module.exports.WriteValue = WriteValue;
